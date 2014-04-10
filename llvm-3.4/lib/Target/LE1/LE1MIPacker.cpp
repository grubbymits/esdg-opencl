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
    ScheduleDAGInstrs *createMachineScheduler();
    void EndPacket(MachineBasicBlock *MBB, MachineInstr *MI);
    bool isPseudo(MachineInstr *MI);
    bool isSolo(MachineInstr *MI);
    const TargetSchedModel *SchedModel;
    ScheduleHazardRecognizer *HazardRec;
    std::vector<MachineInstr*> CurrentPacketMIs;
    std::map<MachineInstr*, SUnit*> MIToSUnit;
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
  //AU.addRequired<SlotIndexes>();
  //AU.addPreserved<SlotIndexes>();
  //AU.addRequired<LiveIntervals>();
  //AU.addPreserved<LiveIntervals>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

ScheduleDAGInstrs *LE1MIPacker::createMachineScheduler() {
  //return PassConfig->createMachineScheduler(this);
}

bool LE1MIPacker::isPseudo(MachineInstr *MI) {
  return false;
}

bool LE1MIPacker::isSolo(MachineInstr *MI) {
  return false;
}

void LE1MIPacker::EndPacket(MachineBasicBlock *MBB, MachineInstr *MI) {
  DEBUG(dbgs() << "EndPacket, size = " << CurrentPacketMIs.size() << "\n");
  if (CurrentPacketMIs.size() > 1) {
    MachineInstr *MIFirst = CurrentPacketMIs.front();
    finalizeBundle(*MBB, MIFirst, MI);
  }
  HazardRec->AdvanceCycle();
  CurrentPacketMIs.clear();
}

bool LE1MIPacker::runOnMachineFunction(MachineFunction &mf) {
  DEBUG(dbgs() << "LE1MIPacker::runOnMachineFunction\n");

  MF = &mf;
  const TargetMachine *TM = &(MF->getTarget());
  const TargetInstrInfo *TII = TM->getInstrInfo();
  MLI = &getAnalysis<MachineLoopInfo>();
  MDT = &getAnalysis<MachineDominatorTree>();
  PassConfig = &getAnalysis<TargetPassConfig>();
  AA = &getAnalysis<AliasAnalysis>();
  //LIS = &getAnalysis<LiveIntervals>();
  RegClassInfo->runOnMachineFunction(*MF);

  // Instantiate the selected scheduler for this target, function, and
  // optimization level.
  OwningPtr<ScheduleDAGInstrs>
    Scheduler(new PackerScheduler(*MF, *MLI, *MDT));

  if (!Scheduler.get()) {
    DEBUG(dbgs() << "Failed to create ScheduleDAG\n");
    return false;
  }

  DEBUG(dbgs() << "Created Schedule DAG\n");
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
    SchedModel = Scheduler->getSchedModel();
    //TRI = Scheduler->TRI;
    if (!SchedModel) {
      DEBUG(dbgs() << "No SchedModel\n");
      return false;
    }

    const InstrItineraryData *Itin = SchedModel->getInstrItineraries();
    //HazardRec = TII->CreateTargetMIHazardRecognizer(Itin, Scheduler.get());
    HazardRec = TII->CreateTargetPostRAHazardRecognizer(Itin, Scheduler.get());

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

      if (HazardRec->getHazardType(SU) == ScheduleHazardRecognizer::NoHazard) {
        HazardRec->EmitInstruction(SU);
      }
      else {
        EndPacket(MBB, MI);
      }
    }
    EndPacket(MBB, MBB->end());
    // HazardRec->Reset();
    // delete HazardRec;
    Scheduler->exitRegion();
    Scheduler->finishBlock();
  }
  return true;
}

FunctionPass *llvm::createLE1MIPacker(LE1TargetMachine &TM) {
  return new LE1MIPacker();
}
