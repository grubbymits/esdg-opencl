#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/MachineDominators.h"
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
#include "LE1.h"

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
  std::cout << "EndPacket with " << MI->getOpcode() << std::endl;
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
  std::cout << "Adding " << MI->getOpcode() << " to packet" << std::endl;
  const MCSchedClassDesc *SchedDesc = SU->SchedClass;
  const MCWriteProcResEntry *WriteProc =
    MCSubtarget->getWriteProcResBegin(SchedDesc);
  ResourceTable[WriteProc->ProcResourceIdx]++;
  CurrentPacket.push_back(MI);
  CurrentPacketSize += SU->SchedClass->NumMicroOps;
}

bool LE1MIPacker::ResourcesAvailable(const MCSchedClassDesc *SchedDesc) {
  if (!SchedDesc) {
    std::cout << "Failed to retrieve SchedDesc" << std::endl;
    DEBUG(dbgs() << "Failed to retrieve SchedDesc\n");
    return false;
  }
  std::cout << "CurrentPacketSize = " << CurrentPacketSize << std::endl;

  unsigned NumMicroOps = SchedDesc->NumMicroOps;
  std::cout << "NumMicroOps = " << NumMicroOps << std::endl;

  if ((CurrentPacketSize + NumMicroOps) > SchedModel->IssueWidth) {
    std::cout << "CurrentPacket size + MicroOps > IssueWidth"
      << std::endl;
    return false;
  }

  const MCWriteProcResEntry *WriteProc =
    MCSubtarget->getWriteProcResBegin(SchedDesc);
  unsigned ProcResId = WriteProc->ProcResourceIdx;
  const MCProcResourceDesc *ProcRes =
    SchedModel->getProcResource(ProcResId);

  if (ProcRes->NumUnits < (ResourceTable[ProcResId] + 1)) {
    std::cout << "No resources left" << std::endl;
    return false;
  }

  std::cout << "Resources are available" << std::endl;
  return true;
}

bool LE1MIPacker::PacketDependences(SUnit *NewSU) {
  for (packet_iterator I = CurrentPacket.begin(), E = CurrentPacket.end();
       I != E; ++I) {

    SUnit *PackagedSU = MIToSUnit[*I];
    std::cout << "Checking packet dependencies" << std::endl;
    std::cout << "PackagedSU has " << PackagedSU->Succs.size() <<
      " successors" << std::endl;
    std::cout << "NewSU has " << NewSU->Succs.size() <<
      " successors" << std::endl;

    if (PackagedSU->isSucc(NewSU)) {
      std::cout << "PackagedSU isSucc NewSU" << std::endl;
      for (unsigned i = 0; i < PackagedSU->Succs.size(); ++i) {

        if (PackagedSU->Succs[i].getSUnit() != NewSU)
          continue;

        SDep::Kind DepKind = PackagedSU->Succs[i].getKind();
        if (DepKind == SDep::Data) {
          std::cout << "Found dependency" << std::endl;
          return true;
        }
        else if (DepKind == SDep::Order)
          std::cout << "Order dependency" << std::endl;
        else if (DepKind == SDep::Anti)
          std::cout << "Anti dependency" << std::endl;
        else if (DepKind == SDep::Output)
          std::cout << "Output dependency" << std::endl;
      }
    }
  }
  // No data dependences found
  std::cout << "No dependencies found" << std::endl;
  return false;
}

bool LE1MIPacker::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "LE1MIPacker::runOnMachineFunction\n");

  //MF = &mf;
  const TargetMachine &TM = MF.getTarget();
  MCSubtarget = TM.getSubtargetImpl();

  if (!MCSubtarget) {
    std::cout << "Failed to retrieve SubtargetImpl" << std::endl;
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
    std::cout << "Failed to create ScheduleDAG" << std::endl;
    return false;
  }

  DEBUG(dbgs() << "Created Schedule DAG\n");
  SchedModel = Scheduler->getSchedModel()->getMCSchedModel();
  if (!SchedModel) {
    std::cout << "Failed to retrieve SchedModel" << std::endl;
    DEBUG(dbgs() << "Failed to retreive SchedModel\n");
    return false;
  }

  NumResources = SchedModel->getNumProcResourceKinds();
  ResourceTable = new unsigned[NumResources];
  std::cout << "Created ResourceTable with "
        << SchedModel->getNumProcResourceKinds() << " entries" << std::endl;
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

      std::cout << "Attempting to pack  " << MI->getOpcode() << std::endl;

      if (isPseudo(MI)) {
        std::cout << "isPseudo" << std::endl;
        continue;
      }

      if (isSolo(MI)) {
        EndPacket(MBB, MI);
        continue;
      }

      if ((ResourcesAvailable(Scheduler->getSchedClass(SU))) &&
          (!PacketDependences(SU)))
        AddToPacket(MI);
      else
        EndPacket(MBB, MI);

      std::cout << std::endl;
    }
    std::cout << "End of basic block\n" << std::endl;
    EndPacket(MBB, MBB->end());
    Scheduler->exitRegion();
    Scheduler->finishBlock();
  }
  delete ResourceTable;
  return true;
}

FunctionPass *llvm::createLE1MIPacker(LE1TargetMachine &TM) {
  return new LE1MIPacker();
}
