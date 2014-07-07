#define DEBUG_TYPE "le1tti"
#include "LE1.h"
#include "LE1TargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/CostTable.h"

#include <iostream>

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

    virtual unsigned getIntImmCost(const APInt &Imm, Type *Ty) const;

    unsigned getMaximumUnrollFactor() const;

    unsigned getNumberOfRegisters(bool Vector) const;

    unsigned getRegisterBitWidth(bool Vector) const;

    unsigned
      getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy) const;

    unsigned
      getVectorInstrCost(unsigned Opcode, Type *ValTy, unsigned Index) const;

    unsigned getAddressComputationCost(Type *Val, bool IsComplex) const;

    unsigned getArithmeticInstrCost(unsigned Opcode, Type *Ty,
                                    OperandValueKind Op1Info = OK_AnyValue,
                                    OperandValueKind Op2Info = OK_AnyValue)
      const;

    unsigned getMemoryOpCost(unsigned Opcode, Type *Src, unsigned Alignment,
                             unsigned AddressSpace) const;

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
  std::cerr << "getNumberOfRegister" << std::endl;
  if (Vector)
    return 24;
  else
    return 60;
}

unsigned LE1TTI::getRegisterBitWidth(bool Vector) const {
  std::cerr << "getRegisterBitWidth" << std::endl;
  if (Vector)
    return 64;
  else
    return 32;
}

unsigned LE1TTI::getMaximumUnrollFactor() const {
  std::cerr << "getMaximumUnrollFactor" << std::endl;
  return 8;
}

unsigned LE1TTI::getIntImmCost(const APInt &Imm, Type *Ty) const {
  assert(Ty->isIntegerTy());

  std::cerr << "getIntImmCost" << std::endl;

  unsigned Bits = Ty->getPrimitiveSizeInBits();
  if (Bits > 16)
    return 4;
  else
    return 1;
}

unsigned LE1TTI::getAddressComputationCost(Type *Ty, bool isComplex) const {
  std::cerr << "getAddressComputationCost" << std::endl;
  return 1;
}

unsigned
LE1TTI::getCmpSelInstrCost(unsigned Opcode, Type *ValTy, Type *CondTy) const {
  std::cerr << "getCmpIselInstrCost" << std::endl;
  return 2;
}

unsigned
LE1TTI::getVectorInstrCost(unsigned Opcode, Type *ValTy, unsigned Index) const {
  std::cerr << "getVectorInstrCost" << std::endl;
  return 1;
}

unsigned
LE1TTI::getArithmeticInstrCost(unsigned Opcode, Type *Ty,
                               OperandValueKind Op1Info,
                               OperandValueKind Op2Info) const {
  std::cerr << "getArithmeticInstrCost" << std::endl;
  return 1;
}

unsigned LE1TTI::getMemoryOpCost(unsigned Opcode, Type *src, unsigned Alignment,
                                 unsigned AddressSpace) const {
  std::cerr << "getMemoryOpCost" << std::endl;
  return 1;
}
