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
  initSUnits();
}

char LE1MIPacker::ID = 0;
//char &llvm::LE1MIPackerID = LE1MIPacker::ID;

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
  return false;
}

bool LE1MIPacker::isSolo(MachineInstr *MI) {
  return false;
}

void LE1MIPacker::EndPacket(MachineBasicBlock *MBB, MachineInstr *MI) {
  DEBUG(dbgs() << "EndPacket, size = " << CurrentPacket.size() << "\n");
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
  unsigned NumMicroOps = SchedDesc->NumMicroOps;
  std::cout << "NumMicroOps = " << NumMicroOps << std::endl;
  std::cout << "CurrentPacketSize = " << CurrentPacketSize << std::endl;

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

bool LE1MIPacker::runOnMachineFunction(MachineFunction &mf) {
  DEBUG(dbgs() << "LE1MIPacker::runOnMachineFunction\n");

  MF = &mf;
  const TargetMachine *TM = &(MF->getTarget());
  MCSubtarget = TM->getSubtargetImpl();

  if (!MCSubtarget) {
    std::cout << "Failed to retrieve SubtargetImpl" << std::endl;
    DEBUG(dbgs() << "Failed to retrieve SubtargetImpl\n");
    return false;
  }

  MLI = &getAnalysis<MachineLoopInfo>();
  MDT = &getAnalysis<MachineDominatorTree>();
  PassConfig = &getAnalysis<TargetPassConfig>();
  AA = &getAnalysis<AliasAnalysis>();
  RegClassInfo->runOnMachineFunction(*MF);

  OwningPtr<ScheduleDAGInstrs>
    Scheduler(new PackerScheduler(*MF, *MLI, *MDT));

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
  for (MachineFunction::iterator MBB = MF->begin(), MBBEnd = MF->end();
       MBB != MBBEnd; ++MBB) {

    Scheduler->startBlock(MBB);
    Scheduler->enterRegion(MBB, MBB->begin(), MBB->end(),
                           std::distance(MBB->begin(), MBB->end()));
    Scheduler->schedule();

    // Create a map of MachineInstrs to SUnits
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

      if (isPseudo(MI)) {
        continue;
      }

      if (isSolo(MI)) {
        EndPacket(MBB, MI);
        continue;
      }

      if (ResourcesAvailable(Scheduler->getSchedClass(SU)))
        AddToPacket(MI);
      else
        EndPacket(MBB, MI);
    }
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
