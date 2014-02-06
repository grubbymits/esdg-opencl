//===-- LE1MCInstLower.cpp - Convert LE1 MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower LE1 MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "LE1MCInstLower.h"
#include "LE1AsmPrinter.h"
#include "LE1InstrInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Target/Mangler.h"
using namespace llvm;
#include <iostream>

LE1MCInstLower::LE1MCInstLower(Mangler *mang, const MachineFunction &mf,
                                 LE1AsmPrinter &asmprinter)
  : Ctx(mf.getContext()), Mang(mang), AsmPrinter(asmprinter) {}

MCOperand LE1MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
  const MCSymbol *Symbol;

  switch (MOTy) {
    case MachineOperand::MO_MachineBasicBlock:
      Symbol = MO.getMBB()->getSymbol();
      break;

    case MachineOperand::MO_GlobalAddress:
      Symbol = AsmPrinter.getSymbol(MO.getGlobal());
      break;

    case MachineOperand::MO_BlockAddress:
      Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
      break;

    case MachineOperand::MO_ExternalSymbol:
      Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
      break;

    case MachineOperand::MO_JumpTableIndex:
      Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
      break;

    case MachineOperand::MO_ConstantPoolIndex:
      Symbol = AsmPrinter.GetCPISymbol(MO.getIndex());
      if (MO.getOffset())
        Offset += MO.getOffset();
      break;

    default:
      llvm_unreachable("<unknown operand type>");
  }

  return MCOperand::CreateExpr(MCSymbolRefExpr::Create(Symbol,
                                                       MCSymbolRefExpr::VK_None,
                                                       Ctx));
}

MCOperand LE1MCInstLower::LowerOperand(const MachineOperand& MO, int i) const {
  MachineOperandType MOTy = MO.getType();
  //MachineInstr *MI = MO.getParent();

  switch (MOTy) {
  default:
    assert(0 && "unknown operand type");
    break;
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit()) break;
    return MCOperand::CreateReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::CreateImm(MO.getImm());
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(MO, MOTy, 0);
 }

  return MCOperand();
}

void LE1MCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO, i);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
