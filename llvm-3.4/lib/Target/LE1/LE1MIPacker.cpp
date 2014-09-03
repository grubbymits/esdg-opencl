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

using namespace llvm;

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
    void EndPacket(MachineBasicBlock *MBB, MachineInstr *MI);
    bool isPseudo(MachineInstr *MI);
    bool isSolo(MachineInstr *MI);
    void Finalise(MachineBasicBlock *MBB);
    unsigned CheckBundleLatencies(MachineInstr *B1,
                                  MachineInstr *B2);
    unsigned CheckInstructionLatencies(MachineInstr *MI,
                                       MachineInstr *M2);

    const TargetInstrInfo *TII;
    const TargetLowering *TLI;
    const MCSchedModel *SchedModel;
    const MCSubtargetInfo *MCSubtarget;
    std::vector<MachineInstr*> CurrentPacket;
    unsigned CurrentPacketSize;
    std::map<MachineInstr*, SUnit*> MIToSUnit;
    unsigned *ResourceTable;
    unsigned NumResources;
  };

}

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
  //initializeLE1MIPackerPass();
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

void LE1MIPacker::EndPacket(MachineBasicBlock *MBB, MachineInstr *MI) {
  DEBUG(dbgs() << "EndPacket, size = " << CurrentPacket.size() << "\n");
  //std::cout << "EndPacket with " << MI->getOpcode() << std::endl;
  if (CurrentPacket.size() > 1) {
    MachineInstr *MIFirst = CurrentPacket.front();
    finalizeBundle(*MBB, MIFirst, MI);
  }
  CurrentPacket.clear();
  CurrentPacketSize = 0;
  for (unsigned i = 0; i < NumResources; ++i)
    ResourceTable[i] = 0;
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
}

bool LE1MIPacker::ResourcesAvailable(const MCSchedClassDesc *SchedDesc) {
  if (!SchedDesc) {
    DEBUG(dbgs() << "Failed to retrieve SchedDesc\n");
    return false;
  }
  DEBUG(dbgs() << "Are resources available for?\n");

  unsigned NumMicroOps = SchedDesc->NumMicroOps;
  DEBUG(dbgs() << "Number of micro ops = " << NumMicroOps << "\n");

  if ((CurrentPacketSize + NumMicroOps) > SchedModel->IssueWidth) {
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

  //MF = &mf;
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
    // when there is a hazard or the instruction is a solo (branches)
    for (MachineBasicBlock::iterator MI = MBB->begin(), ME = MBB->end();
         MI != ME; ++MI) {
      SUnit *SU = MIToSUnit[MI];

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
          (!PacketDependences(SU)))
        AddToPacket(MI);
      else {
        EndPacket(MBB, MI);
        AddToPacket(MI);
      }
    }

    EndPacket(MBB, MBB->end());
    Finalise(MBB);
    Scheduler->exitRegion();
    Scheduler->finishBlock();
  }
  delete ResourceTable;
  return true;
}

void LE1MIPacker::Finalise(MachineBasicBlock *MBB) {
  std::cout << "Finalise" << std::endl;
  for (MachineBasicBlock::iterator BI = MBB->begin(), BE = --MBB->end();
       BI != BE; ++BI) {

    std::cout << "Bundle size = " << BI->getBundleSize() << std::endl;

    MachineBasicBlock::iterator Next = BI;
    ++Next;
    unsigned Latency = CheckBundleLatencies(&(*BI), &(*Next));
    std::cout << "Latency = " << Latency << std::endl;
    // Assumes latency of two for all instructions
    if (Latency != 0) {
      DebugLoc dl;
      BuildMI(*MBB, Next, dl, TII->get(LE1::CLK));
      ++BI;
    }
  }
}

unsigned LE1MIPacker::CheckBundleLatencies(MachineInstr *B1,
                                           MachineInstr *B2) {

  std::cout << "CheckBundleLatencies" << std::endl;

  MachineBasicBlock::instr_iterator M1 = B1;
  MachineBasicBlock::instr_iterator M2 = B2;

  unsigned PacketLatency = 0;

  // First, handle the conditions where B1 and/or B2 are not bundled, (size = 0)
  if (!M1->isBundled() && !M2->isBundled()) {
    std::cout << "Neither M1 or M2 are bundled" << std::endl;
    return CheckInstructionLatencies(M1, M2);
  }

  else if (!M1->isBundled()) {
    std::cout << "M1 isnt bundled, but M2 is" << std::endl;
    std::cout << "M2 BundleSize = " << M2->getBundleSize() << std::endl;
    unsigned size = M2->getBundleSize();
    for (unsigned j = 0; j < size; ++j) {
      ++M2;
      PacketLatency = std::max(CheckInstructionLatencies(M1, M2),
                               PacketLatency);
    }
    return PacketLatency;
  }
  else if (M1->isBundled() && !M2->isBundled()) {
    std::cout << "M1 is bundled, M2 is not" << std::endl;
    std::cout << "M1 BundleSize = " << M1->getBundleSize() << std::endl;
    unsigned size = M1->getBundleSize();
    for (unsigned i = 0; i < size; ++i) {
      ++M1;
      PacketLatency = std::max(CheckInstructionLatencies(M1, M2),
                               PacketLatency);
    }
    return PacketLatency;
  }

  std::cout << "Both are bundled" << std::endl;
  // Iterate into bundles from the BUNDLE inst
  std::cout << "M1 BundleSize = " << M1->getBundleSize() << std::endl;
  std::cout << "M2 BundleSize = " << M2->getBundleSize() << std::endl;
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
  std::cout << "CheckInstructionLatencies:\n  ";
  unsigned Latency = 0;
  if (isPseudo(M1)) {
    std::cout << "M1 isPseudo" << std::endl;
    return Latency;
  }
  if (isPseudo(M2)) {
    std::cout << "M2 isPseudo" << std::endl;
    return Latency;
  }
  std::cout << M1->getOpcode() << ",  ";
  std::cout << M2->getOpcode() << std::endl;

  SUnit *S1 = MIToSUnit[M1];
  SUnit *S2 = MIToSUnit[M2];

  for (unsigned i = 0; i < S1->Succs.size(); ++i) {
    if (S1->Succs[i].getSUnit() != S2)
      continue;

    SDep::Kind DepKind = S1->Succs[i].getKind();
    if (DepKind == SDep::Data) {
      std::cout << "Data dep found" << std::endl;
      Latency = std::max(S1->Succs[i].getLatency(), Latency);
    }
  }
  return Latency;
}

FunctionPass *llvm::createLE1MIPacker(LE1TargetMachine &TM) {
  return new LE1MIPacker();
}
