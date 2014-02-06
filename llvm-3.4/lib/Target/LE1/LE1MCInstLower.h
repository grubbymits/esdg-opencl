//===-- LE1MCInstLower.h - Lower MachineInstr to MCInst -------------------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LE1MCINSTLOWER_H
#define LE1MCINSTLOWER_H
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
  class MCAsmInfo;
  class MCContext;
  class MCInst;
  class MCOperand;
  class MCSymbol;
  class MachineInstr;
  class MachineFunction;
  class Mangler;
  class LE1AsmPrinter;
  
/// LE1MCInstLower - This class is used to lower an MachineInstr into an
//                    MCInst.
class LLVM_LIBRARY_VISIBILITY LE1MCInstLower {
  typedef MachineOperand::MachineOperandType MachineOperandType;
  MCContext &Ctx;
  Mangler *Mang;
  LE1AsmPrinter &AsmPrinter;
public:
  LE1MCInstLower(Mangler *mang, const MachineFunction &MF,
                  LE1AsmPrinter &asmprinter);  
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
private:
  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperandType MOTy, unsigned Offset) const;
  MCOperand LowerOperand(const MachineOperand& MO, int i) const;
};
}

#endif
