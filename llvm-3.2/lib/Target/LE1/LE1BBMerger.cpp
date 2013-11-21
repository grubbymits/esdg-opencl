#include "LE1TargetMachine.h"
#include "LE1Subtarget.h"
#include "LE1MachineFunction.h"
#include "MCTargetDesc/LE1MCTargetDesc.h"
#include "llvm/Instructions.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"


//#define GET_INSTRINFO_ENUM
//#define GET_INSTRINFO_MC_DESC
//#include "LE1GenInstrInfo.inc"

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

    MachineBasicBlock *A = MBBb;

    if (A->succ_size() == 2) {
      std::cerr << "MBB has 2 successors" << std::endl;
      MachineBasicBlock *B = *(A->succ_begin());
      MachineBasicBlock *C = (*++(A->succ_begin()));
      if ((B->succ_size() == 1) && (C->succ_size() == 1)) {
        std::cerr << "Both successors have one successor" << std::endl;

        if (*(B->succ_begin()) == *(C->succ_begin())) {
          MachineBasicBlock *D = *(B->succ_begin());

          std::cerr << "FOUND MBB DIAMOND:" << std::endl;
          std::cerr << "A = " << std::endl;
          A->dump();
          std::cerr << "B = " << std::endl;
          B->dump();
          std::cerr << "C = " << std::endl;
          C->dump();
          std::cerr << "D = " << std::endl;
          D->dump();

          // Blocks B and C should not have PHI nodes and they will have at most
          // one terminator each.
          MachineBasicBlock::iterator BBegin = B->getFirstNonPHI();
          MachineBasicBlock::iterator BEnd = B->getFirstTerminator();
          if (BEnd->isTerminator())
            --BEnd;
          MachineBasicBlock::iterator CBegin = C->getFirstNonPHI();
          MachineBasicBlock::iterator CEnd = C->getFirstTerminator();
          if (CEnd->isTerminator())
            --CEnd;

          // There will be a maximum of two terminators, the first of which will
          // be the conditional.
          MachineBasicBlock::iterator ATerm = A->getFirstTerminator();
          unsigned CondReg = ATerm->getOperand(0).getReg();
          MachineBasicBlock *TargetBB = ATerm->getOperand(1).getMBB();
          unsigned Opcode = ATerm->getOpcode();

          std::cerr << "Splicing A with B" << std::endl;
          if (BBegin == BEnd)
            A->splice(ATerm, B, BBegin);
          else
            A->splice(ATerm, B, BBegin, BEnd);

          std::cerr << "Splicing A with C" << std::endl;
          if (CBegin == CEnd)
            A->splice(ATerm, C, CBegin);
          else
            A->splice(ATerm, C, CBegin, CEnd);

          // Remove terminator(s) from block A
          std::cerr << "Removing terminator from A" << std::endl;
          A->erase(ATerm);
          if (A->getFirstTerminator()->isTerminator())
            A->erase(A->getFirstTerminator());

          // If Block D's predecessors are only B and C, we don't have to leave
          // a copy of the block and so we don't have to modify the PHI inst.
          // Iterate through the PHI nodes and insert LE1::SLCT insts at the end
          // of block A.
          if (D->pred_size() == 2) {
            MachineBasicBlock::iterator Phi = --(D->getFirstNonPHI());
            while (Phi->isPHI()) {

              unsigned Opc = 0;
              if (Opcode == LE1::BR)
                Opc = LE1::SLCTrr;
              else
                Opc = LE1::SLCTFrr;

              // The conditional terminator of block A determines how the select
              // instructions work. The operands of the PHI node have values as
              // well as the BB which it is coming from:
              // vregD = PHI vregB <BB_B>, vregC <BB_C>
              // We can check whether Target refers to BB_B or BB_C
              // vregD = Opc Cond, TargetBB, Other
              Value *True = NULL;
              Value *False = NULL;
              /*
              if (Phi->getIncomingBlock(0) == TargetBB) {
                True = Phi->getIncomingValue(0);
                False = Phi->getIncomingValue(1);
              }
              else {
                True = Phi->getIncomingValue(1);
                False = Phi->getIncomingValue(0);
              }
              MachineInstr *MI = BuildMI(Opc, 3, DestReg).addReg(CondReg);*/

            }
          }
          else {

          }

          std::cerr << "New A = " << std::endl;
          A->dump();

          B->eraseFromParent();
          C->eraseFromParent();
        }
      }
    }
  }
  return true;
}


FunctionPass *llvm::createLE1BBMerger(LE1TargetMachine &TM) {
  return new LE1BBMerger(TM);
}
