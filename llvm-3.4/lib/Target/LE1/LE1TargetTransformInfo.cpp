#include "LE1.h"
#include "LE1TargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/CostTable.h"
using namespace llvm;

namespace llvm {
  void initializeLE1TTIPass(PassRegistry &);
}

namespace {

  class LE1TTI : public ImmutablePass, public TargetTransformInfo {
    const LE1TargetMachine *TM;
    const LE1Subtarget *ST;
    const LE1TargetLowering *TLI;

  public:
    LE1TTI() : ImmutablePass(ID), ST(0), TLI(0) {
      llvm_unreachable("This pass cannot be directly constructed");
    }

    LE1TTI(const LE1TargetMachine *TM)
      : ImmutablePass(ID), TM(TM), ST(TM->getSubtargetImpl()),
      TLI(TM->getTargetLowering()) {
        initializeLE1TTIPass(*PassRegistry::getPassRegistry());
      }

    virtual void initializePass() {
      pushTTIStack(this);
    }

    virtual void finializePass() {
      popTTIStack();
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      TargetTransformInfo::getAnalysisUsage(AU);
    }

    static char ID;

    virtual void *getAdjustedAnalysisPointer(const void *ID) {
      if (ID == &TargetTransformInfo::ID)
        return (TargetTransformInfo*)this;
      return this;
    }

    virtual unsigned getMaximumUnrollFactor() const;

    virtual unsigned getNumberOfRegisters(bool Vector) const;

    virtual unsigned getRegisterBitWidth(bool Vector) const;

  };

} // end anonymous namespace

char LE1TTI::ID = 0;

INITIALIZE_AG_PASS(LE1TTI, TargetTransformInfo, "le1tti",
                   "LE1 Target Transform Info", true, true, false)

ImmutablePass *
llvm::createLE1TargetTransformInfoPass(const LE1TargetMachine *TM) {
  return new LE1TTI(TM);
}

unsigned LE1TTI::getNumberOfRegisters(bool Vector) const {
  if (Vector)
    return 24;
  else
    return 60;
}

unsigned LE1TTI::getRegisterBitWidth(bool Vector) const {
  if (Vector)
    return 64;
  else
    return 32;
}

unsigned LE1TTI::getMaximumUnrollFactor() const {
  return 8;
}
