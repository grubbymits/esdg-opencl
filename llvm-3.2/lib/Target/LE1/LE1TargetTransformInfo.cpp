// Refer to X86TargetTransformInfo of llvm-3.3 to get implementation details.
// We should be able to use this to vectorize some of the kernel loops if we
// backport the loop vectorizer code, or update the driver to 3.3

/*
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

    virtual unsigned getNumberOfRegisters(bool Vector) const;
    // Return the widest value.
    virtual unsigned getRegisterBitWidth(bool Vector) const;

  };

} // end anonymous namespace

*/
