#include "LE1TargetMachine.h"
#include "LE1Subtarget.h"
#include "LE1MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {
  class LE1CFGOptimiser : public MachineFunctionPass {
    LE1TargetMachine &TM;
    const LE1Subtarget &ST;
    const TargetInstrInfo *TII;

  public:
    static char ID;
    LE1CFGOptimiser(LE1TargetMachine &tm) :
      MachineFunctionPass(ID), TM(tm), ST(*tm.getSubtargetImpl()),
      TII(tm.getInstrInfo()) {}

    const char *getPassName() const {
      return "LE1 CFG Optimiser";
    }

    bool runOnMachineFunction(MachineFunction &MF);
  };

  char LE1CFGOptimiser::ID = 0;

bool LE1CFGOptimiser::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "\nRunning CFG Optimiser");
  for(MachineFunction::iterator MBBi = MF.begin(); MBBi != MF.end(); ++ MBBi) {
    MachineBasicBlock *MBB = MBBi;
    MachineBasicBlock::reverse_iterator I = MBB->rbegin(), REnd = MBB->rend();

    // Skip debug instructions
    while (I != REnd && I->isDebugValue())
      ++I;

    DEBUG(dbgs() << "\nBB = '" << MBB->getFullName() << "' , it contains " <<
          MBB->size() << " instructions.");
    // If the block doesn't end with a unconditional branch,
    if (I == REnd || (I->getOpcode() != LE1::GOTO))
      continue;

    MachineInstr* LastInst = &*I;
    // Check whether the target basic block is effectively a fall through
    MachineBasicBlock *TBB = I->getOperand(0).getMBB();
    DEBUG(dbgs() << "\nTBB = " << TBB->getFullName());
    if(MBB->isLayoutSuccessor(TBB)) {
      //std::cout << "erasing from parent\n";
      DEBUG(dbgs() << "\nSuccessor is Target of unconditional branch");
      LastInst->eraseFromParent();
      DEBUG(dbgs() << "\nErased " << LastInst->getOpcode() << " from parent");
      //continue;
    }
    else if(MBB->size() > 1) {
      ++I;
      if(I->getOpcode() == LE1::BR ||
         I->getOpcode() == LE1::BRF) {
        TBB = I->getOperand(1).getMBB();

        if(MBB->isLayoutSuccessor(TBB)) {
          unsigned NewOpcode;
          DEBUG(dbgs() << "\nSuccessor is Target of conditional branch");
          if(I->getOpcode() == LE1::BR)
            NewOpcode = LE1::BRF;
          else
            NewOpcode = LE1::BR;

          // Now reverse set the reverse opcode and replace the branch location
          // with that of the following GOTO
          const MCInstrDesc &MCID = TII->get(NewOpcode);
          I->setDesc(MCID);
          DEBUG(dbgs() << "\nReplaced Opcode with opposite conditional Op");
          I->RemoveOperand(1);
          DEBUG(dbgs() << "\nSwapping target BB");
          I->addOperand(LastInst->getOperand(0));
          LastInst->eraseFromParent();
          DEBUG(dbgs() << "\nErased instruction from parent");
        }
      }
    }
  }
  return true;
}

} // namespace

FunctionPass *llvm::createLE1CFGOptimiser(LE1TargetMachine &TM) {
  return new LE1CFGOptimiser(TM);
}
