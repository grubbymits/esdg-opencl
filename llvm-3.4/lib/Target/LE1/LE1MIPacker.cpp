#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/MachinePassRegistry.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/ScoreboardHazardRecognizer.h"
#include "llvm/CodeGen/TargetSchedule.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "LE1.h"

#include <algorithm>
#include <iostream>

#define DEBUG_TYPE "le1-mi-packer"

using namespace llvm;

namespace llvm {
  void initializeLE1MIPackerPass(PassRegistry&);
}

namespace {
  // DefaultVLIWScheduler - This class extends ScheduleDAGInstrs and overrides
  // Schedule method to build the dependence graph.
  class PackerScheduler : public ScheduleDAGInstrs {
  public:
    PackerScheduler(MachineFunction &MF, const MachineLoopInfo &MLI,
                    const MachineDominatorTree &MDT);
    // Schedule - Actual scheduling work.
    void schedule();
  };

  typedef std::vector<MachineInstr*>::iterator packet_iterator;

  class LE1MIPacker : public MachineSchedContext,
                      public MachineFunctionPass {
  public:
    LE1MIPacker();
    void getAnalysisUsage(AnalysisUsage &AU) const;
    bool runOnMachineFunction(MachineFunction&);
    const char *getPassName() const {
      return "LE1 MI Packer";
    }
    static char ID;
  private:
    bool ResourcesAvailable(const MCSchedClassDesc *SchedDesc);
    bool PacketDependences(SUnit *NewSU);
    void AddToPacket(MachineInstr *MI);
    MachineInstr* ChoosePadInst(MachineBasicBlock *MBB, MachineInstr *MI);
    void EndPacket(MachineBasicBlock *MBB, MachineInstr *MI);
    void EndBoundaryPacket(MachineBasicBlock *MBB, MachineInstr *MI);
    bool isPseudo(MachineInstr *MI);
    bool isSolo(MachineInstr *MI);
    void CreateEmptyBundle(MachineBasicBlock *MBB,
                           MachineInstr *MI);
    void Finalise(MachineBasicBlock *MBB);
    unsigned CheckBundleLatencies(MachineInstr *B1,
                                  MachineInstr *B2);
    unsigned CheckInstructionLatencies(MachineInstr *MI,
                                       MachineInstr *M2);

    const TargetInstrInfo *TII;
    const TargetLowering *TLI;
    const MCSchedModel *SchedModel;
    const MCSubtargetInfo *MCSubtarget;
    MachineFunction *Function;
    std::vector<MachineInstr*> CurrentPacket;
    unsigned CurrentPacketSize;
    std::map<MachineInstr*, SUnit*> MIToSUnit;
    unsigned *ResourceTable;
    unsigned NumResources;
    unsigned IssueWidth;
  };

}

INITIALIZE_PASS_BEGIN(LE1MIPacker, "packets", "LE1 Packer", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_END(LE1MIPacker, "packets", "LE1 Packer", false, false)

PackerScheduler::PackerScheduler(MachineFunction &MF,
                                 const MachineLoopInfo &MLI,
                                 const MachineDominatorTree &MDT) :
  ScheduleDAGInstrs(MF, MLI, MDT, true) {
  CanHandleTerminators = true;
}

void PackerScheduler::schedule() {
  // Build the scheduling graph.
  buildSchedGraph(0);
}

char LE1MIPacker::ID = 0;

LE1MIPacker::LE1MIPacker() : MachineFunctionPass(ID) {
  DEBUG(dbgs() << "LE1MIPacker Constructor\n");
  initializeLE1MIPackerPass(*PassRegistry::getPassRegistry());
}

void LE1MIPacker::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequiredID(MachineDominatorsID);
  AU.addRequired<MachineLoopInfo>();
  AU.addRequired<AliasAnalysis>();
  AU.addRequired<TargetPassConfig>();
  AU.addPreserved<MachineDominatorTree>();
  AU.addPreserved<MachineLoopInfo>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool LE1MIPacker::isPseudo(MachineInstr *MI) {
  unsigned Opcode = MI->getOpcode();
  if (MI->isTransient())
    return true;
  return false;
}

bool LE1MIPacker::isSolo(MachineInstr *MI) {
  return false;
}

MachineInstr* LE1MIPacker::ChoosePadInst(MachineBasicBlock *MBB,
                                         MachineInstr *MI) {
  DebugLoc dl;
  unsigned NumPadInsts = IssueWidth - CurrentPacketSize;
  unsigned NumSlots = 1;
  const MCProcResourceDesc *ALURes = SchedModel->getProcResource(1);
  const MCProcResourceDesc *MULRes = SchedModel->getProcResource(4);

  //FIXME Remove hard coded entry to ResourceTable
  unsigned FreeALUs = ALURes->NumUnits - ResourceTable[1];
  unsigned FreeMULs = MULRes->NumUnits - ResourceTable[4];

  unsigned PadImm = 0;
  unsigned PadInst = 0;

  if (NumPadInsts > 1) {
    PadImm = 0xFFF;
    if (FreeALUs > 0) {
      PadInst = LE1::ANDi32;
      ResourceTable[1]++;
    }
    else if (FreeMULs > 0) {
      PadInst = LE1::MULLi32;
      ResourceTable[4]++;
    }
    else
      llvm_unreachable("not enough FUs to pad bundle");
    NumSlots = 2;
  }
  else {
    PadImm = 0;
    if (FreeALUs > 0) {
      PadInst = LE1::ANDi9;
      ResourceTable[1]++;
    }
    else if (FreeMULs > 0) {
      PadInst = LE1::MULLLi9;
      ResourceTable[4]++;
    }
    else
      llvm_unreachable("not enough FUs to pad bundle");
  }

  CurrentPacketSize += NumSlots;

  return BuildMI(*MBB, MI, dl, TII->get(PadInst), LE1::ZERO)
    .addReg(LE1::ZERO).addImm(PadImm);

}

void LE1MIPacker::EndBoundaryPacket(MachineBasicBlock *MBB, MachineInstr *MI) {
  DebugLoc dl;
  MachineInstr *NewInst = MI;
  MachineInstr *MIFirst = CurrentPacket.front();
  MachineBasicBlock::instr_iterator I = MI;
#ifndef NDEBUG
  DEBUG(dbgs() << "-------- EndBoundaryPacket with: \n");
  MI->dump();
  DEBUG(dbgs() << "CurrentPacketSize = " << CurrentPacketSize << "\n");
  DEBUG(dbgs() << "CurrentPacket.size() = " << CurrentPacket.size() << "\n");
#endif

  // Exits are handled differently because we don't want trailing operations
  // since they get ADDEDNOPS inserted between itself the the HALT operation.
  if (MI->getOpcode() != LE1::Exit)
    ++I;

  if (!CurrentPacket.empty()) {

    while (CurrentPacketSize < IssueWidth) {

      NewInst = ChoosePadInst(MBB, &(*I));

      //NewInst = BuildMI(*MBB, &(*I), dl, TII->get(LE1::PadInst), LE1::ZERO)
        //.addReg(LE1::ZERO).addImm(0);

      if ((MI->getOpcode() == LE1::Exit))
        CurrentPacket.insert(CurrentPacket.begin(), NewInst);
      else
        CurrentPacket.push_back(NewInst);

      //++CurrentPacketSize;
    }
    MIFirst = NewInst;
  }
#ifndef NDEBUG
  DEBUG(dbgs() << "MBB after inserting padding:\n");
  MBB->dump();
#endif

  if (CurrentPacket.size() > 1) {
    MIFirst = CurrentPacket.front();
    MachineInstr *MILast = (MI->getOpcode() == LE1::Exit) ? MI : &(*I);
    finalizeBundle(*MBB, MIFirst, MILast);
  }

  CurrentPacket.clear();
  CurrentPacketSize = 0;
  for (unsigned i = 0; i < NumResources; ++i)
    ResourceTable[i] = 0;
}

void LE1MIPacker::EndPacket(MachineBasicBlock *MBB, MachineInstr *MI) {
  DEBUG(dbgs() << "--- Entering EndPacket, CurrentPacket.size = "
        << CurrentPacket.size() << ", CurrentPacketSize = " << CurrentPacketSize
        << "\n");
#ifndef NDEBUG
  if (MI->getOpcode() >= LE1::ADDCG) {
    DEBUG(dbgs() << "Ending packet at instruction: \n");
    MI->dump();
  }
#endif

  // Insert instructions to ensure that bundles are the maximum size
  DebugLoc dl;
  if (!CurrentPacket.empty()) {
    unsigned NumPadInsts = IssueWidth - CurrentPacketSize;

#ifndef NDEBUG
    DEBUG(dbgs() << "CurrentPacket ! empty\n");
    if (CurrentPacketSize < IssueWidth)
      DEBUG(dbgs() << "Adding " << NumPadInsts
            << " padding instructions to:\n");
    MBB->dump();
#endif

    MachineInstr *FinalInst = MI; //CurrentPacket.back();
    while (CurrentPacketSize < IssueWidth) {

      //FinalInst = BuildMI(*MBB, MI, dl, TII->get(LE1::PadInst), LE1::ZERO)
        //.addReg(LE1::ZERO).addImm(0);
      FinalInst = ChoosePadInst(MBB, MI);
      CurrentPacket.push_back(FinalInst);
      //++CurrentPacketSize;
    }
  }
  if (CurrentPacket.size() > 1) {
    DEBUG(dbgs() << " ---- After inserting padding, but before finalisation\n");
#ifndef NDEBUG
    MBB->dump();
#endif
    MachineInstr *MIFirst = CurrentPacket.front();
    MachineInstr *MILast = MI;
    finalizeBundle(*MBB, MIFirst, MILast);
  }
  DEBUG(dbgs() << "After finalising:\n");
#ifndef NDEBUG
  MBB->dump();
#endif

  CurrentPacket.clear();
  CurrentPacketSize = 0;
  for (unsigned i = 0; i < NumResources; ++i)
    ResourceTable[i] = 0;

  DEBUG(dbgs() << "--- Leaving EndPacket\n");
}

void LE1MIPacker::AddToPacket(MachineInstr *MI) {
  SUnit *SU = MIToSUnit[MI];
  DEBUG(dbgs() << "Adding to " << MI->getOpcode() << " to packet\n");
  const MCSchedClassDesc *SchedDesc = SU->SchedClass;
  const MCWriteProcResEntry *WriteProc =
    MCSubtarget->getWriteProcResBegin(SchedDesc);
  ResourceTable[WriteProc->ProcResourceIdx]++;
  CurrentPacket.push_back(MI);
  CurrentPacketSize += SU->SchedClass->NumMicroOps;
  DEBUG(dbgs() << "NumMicroOps = " << SU->SchedClass->NumMicroOps << "\n");
}

bool LE1MIPacker::ResourcesAvailable(const MCSchedClassDesc *SchedDesc) {
  if (!SchedDesc) {
    DEBUG(dbgs() << "Failed to retrieve SchedDesc\n");
    return false;
  }
  DEBUG(dbgs() << "Are resources available for?\n");

  unsigned NumMicroOps = SchedDesc->NumMicroOps;
  DEBUG(dbgs() << "Number of micro ops = " << NumMicroOps << "\n");

  if ((CurrentPacketSize + NumMicroOps) > IssueWidth) {
    DEBUG(dbgs() << "Too many micro ops for current packet size\n");
    return false;
  }

  const MCWriteProcResEntry *WriteProc =
    MCSubtarget->getWriteProcResBegin(SchedDesc);
  unsigned ProcResId = WriteProc->ProcResourceIdx;
  const MCProcResourceDesc *ProcRes =
    SchedModel->getProcResource(ProcResId);

  DEBUG(dbgs() << "ProcRes->NumUnits = " << ProcRes->NumUnits
        << " and current usage = " << ResourceTable[ProcResId] << "\n");
  if (ProcRes->NumUnits < (ResourceTable[ProcResId] + 1)) {
    DEBUG(dbgs() << "No resources left\n");
    return false;
  }

  DEBUG(dbgs() << "Resource are available\n");
  return true;
}

bool LE1MIPacker::PacketDependences(SUnit *NewSU) {
  for (packet_iterator I = CurrentPacket.begin(), E = CurrentPacket.end();
       I != E; ++I) {

    SUnit *PackagedSU = MIToSUnit[*I];
    //std::cout << "Checking packet dependencies" << std::endl;
    //std::cout << "PackagedSU has " << PackagedSU->Succs.size() <<
      //" successors" << std::endl;
    //std::cout << "NewSU has " << NewSU->Succs.size() <<
      //" successors" << std::endl;

    if (PackagedSU->isSucc(NewSU)) {
      //std::cout << "PackagedSU isSucc NewSU" << std::endl;
      for (unsigned i = 0; i < PackagedSU->Succs.size(); ++i) {

        if (PackagedSU->Succs[i].getSUnit() != NewSU)
          continue;

        SDep::Kind DepKind = PackagedSU->Succs[i].getKind();
        if (DepKind == SDep::Data) {
          DEBUG(dbgs() << "Data dependency found\n");
          return true;
        }
      }
    }
  }
  // No data dependences found
  //std::cout << "No dependencies found" << std::endl;
  return false;
}

bool LE1MIPacker::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "LE1MIPacker::runOnMachineFunction\n");

  Function = &MF;
  const TargetMachine &TM = MF.getTarget();
  MCSubtarget = TM.getSubtargetImpl();
  TII = TM.getInstrInfo();
  TLI = TM.getTargetLowering();

  if (!MCSubtarget) {
    //std::cout << "Failed to retrieve SubtargetImpl" << std::endl;
    DEBUG(dbgs() << "Failed to retrieve SubtargetImpl\n");
    return false;
  }

  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  MachineDominatorTree &MDT = getAnalysis<MachineDominatorTree>();
  PassConfig = &getAnalysis<TargetPassConfig>();
  AA = &getAnalysis<AliasAnalysis>();
  //RegClassInfo->runOnMachineFunction(*MF);

  OwningPtr<ScheduleDAGInstrs>
    Scheduler(new PackerScheduler(MF, MLI, MDT));

  if (!Scheduler.get()) {
    DEBUG(dbgs() << "Failed to create ScheduleDAG\n");
    //std::cout << "Failed to create ScheduleDAG" << std::endl;
    return false;
  }

  DEBUG(dbgs() << "Created Schedule DAG\n");
  SchedModel = Scheduler->getSchedModel()->getMCSchedModel();
  if (!SchedModel) {
    //std::cout << "Failed to retrieve SchedModel" << std::endl;
    DEBUG(dbgs() << "Failed to retreive SchedModel\n");
    return false;
  }

  NumResources = SchedModel->getNumProcResourceKinds();
  ResourceTable = new unsigned[NumResources];
  IssueWidth = SchedModel->IssueWidth;
  //std::cout << "Created ResourceTable with "
    //    << SchedModel->getNumProcResourceKinds() << " entries" << std::endl;
  DEBUG(dbgs() << "Create ResourceTable with "
        << SchedModel->getNumProcResourceKinds() << " entries\n");

  // Visit all machine basic blocks.
  //
  // TODO: Visit blocks in global postorder or postorder within the bottom-up
  // loop tree. Then we can optionally compute global RegPressure.
  for (MachineFunction::iterator MBB = MF.begin(), MBBEnd = MF.end();
       MBB != MBBEnd; ++MBB) {

    Scheduler->startBlock(MBB);
    Scheduler->enterRegion(MBB, MBB->begin(), MBB->end(),
                           std::distance(MBB->begin(), MBB->end()));
    Scheduler->schedule();

    // Create a map of MachineInstrs to SUnits
    MIToSUnit.clear();
    for (unsigned i = 0, e = Scheduler->SUnits.size(); i != e; ++i) {
      SUnit *SU = &Scheduler->SUnits[i];
      MIToSUnit[SU->getInstr()] = SU;
    }
    DEBUG(dbgs() << "Mapped MIToSUnit\n");

    // Iterate through the machine instructions, skip pseudos and end packet
    // when there is a hazard
    for (MachineBasicBlock::iterator MI = MBB->begin(), ME = MBB->end();
         MI != ME; ++MI) {

      if (MI->getOpcode() == LE1::PadInst)
        continue;

      SUnit *SU = MIToSUnit[MI];

      // SU may be an inserted padded instruction that has been added after a
      // branch, so SU will be NULL
      if (!SU)
        continue;

      //std::cout << "Attempting to pack  " << MI->getOpcode() << std::endl;
      DEBUG(dbgs() << "Attempting to pack: " << MI->getOpcode() << "\n");
#ifndef NDEBUG
      MI->dump();
#endif

      if (isPseudo(MI)) {
        //std::cout << "isPseudo" << std::endl;
        continue;
      }

      if (isSolo(MI)) {
        EndPacket(MBB, MI);
        continue;
      }

      if ((ResourcesAvailable(Scheduler->getSchedClass(SU))) &&
          (!PacketDependences(SU))) {
        AddToPacket(MI);
      }
      else {
        EndPacket(MBB, MI);
        AddToPacket(MI);
      }
      if (TII->isSchedulingBoundary(MI, MBB, MF)) {
        DEBUG(dbgs() << "MI isSchedulingBoundary\n");
        EndBoundaryPacket(MBB, MI);
      }
    }

    DEBUG(dbgs() << " ----- Finishing MBB ----- \n");
    EndPacket(MBB, MBB->end());
    Finalise(MBB);
    Scheduler->exitRegion();
    Scheduler->finishBlock();
  }
  delete [] ResourceTable;
  return true;
}

void LE1MIPacker::CreateEmptyBundle(MachineBasicBlock *MBB,
                                    MachineInstr *MI) {
#ifndef NDEBUG
  DEBUG(dbgs() << " ------- CreateEmptyBundle at: \n");
  MI->dump();
#endif
  MachineInstr *FinalInst = MI;
  DebugLoc dl;

  // There is always at least half the issue width number of ALUs, and an
  // extra immediate can be used to help pad too. This is required when the
  // number of ALUs does not match the issue width.
  unsigned PadImm = (IssueWidth > 1) ? 0xFFFF : 0;
  unsigned PadSize = (IssueWidth > 1) ? 2 : 1;
  unsigned PadInst = (IssueWidth > 1) ? LE1::ANDi32 : LE1::ANDi9;

  unsigned BundleSize = 0;
  while (BundleSize < IssueWidth) {
    FinalInst = BuildMI(*MBB, MI, dl, TII->get(PadInst), LE1::ZERO)
      .addReg(LE1::ZERO).addImm(PadImm);
    CurrentPacket.push_back(FinalInst);
    BundleSize += PadSize;
  }
  if (CurrentPacket.size() > 1) {
    MachineInstr *MIFirst = CurrentPacket.front();
    finalizeBundle(*MBB, MIFirst, MI);
  }
  CurrentPacket.clear();
  CurrentPacketSize = 0;

  DEBUG(dbgs() << " ------- Finished creating EmptyBundle\n");
}

void LE1MIPacker::Finalise(MachineBasicBlock *MBB) {
#ifndef NDEBUG
  DEBUG(dbgs() << " --------- Finalise MBB: \n");
  MBB->dump();
#endif
  for (MachineBasicBlock::iterator BI = MBB->begin(), BE = --MBB->end();
       BI != BE; ++BI) {

    MachineBasicBlock::iterator Next = BI;
    ++Next;
    unsigned Latency = CheckBundleLatencies(&(*BI), &(*Next));
    // Assumes latency of two for all instructions
    if (Latency != 0) {
      DEBUG(dbgs() << "Latency != 0 between bundles\n");
      //DebugLoc dl;
      //BuildMI(*MBB, Next, dl, TII->get(LE1::CLK));
      CreateEmptyBundle(MBB, Next);
      ++BI;
    }
  }
}

unsigned LE1MIPacker::CheckBundleLatencies(MachineInstr *B1,
                                           MachineInstr *B2) {

  MachineBasicBlock::instr_iterator M1 = B1;
  MachineBasicBlock::instr_iterator M2 = B2;

  unsigned PacketLatency = 0;

  // First, handle the conditions where B1 and/or B2 are not bundled, (size = 0)
  if (!M1->isBundled() && !M2->isBundled()) {
    return CheckInstructionLatencies(M1, M2);
  }

  else if (!M1->isBundled()) {
    unsigned size = M2->getBundleSize();
    for (unsigned j = 0; j < size; ++j) {
      ++M2;
      PacketLatency = std::max(CheckInstructionLatencies(M1, M2),
                               PacketLatency);
    }
    return PacketLatency;
  }
  else if (M1->isBundled() && !M2->isBundled()) {
    unsigned size = M1->getBundleSize();
    for (unsigned i = 0; i < size; ++i) {
      ++M1;
      PacketLatency = std::max(CheckInstructionLatencies(M1, M2),
                               PacketLatency);
    }
    return PacketLatency;
  }

  // Iterate into bundles from the BUNDLE inst
  unsigned B1Size = M1->getBundleSize();
  unsigned B2Size = M1->getBundleSize();

  for (unsigned i = 0; i < B1Size; ++i) {
    ++M1;
    for (unsigned j = 0; j < B2Size; ++j) {
      ++M2;

      PacketLatency = std::max(CheckInstructionLatencies(M1, M2),
                               PacketLatency);
    }
    M2 = B2;
  }
  return PacketLatency;
}

unsigned LE1MIPacker::CheckInstructionLatencies(MachineInstr *M1,
                                                MachineInstr *M2) {
  unsigned Latency = 0;
  if (isPseudo(M1)) {
    return Latency;
  }
  if (isPseudo(M2)) {
    return Latency;
  }

  SUnit *S1 = MIToSUnit[M1];
  SUnit *S2 = MIToSUnit[M2];

  // M1 or M2 maybe one of the instructions that has been inserted for padding,
  // so S1 or S2 may not exist
  if (M1->getOpcode() == LE1::PadInst)
    return Latency;
  if (M2->getOpcode() == LE1::PadInst)
    return Latency;
  if (!S1)
    return Latency;
  if (!S2)
    return Latency;

  for (unsigned i = 0; i < S1->Succs.size(); ++i) {
    if (S1->Succs[i].getSUnit() != S2)
      continue;

    SDep::Kind DepKind = S1->Succs[i].getKind();
    if (DepKind == SDep::Data) {
      Latency = std::max(S1->Succs[i].getLatency(), Latency);
    }
  }
  return Latency;
}

FunctionPass *llvm::createLE1MIPacker(LE1TargetMachine &TM) {
  return new LE1MIPacker();
}
