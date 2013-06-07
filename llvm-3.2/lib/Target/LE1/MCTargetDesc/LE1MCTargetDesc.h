//===-- LE1MCTargetDesc.h - LE1 Target Descriptions -----------*- C++ -*-===//
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

#ifndef LE1MCTARGETDESC_H
#define LE1MCTARGETDESC_H

namespace llvm {
class MCAsmBackend;
class MCInstrInfo;
class MCRegisterInfo;
class MCCodeEmitter;
class MCContext;
class MCSubtargetInfo;
class StringRef;
class Target;

extern Target TheLE1Target;
//extern Target TheLE1elTarget;
//extern Target TheLE164Target;
//extern Target TheLE164elTarget;

MCCodeEmitter *createLE1MCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      const MCSubtargetInfo &STI,
                                      MCContext &Ctx);

//MCAsmBackend *createLE1AsmBackend(const Target &T, StringRef TT);
} // End llvm namespace

// Defines symbolic names for LE1 registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "LE1GenRegisterInfo.inc"

// Defines symbolic names for the LE1 instructions.
#define GET_INSTRINFO_ENUM
#include "LE1GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "LE1GenSubtargetInfo.inc"

#endif
