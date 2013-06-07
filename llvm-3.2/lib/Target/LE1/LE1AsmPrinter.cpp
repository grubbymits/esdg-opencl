//===-- LE1AsmPrinter.cpp - LE1 LLVM assembly writer --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format LE1 assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "le1-asm-printer"
#include "LE1AsmPrinter.h"
#include "LE1.h"
#include "LE1InstrInfo.h"
#include "LE1MachineFunction.h"
#include "LE1MCInst.h"
#include "LE1MCInstLower.h"
#include "InstPrinter/LE1InstPrinter.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/Mangler.h"
//#include "llvm/Target/TargetData.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
//#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

//static bool isUnalignedLoadStore(unsigned Opc) {
//  return Opc == LE1::ULW    || Opc == LE1::ULH    || Opc == LE1::ULHu ||
//         Opc == LE1::USW    || Opc == LE1::USH;
//}

void LE1AsmPrinter::EmitInstruction(const MachineInstr *MI) {
  SmallString<128> Str;
  raw_svector_ostream OS(Str);

  //LE1MCInstLower MCInstLowering(Mang, *MF, *this);
  //MCInst Inst;
  //MCInstLowering.Lower(MI, Inst);
  //OutStreamer.EmitInstruction(Inst);
  //if (MI->isDebugValue()) {
    //PrintDebugValueComment(MI, OS);
    //return;
  //}

  
  LE1MCInstLower MCInstLowering(Mang, *MF, *this);
  if(MI->isBundle()) {
    std::vector<const MachineInstr*> BundledMIs;
    const MachineBasicBlock *MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI;
    ++MII;
    while(MII != MBB->end() && MII->isInsideBundle()) {
      const MachineInstr *TmpMI = MII;
      if(TmpMI->getOpcode() == TargetOpcode::DBG_VALUE ||
         TmpMI->getOpcode() == TargetOpcode::IMPLICIT_DEF ||
         TmpMI->isPseudo()) {
        ++MII;
        continue;
      }
      BundledMIs.push_back(TmpMI);
      ++MII;
    }

    int index=0;
    for(;index<BundledMIs.size()-1;++index) {
      LE1MCInst TmpInst;
      MCInstLowering.Lower(BundledMIs[index], TmpInst);
      OutStreamer.EmitInstruction(TmpInst);
    }
    LE1MCInst LastInst;
    MCInstLowering.Lower(BundledMIs[index], LastInst);
    LastInst.setFinalInst(true);
    OutStreamer.EmitInstruction(LastInst);
  }
  else {
    LE1MCInst SingleInst;
    MCInstLowering.Lower(MI, SingleInst);
    if(SingleInst.getOpcode() != LE1::CLK)
      SingleInst.setFinalInst(true);
    OutStreamer.EmitInstruction(SingleInst);
  }

}

//===----------------------------------------------------------------------===//
//
//  LE1 Asm Directives
//
//  -- Frame directive "frame Stackpointer, Stacksize, RARegister"
//  Describe the stack frame.
//
//  -- Mask directives "(f)mask  bitmask, offset"
//  Tells the assembler which registers are saved and where.
//  bitmask - contain a little endian bitset indicating which registers are
//            saved on function prologue (e.g. with a 0x80000000 mask, the
//            assembler knows the register 31 (RA) is saved at prologue.
//  offset  - the position before stack pointer subtraction indicating where
//            the first saved register on prologue is located. (e.g. with a
//
//  Consider the following function prologue:
//
//    .frame  $fp,48,$ra
//    .mask   0xc0000000,-8
//       addiu $sp, $sp, -48
//       sw $ra, 40($sp)
//       sw $fp, 36($sp)
//
//    With a 0xc0000000 mask, the assembler knows the register 31 (RA) and
//    30 (FP) are saved at prologue. As the save order on prologue is from
//    left to right, RA is saved first. A -8 offset means that after the
//    stack pointer subtration, the first register in the mask (RA) will be
//    saved at address 48-8=40.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Mask directives
//===----------------------------------------------------------------------===//


// Print a 32 bit hex number with all numbers.
void LE1AsmPrinter::printHex32(unsigned Value, raw_ostream &O) {
  O << "0x";
  for (int i = 7; i >= 0; i--)
    O << utohexstr((Value & (0xF << (i*4))) >> (i*4));
}

//===----------------------------------------------------------------------===//
// Frame and Set directives
//===----------------------------------------------------------------------===//


//void LE1AsmPrinter::EmitFunctionEntryLabel() {
  //OutStreamer.EmitRawText("\t.ent\t" + Twine(CurrentFnSym->getName()));
  //OutStreamer.EmitRawText("-- FUNC_" + Twine(CurrentFnSym->getName()));

  //OutStreamer.EmitLabel(CurrentFnSym);
//}

/// EmitFunctionBodyStart - Targets can override this to emit stuff before
/// the first basic block in the function.
//void LE1AsmPrinter::EmitFunctionBodyStart() {
  //emitFrameDirective();

  //SmallString<128> Str;
  //raw_svector_ostream OS(Str);
  //printSavedRegsBitmask(OS);
  //OutStreamer.EmitRawText(OS.str());
//}

/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
//void LE1AsmPrinter::EmitFunctionBodyEnd() {
  // There are instruction for this macros, but they must
  // always be at the function end, and we can't emit and
  // break with BB logic.
  //OutStreamer.EmitRawText(StringRef("\t.set\tmacro"));
  //OutStreamer.EmitRawText(StringRef("\t.set\treorder"));
  //OutStreamer.EmitRawText("\t.end\t" + Twine(CurrentFnSym->getName()));
//}


/// isBlockOnlyReachableByFallthough - Return true if the basic block has
/// exactly one predecessor and the control transfer mechanism between
/// the predecessor and this block is a fall-through.
bool LE1AsmPrinter::isBlockOnlyReachableByFallthrough(const MachineBasicBlock*
                                                       MBB) const {
  // The predecessor has to be immediately before this block.
  const MachineBasicBlock *Pred = *MBB->pred_begin();

  // If the predecessor is a switch statement, assume a jump table
  // implementation, so it is not a fall through.
  if (const BasicBlock *bb = Pred->getBasicBlock())
    if (isa<SwitchInst>(bb->getTerminator()))
      return false;

  // If this is a landing pad, it isn't a fall through.  If it has no preds,
  // then nothing falls through to it.
  if (MBB->isLandingPad() || MBB->pred_empty())
    return false;

  // If there isn't exactly one predecessor, it can't be a fall through.
  MachineBasicBlock::const_pred_iterator PI = MBB->pred_begin(), PI2 = PI;
  ++PI2;

  if (PI2 != MBB->pred_end())
    return false;

  // The predecessor has to be immediately before this block.
  if (!Pred->isLayoutSuccessor(MBB))
    return false;

  // If the block is completely empty, then it definitely does fall through.
  if (Pred->empty())
    return true;

  // Otherwise, check the last instruction.
  // Check if the last terminator is an unconditional branch.
  MachineBasicBlock::const_iterator I = Pred->end();
  while (I != Pred->begin() && !(--I)->getDesc().isTerminator()) ;

  return !I->getDesc().isBarrier();
}

// Print out an operand for an inline asm expression.
bool LE1AsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                     unsigned AsmVariant,const char *ExtraCode,
                                     raw_ostream &O) {
  // Does this asm operand have a single letter operand modifier?
  if (ExtraCode && ExtraCode[0])
    return true; // Unknown modifier.

  printOperand(MI, OpNo, O);
  return false;
}

bool LE1AsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                           unsigned OpNum, unsigned AsmVariant,
                                           const char *ExtraCode,
                                           raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
     return true; // Unknown modifier.

  const MachineOperand &MO = MI->getOperand(OpNum);
  assert(MO.isReg() && "unexpected inline asm memory operand");
  O << "0($" << LE1InstPrinter::getRegisterName(MO.getReg()) << ")";
  return false;
}

void LE1AsmPrinter::printOperand(const MachineInstr *MI, int opNum,
                                  raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(opNum);
  bool closeP = false;

  switch (MO.getType()) {
    case MachineOperand::MO_Register:
      //O << '$'
      //O  << LowercaseString(LE1InstPrinter::getRegisterName(MO.getReg()));
      O << StringRef(LE1InstPrinter::getRegisterName(MO.getReg()));
      break;

    case MachineOperand::MO_Immediate:
      O << MO.getImm();
      break;

    case MachineOperand::MO_MachineBasicBlock:
      O << *MO.getMBB()->getSymbol();
      return;

    case MachineOperand::MO_GlobalAddress:
      O << "[(";
      O << *Mang->getSymbol(MO.getGlobal());
      O << ")]";
      break;

    case MachineOperand::MO_BlockAddress: {
      MCSymbol* BA = GetBlockAddressSymbol(MO.getBlockAddress());
      O << BA->getName();
      break;
    }

    case MachineOperand::MO_ExternalSymbol:
      O << *GetExternalSymbolSymbol(MO.getSymbolName());
      break;

    case MachineOperand::MO_JumpTableIndex:
      O << MAI->getPrivateGlobalPrefix() << "JTI" << getFunctionNumber()
        << '_' << MO.getIndex();
      break;

    case MachineOperand::MO_ConstantPoolIndex:
      O << MAI->getPrivateGlobalPrefix() << "CPI"
        << getFunctionNumber() << "_" << MO.getIndex();
      if (MO.getOffset())
        O << "+" << MO.getOffset();
      break;

    default:
      llvm_unreachable("<unknown operand type>");
  }

  if (closeP) O << ")";
}

void LE1AsmPrinter::printUnsignedImm(const MachineInstr *MI, int opNum,
                                      raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(opNum);
  if (MO.isImm())
    O << (unsigned short int)MO.getImm();
  else
    printOperand(MI, opNum, O);
}

void LE1AsmPrinter::
printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &O) {
  // Load/Store memory operands -- imm($reg)
  // If PIC target the target is loaded as the
  // pattern lw $25,%call16($28)
  O << "[(";
  printOperand(MI, opNum, O);
  O << "+";
  printOperand(MI, opNum+1, O);
  O << ")]";
}

void LE1AsmPrinter::
printMemOperandEA(const MachineInstr *MI, int opNum, raw_ostream &O) {
  // when using stack locations for not load/store instructions
  // print the same way as all normal 3 operand instructions.
  printOperand(MI, opNum, O);
  O << ", ";
  printOperand(MI, opNum+1, O);
  return;
}

//void LE1AsmPrinter::
//printFCCOperand(const MachineInstr *MI, int opNum, raw_ostream &O,
  //              const char *Modifier) {
  //const MachineOperand& MO = MI->getOperand(opNum);
  //O << LE1::LE1FCCToString((LE1::CondCode)MO.getImm());
//}

//void LE1AsmPrinter::EmitStartOfAsmFile(Module &M) {
  // FIXME: Use SwitchSection.

  // Tell the assembler which ABI we are using
  //OutStreamer.EmitRawText("\t.section .mdebug." + Twine(getCurrentABIString()));

  // TODO: handle O64 ABI
  //if (Subtarget->isABI_EABI()) {
    //if (Subtarget->isGP32bit())
  //OutStreamer.EmitRawText(StringRef("\t.section .gcc_compiled_long32"));
  //  else
    //  OutStreamer.EmitRawText(StringRef("\t.section .gcc_compiled_long64"));
  //}

  // return to previous section
  //OutStreamer.EmitRawText(StringRef("\t.previous"));
//}

MachineLocation
LE1AsmPrinter::getDebugValueLocation(const MachineInstr *MI) const {
  // Handles frame addresses emitted in LE1InstrInfo::emitFrameIndexDebugValue.
  assert(MI->getNumOperands() == 4 && "Invalid no. of machine operands!");
  assert(MI->getOperand(0).isReg() && MI->getOperand(1).isImm() &&
         "Unexpected MachineOperand types");
  return MachineLocation(MI->getOperand(0).getReg(),
                         MI->getOperand(1).getImm());
}

//void LE1AsmPrinter::PrintDebugValueComment(const MachineInstr *MI,
  //                                         raw_ostream &OS) {
  // TODO: implement
//}

// Force static initialization.
extern "C" void LLVMInitializeLE1AsmPrinter() {
  RegisterAsmPrinter<LE1AsmPrinter> X(TheLE1Target);
 }
