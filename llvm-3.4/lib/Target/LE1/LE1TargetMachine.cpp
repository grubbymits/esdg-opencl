//===-- LE1TargetMachine.cpp - Define TargetMachine for LE1 -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about LE1 target spec.
//
//===----------------------------------------------------------------------===//

#include "LE1.h"
#include "LE1TargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/PassSupport.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include <iostream>

using namespace llvm;

extern "C" void LLVMInitializeLE1Target() {
  // Register the target.
  //RegisterTargetMachine<LE1v1TargetMachine> X(TheLE1Target);
  RegisterTargetMachine<LE1TargetMachine> X(TheLE1Target);
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned 
// FIXME needs to be 32 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
LE1TargetMachine::
LE1TargetMachine(const Target &T, StringRef TT,
                  StringRef CPU, StringRef FS, const TargetOptions &Options,
                  Reloc::Model RM, CodeModel::Model CM,
                  CodeGenOpt::Level OL):
  LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
  DL("E-p:32:32:32-S32-i8:8-i16:16:-i32:32:32-i64:64:64-n32-a:0:32"),
  Subtarget(TT, CPU, FS),
  InstrInfo(Subtarget),
  TLInfo(*this), TSInfo(*this),
  FrameLowering(Subtarget),
  InstrItins(Subtarget.getInstrItineraryData()) {
    initAsmInfo();
    this->Options.FloatABIType = FloatABI::Soft;
}

bool LE1TargetMachine::addPassesForOptimizations(PassManagerBase &PM) {
  /*
  PM.add(createPromoteMemoryToRegisterPass());
  PM.add(createConstantPropagationPass());
  PM.add(createDeadStoreEliminationPass());
  PM.add(createConstantPropagationPass());
  PM.add(createCFGSimplificationPass());
  PM.add(createIndVarSimplifyPass());
  PM.add(createLoopSimplifyPass());
  PM.add(createLoopRotatePass());
  PM.add(createLoopUnswitchPass());
  PM.add(createLoopUnrollPass(10, 2, 1));*/
  return true;
}

namespace {
class LE1PassConfig : public TargetPassConfig {
public:
  LE1PassConfig(LE1TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {
      enablePass(&MachineSchedulerID);
      setEnableTailMerge(true);
    }

  LE1TargetMachine &getLE1TargetMachine() const {
    //std::cout << "getting LE1TargetMachine\n";
    return getTM<LE1TargetMachine>();
  }

  const LE1Subtarget &getLE1Subtarget() const {
    return *getLE1TargetMachine().getSubtargetImpl();
  }

  virtual bool addPreISel();
  virtual bool addInstSelector();
  virtual void addMachineSSAOptimization();
  virtual bool addPreSched2();
  virtual bool addPreRegAlloc();
  virtual bool addPostRegAlloc();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *LE1TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new LE1PassConfig(this, PM);
}
// Install an instruction selector pass using
// the ISelDag to gen LE1 code.
bool LE1PassConfig::addInstSelector()
{
  //std::cout << "adding InstSelector\n";
  //addPass(createLE1RemoveExtendOps(getLE1TargetMachine()));
  addPass(createLE1ISelDag(getLE1TargetMachine()));
  return false;
}

bool LE1PassConfig::addPreISel() {
  addPass(createPromoteMemoryToRegisterPass());
  addPass(createConstantPropagationPass());
  addPass(createCFGSimplificationPass());
  addPass(createDeadCodeEliminationPass());
  //addPass(createBlockPlacementPass());
  addPass(createDeadStoreEliminationPass());
  addPass(createConstantPropagationPass());
  addPass(createIndVarSimplifyPass());
  addPass(createLoopSimplifyPass());
  addPass(createLoopRotatePass());
  addPass(createLoopUnswitchPass());
  //addPass(createLoopUnrollPass(10, 2, 1));
  return true;
}

void LE1PassConfig::addMachineSSAOptimization() {
  addPass(&EarlyTailDuplicateID);
}

bool LE1PassConfig::addPreSched2() {
  addPass(&TailDuplicateID);
  return false;
}

bool LE1PassConfig::addPreRegAlloc() {
  //addPass(createLE1BBMerger(getLE1TargetMachine()));
  addPass(&MachineBlockPlacementID);
  addPass(&DeadMachineInstructionElimID);
  return false;
}

bool LE1PassConfig::addPostRegAlloc() {
  return false;
}

bool LE1PassConfig::addPreEmitPass()
{
  addPass(createLE1ExpandPredSpillCode(getLE1TargetMachine()));
  addPass(createLE1Packetizer(getLE1TargetMachine()));
  return false;
}

