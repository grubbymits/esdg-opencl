//===- LE1MCInst.h - LE1 sub-class of MCInst ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class extends MCInst to allow some VLIW annotation.
//
//===----------------------------------------------------------------------===//

#ifndef LE1NMCINST_H
#define LE1MCINST_H

#include "llvm/MC/MCInst.h"
#include "llvm/CodeGen/MachineInstr.h"

namespace llvm {
  class LE1MCInst: public MCInst {
    bool isFinal;
    const MachineInstr *MachineI;
  public:
    explicit LE1MCInst(): MCInst(),
                              isFinal(false) {}

    const MachineInstr* getMI() const { return MachineI; }

    void setMI(const MachineInstr *MI) { MachineI = MI; }

    void setFinalInst(bool yes) { isFinal = yes; }
    bool isFinalInst() const { return isFinal; }
  };
}

#endif
