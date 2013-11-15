#include "LE1TargetMachine.h"
#include "LE1Subtarget.h"
#include "LE1MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

namespace {
  class LE1BBMerger : public MachineFunctionPass {
    LE1TargetMachine &TM;
    const LE1Subtarget &ST;
  public:
    static char ID;
    LE1BBMerger(LE1TargetMachine &tm) :
      MachineFunctionPass(ID), TM(tm), ST(*tm.getSubtargetImpl()) { }
    const char *getPassName() const {
      return "LE1 BB Merger";
    }
    bool runOnMachineFunction(MachineFunction &MF);
  };

  char LE1BBMerger::ID = 0;

} // end namespace

bool LE1BBMerger::runOnMachineFunction(MachineFunction &MF) {

  for(MachineFunction::iterator MBBb = MF.begin(), MBBe = MF.end();
      MBBb != MBBe; ++MBBb) {

    MachineBasicBlock *MBB = MBBb;

    if (MBB->succ_size() == 2) {
      std::cerr << "MBB has 2 successors" << std::endl;
      MachineBasicBlock *succ1 = *(MBB->succ_begin());
      MachineBasicBlock *succ2 = (*++(MBB->succ_begin()));
      if ((succ1->succ_size() == 1) && (succ2->succ_size() == 1)) {
        std::cerr << "Both successors have one successor" << std::endl;

        MachineBasicBlock *succ1_succ1 = *(succ1->succ_begin());
        MachineBasicBlock *succ2_succ1 = *(succ2->succ_begin());


        if (succ1_succ1 == succ2_succ1) {
          std::cerr << "FOUND MBB DIAMOND:" << std::endl;
          std::cerr << "MBB = " << std::endl;
          MBB->dump();
          std::cerr << "SUCC1 = " << std::endl;
          succ1->dump();
          std::cerr << "SUCC2 = " << std::endl;
          succ2->dump();
          std::cerr << "END = " << std::endl;
          succ1_succ1->dump();
        }
        else {
          std::cout << "succ1_succ1 has " << succ1_succ1->succ_size()
            << " successors" << std::endl;
          std::cout << "succ2_succ1 has " << succ2_succ1->succ_size()
            << " successors" << std::endl;
          if (succ1->isSuccessor(succ2_succ1)) {
            std::cerr << "succ2's successor is a successor of succ1"
              << std::endl;
          }
        }
      }
    }
  }
  return true;
}


FunctionPass *llvm::createLE1BBMerger(LE1TargetMachine &TM) {
  return new LE1BBMerger(TM);
}
