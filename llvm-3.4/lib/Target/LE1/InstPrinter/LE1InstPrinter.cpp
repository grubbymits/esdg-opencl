//===-- LE1InstPrinter.cpp - Convert LE1 MCInst to assembly syntax ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an LE1 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "LE1InstPrinter.h"
#include "LE1MCInst.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringExtras.h"

//#include <iostream>

using namespace llvm;

#define GET_INSTRUCTION_NAME
#include "LE1GenAsmWriter.inc"
/*
StringRef LE1InstPrinter::getOpcodeName(unsigned Opcode) const {
  return getInstructionName(Opcode);
}*/

void LE1InstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  OS << StringRef(getRegisterName(RegNo));
}

void LE1InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                StringRef Annot) {
  printInst((const LE1MCInst*)(MI), O, Annot);

}

void LE1InstPrinter::printInst(const LE1MCInst *MI, raw_ostream &O,
                               StringRef Annot) {
  printInstruction(MI, O);
  if(MI->isFinalInst())
    O << "\n\t;;";
  printAnnotation(O, Annot);
}

void LE1InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    if(Op.getImm() >= 0) {
      O << "0x";
      O.write_hex(Op.getImm());
    }
    else {
      O << "-0x";
      O.write_hex((-1 * Op.getImm()));
    }
    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  O << *Op.getExpr();
}

void LE1InstPrinter::printUnsignedImm(const MCInst *MI, int opNum,
                                       raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm()) {
    O << "0x";
    O.write_hex(MO.getImm());
  }
  else
    printOperand(MI, opNum, O);
}

void LE1InstPrinter::
printMemOperand(const MCInst *MI, int opNum, raw_ostream &O) {

  if(MI->getOperand(opNum).isReg()) {
    printOperand(MI, opNum, O);
    O << "[";
    printOperand(MI, opNum+1, O);
    O << "]";
  }
  else {
    // Way for us to load from global address
    // TODO Need to be this properly so r0 is encoded in the instruction
    O << "r0.0[(";
    printOperand(MI, opNum, O);
    O << "+";
    printOperand(MI, opNum+1, O);
    O <<")]";
  }
}

void LE1InstPrinter::
printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O) {
  // when using stack locations for not load/store instructions
  // print the same way as all normal 3 operand instructions.
  printOperand(MI, opNum, O);
  O << ", ";
  printOperand(MI, opNum+1, O);
  return;
}

void LE1InstPrinter::
printGlobalOffset(const MCInst *MI, int opNum, raw_ostream &O) {
  O << MI->getOperand(opNum).getImm();
}

//void LE1InstPrinter::
//printFCCOperand(const MCInst *MI, int opNum, raw_ostream &O) {
  //const MCOperand& MO = MI->getOperand(opNum);
  //O << LE1FCCToString((LE1::CondCode)MO.getImm());
//}
