//===-- LE1MCTargetDesc.cpp - LE1 Target Descriptions ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides LE1 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "LE1MCTargetDesc.h"
#include "LE1MCAsmInfo.h"
#include "InstPrinter/LE1InstPrinter.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "LE1GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LE1GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LE1GenRegisterInfo.inc"

#include <iostream>

using namespace llvm;

static MCInstrInfo *createLE1MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitLE1MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createLE1MCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitLE1MCRegisterInfo(X, LE1::L0);
  return X;
}

static MCSubtargetInfo *createLE1MCSubtargetInfo(StringRef TT, StringRef CPU,
                                                  StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitLE1MCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCAsmInfo *createLE1MCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
  std::cout << "createLE1MCAsmInfo" << std::endl;
  MCAsmInfo *MAI = new LE1MCAsmInfo(MRI, TT);

  //MachineLocation Dst(MachineLocation::VirtualFP);
  //MachineLocation Src(LE1::SP, 0);
  //MAI->addInitialFrameState(0, Dst, Src);

  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(
    0, LE1::SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCCodeGenInfo *createLE1MCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                              CodeModel::Model CM,
                                              CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (RM == Reloc::Default)
    RM = Reloc::PIC_;
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createLE1MCInstPrinter(const Target &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI,
                                              const MCSubtargetInfo &STI) {
  return new LE1InstPrinter(MAI, MII, MRI);
}

/*
static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
                                    MCContext &Ctx, MCAsmBackend &MAB,
                                    raw_ostream &_OS,
                                    MCCodeEmitter *_Emitter,
                                    bool RelaxAll,
                                    bool NoExecStack) {
  Triple TheTriple(TT);

  return createELFStreamer(Ctx, MAB, _OS, _Emitter, RelaxAll, NoExecStack);
}*/

extern "C" void LLVMInitializeLE1TargetMC() {
  std::cout << "LLVMInitializeLE1TargetMC" << std::endl;
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheLE1Target, createLE1MCAsmInfo);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheLE1Target,
                                        createLE1MCCodeGenInfo);
    // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheLE1Target, createLE1MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheLE1Target, createLE1MCRegisterInfo);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(TheLE1Target, createLE1MCCodeEmitter);

  // Register the object streamer.
  //TargetRegistry::RegisterMCObjectStreamer(TheLE1Target, createMCStreamer);

  // Register the asm backend.
  //TargetRegistry::RegisterMCAsmBackend(TheLE1Target, createLE1AsmBackend);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheLE1Target,
                                          createLE1MCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheLE1Target,
                                        createLE1MCInstPrinter);
  }
