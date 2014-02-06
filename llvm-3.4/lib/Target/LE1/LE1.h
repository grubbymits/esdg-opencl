//===-- LE1.h - Top-level interface for LE1 representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM LE1 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_LE1_H
#define TARGET_LE1_H

#include "MCTargetDesc/LE1MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class LE1TargetMachine;
  class FunctionPass;
  class MachineCodeEmitter;
  class formatted_raw_ostream;

  FunctionPass *createLE1RemoveExtendOps(LE1TargetMachine &TM);
  FunctionPass *createLE1ISelDag(LE1TargetMachine &TM);
  FunctionPass *createLE1BBMerger(LE1TargetMachine &TM);
  //FunctionPass *createLE1DelaySlotFillerPass(LE1TargetMachine &TM);
  FunctionPass *createLE1Packetizer(LE1TargetMachine &TM);
  //FunctionPass *createLE1ExpandPseudoPass(LE1TargetMachine &TM);
  FunctionPass *createLE1CFGOptimiser(LE1TargetMachine &TM);
  FunctionPass *createLE1ExpandPredSpillCode(LE1TargetMachine &TM);
  //FunctionPass *createLE1EmitGPRestorePass(LE1TargetMachine &TM);

  //FunctionPass *createLE1JITCodeEmitterPass(LE1TargetMachine &TM,
    //                                         JITCodeEmitter &JCE);

} // end namespace llvm;

#endif
