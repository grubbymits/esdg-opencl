//===-- LE1AsmPrinter.h - LE1 LLVM assembly writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// LE1 Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LE1ASMPRINTER_H
#define LE1ASMPRINTER_H

#include "LE1Subtarget.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class MCStreamer;
class MachineInstr;
class raw_ostream;
class MachineBasicBlock;
class Module;
class GlobalVariable;

class LLVM_LIBRARY_VISIBILITY LE1AsmPrinter : public AsmPrinter {
  const LE1Subtarget *Subtarget;
  
public:
  explicit LE1AsmPrinter(TargetMachine &TM,  MCStreamer &Streamer)
    : AsmPrinter(TM, Streamer) {
    Subtarget = &TM.getSubtarget<LE1Subtarget>();
  }

  virtual const char *getPassName() const {
    return "LE1 Assembly Printer";
  }

  void EmitInstruction(const MachineInstr *MI);
  void printHex32(unsigned int Value, raw_ostream &O);
  virtual bool isBlockOnlyReachableByFallthrough(const MachineBasicBlock*
                                                 MBB) const;
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       unsigned AsmVariant, const char *ExtraCode,
                       raw_ostream &O);
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum,
                             unsigned AsmVariant, const char *ExtraCode,
                             raw_ostream &O);
  void printOperand(const MachineInstr *MI, int opNum, raw_ostream &O);
  void printUnsignedImm(const MachineInstr *MI, int opNum, raw_ostream &O);
  void printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &O);
  void printMemOperandEA(const MachineInstr *MI, int opNum, raw_ostream &O);
  virtual MachineLocation getDebugValueLocation(const MachineInstr *MI) const;
};
}

#endif

