//==--- LE1FrameLowering.h - Define frame lowering for LE1 --*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef LE1_FRAMEINFO_H
#define LE1_FRAMEINFO_H

#include "LE1.h"
#include "LE1Subtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class LE1Subtarget;

class LE1FrameLowering : public TargetFrameLowering {
protected:
  const LE1Subtarget &STI;

public:
  explicit LE1FrameLowering(const LE1Subtarget &sti)
    : TargetFrameLowering(StackGrowsDown, 8, 0),
      STI(sti) {
  }

  bool targetHandlesStackFrameRounding() const;

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

  bool hasFP(const MachineFunction &MF) const { return false; };

  void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                            RegScavenger *RS) const;
};

} // End llvm namespace

#endif
