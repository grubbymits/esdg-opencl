//===- LE1RegisterInfo.h - LE1 Register Information Impl ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LE1 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LE1REGISTERINFO_H
#define LE1REGISTERINFO_H

#include "LE1.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "LE1GenRegisterInfo.inc"

namespace llvm {
class LE1Subtarget;
class TargetInstrInfo;
class Type;

struct LE1RegisterInfo : public LE1GenRegisterInfo {
  const LE1Subtarget &Subtarget;
  const TargetInstrInfo &TII;

  LE1RegisterInfo(const LE1Subtarget &Subtarget, const TargetInstrInfo &tii);

  /// getRegisterNumbering - Given the enum value for some register, e.g.
  /// LE1::RA, return the number that it corresponds to (e.g. 31).
  //static unsigned getRegisterNumbering(unsigned RegEnum);

  /// Get PIC indirect call register
  //static unsigned getPICCallReg();

  /// Adjust the LE1 stack frame.
  void adjustLE1StackFrame(MachineFunction &MF) const;

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction* MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

  /// Stack Frame Processing Methods
  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = NULL) const;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

  /// Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const;

  /// Exception handling queries.
  unsigned getEHExceptionRegister() const;
  unsigned getEHHandlerRegister() const;
};

} // end namespace llvm

#endif
