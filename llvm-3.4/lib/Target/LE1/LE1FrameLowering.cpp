//=======- LE1FrameLowering.cpp - LE1 Frame Information ------*- C++ -*-====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LE1 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "LE1FrameLowering.h"
#include "LE1InstrInfo.h"
#include "LE1MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CommandLine.h"

//#include <iostream>

using namespace llvm;


//===----------------------------------------------------------------------===//
//
// Stack Frame Processing methods
// +----------------------------+
//
// The stack is allocated decrementing the stack pointer on
// the first instruction of a function prologue. Once decremented,
// all stack references are done thought a positive offset
// from the stack/frame pointer, so the stack is considering
// to grow up! Otherwise terrible hacks would have to be made
// to get this stack ABI compliant :)
//
//  The stack frame required by the ABI (after call):
//  Offset
//
//  0                 ----------
//  4                 Args to pass
//  .                 saved $GP  (used in PIC)
//  .                 Alloca allocations
//  .                 Local Area
//  .                 CPU "Callee Saved" Registers
//  .                 saved FP
//  .                 saved RA
//  .                 FPU "Callee Saved" Registers
//  StackSize         -----------
//
// Offset - offset from sp after stack allocation on function prologue
//
// The sp is the stack pointer subtracted/added from the stack size
// at the Prologue/Epilogue
//
// References to the previous stack (to obtain arguments) are done
// with offsets that exceeds the stack size: (stacksize+(4*(num_arg-1))
//
// Examples:
// - reference to the actual stack frame
//   for any local area var there is smt like : FI >= 0, StackOffset: 4
//     sw REGX, 4(SP)
//
// - reference to previous stack frame
//   suppose there's a load to the 5th arguments : FI < 0, StackOffset: 16.
//   The emitted instruction will be something like:
//     lw REGX, 16+StackSize(SP)
//
// Since the total stack size is unknown on LowerFormalArguments, all
// stack references (ObjectOffset) created to reference the function
// arguments, are negative numbers. This way, on eliminateFrameIndex it's
// possible to detect those references and the offsets are adjusted to
// their real location.
//
//===----------------------------------------------------------------------===//

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
//bool LE1FrameLowering::hasFP(const MachineFunction &MF) const {
  //const MachineFrameInfo *MFI = MF.getFrameInfo();
  //return DisableFramePointerElim(MF) || MFI->hasVarSizedObjects()
    //  || MFI->isFrameAddressTaken();
//}

bool LE1FrameLowering::targetHandlesStackFrameRounding() const {
  return true;
}

static unsigned AlignOffset(unsigned Offset, unsigned Align) {
  return (Offset + Align - 1) / Align * Align; 
} 

// expand pair of register and immediate if the immediate doesn't fit in the
// 16-bit offset field.
// e.g.
//  if OrigImm = 0x10000, OrigReg = $sp:
//    generate the following sequence of instrs:
//      lui  $at, hi(0x10000)
//      addu $at, $sp, $at
//
//    (NewReg, NewImm) = ($at, lo(Ox10000))
//    return true
//static bool expandRegLargeImmPair(unsigned OrigReg, int OrigImm,
  //                                unsigned& NewReg, int& NewImm,
    //                              MachineBasicBlock& MBB,
      //                            MachineBasicBlock::iterator I) {
  // OrigImm fits in the 16-bit field
  //if (OrigImm < 0x8000 && OrigImm >= -0x8000) {
    //NewReg = OrigReg;
    //NewImm = OrigImm;
    //return false;
  //}

  //MachineFunction* MF = MBB.getParent();
  //const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();
  //DebugLoc DL = I->getDebugLoc();
  //int ImmLo = (short)(OrigImm & 0xffff);
  //int ImmHi = (((unsigned)OrigImm & 0xffff0000) >> 16) +
    //          ((OrigImm & 0x8000) != 0);

  // FIXME: change this when le1 goes MC".
  //BuildMI(MBB, I, DL, TII->get(LE1::NOAT));
  //BuildMI(MBB, I, DL, TII->get(LE1::LUi), LE1::T45).addImm(ImmHi);
  //BuildMI(MBB, I, DL, TII->get(LE1::ADDu), LE1::T45).addReg(OrigReg)
    //                                                 .addReg(LE1::T45);
  //NewReg = LE1::T45;
  //NewImm = ImmLo;

  //return true;
//}

void LE1FrameLowering::emitPrologue(MachineFunction &MF) const {
  //std::cout << "Entering LE1FrameLowering::emitPrologue\n";
  MachineBasicBlock &MBB   = MF.front();
  MachineFrameInfo *MFI    = MF.getFrameInfo();
  LE1FunctionInfo *LE1FI = MF.getInfo<LE1FunctionInfo>();
  //const LE1RegisterInfo *RegInfo =
    //static_cast<const LE1RegisterInfo*>(MF.getTarget().getRegisterInfo());
  const LE1InstrInfo &TII =
    *static_cast<const LE1InstrInfo*>(MF.getTarget().getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  //bool isPIC = (MF.getTarget().getRelocationModel() == Reloc::PIC_);

  // First, compute final stack size.
  unsigned RegSize = 4; 
  unsigned StackAlign = getStackAlignment();
  unsigned LocalVarAreaOffset = LE1FI->needGPSaveRestore() ? 
    (MFI->getObjectOffset(LE1FI->getGPFI()) + RegSize) :
    LE1FI->getMaxCallFrameSize();
  unsigned StackSize = AlignOffset(LocalVarAreaOffset, StackAlign) +
    AlignOffset(MFI->getStackSize(), StackAlign);

   // Update stack size
  MFI->setStackSize(StackSize); 

  //BuildMI(MBB, MBBI, dl, TII.get(LE1::NOREORDER));

  // TODO: check need from GP here.
  //if (isPIC && STI.isABI_O32())
    //BuildMI(MBB, MBBI, dl, TII.get(LE1::CPLOAD))
      //.addReg(RegInfo->getPICCallReg());
  //BuildMI(MBB, MBBI, dl, TII.get(LE1::NOMACRO));

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI->adjustsStack()) return;

  /*
  MachineModuleInfo &MMI = MF.getMMI();
  std::vector<MachineMove> &Moves = MMI.getFrameMoves();
  MachineLocation DstML, SrcML;*/

  if (StackSize > 1023)
    BuildMI(MBB, MBBI, dl, TII.get(LE1::SUBi32), LE1::SP)
      .addReg(LE1::SP).addImm(StackSize);
  else
    BuildMI(MBB, MBBI, dl, TII.get(LE1::SUBi9), LE1::SP)
      .addReg(LE1::SP).addImm(StackSize);
}

void LE1FrameLowering::emitEpilogue(MachineFunction &MF,
                                 MachineBasicBlock &MBB) const {
  //std::cout << "Entering LE1FrameLowering::emitEpilogue\n";
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo *MFI            = MF.getFrameInfo();
  const LE1InstrInfo &TII =
    *static_cast<const LE1InstrInfo*>(MF.getTarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();

  if(MF.getFunction()->getName() == "main") {
    MachineInstrBuilder Builder = BuildMI(MBB, MBBI, dl, TII.get(LE1::Exit),
                                          LE1::L0);
    MBBI->eraseFromParent();
    // TODO Handle the fact that the stack restore instruction isn't handled
  }
  else {
    // Get the number of bytes from FrameInfo
    unsigned StackSize = MFI->getStackSize();
    MachineBasicBlock::reverse_iterator RI = MBB.rbegin();
    MachineBasicBlock::reverse_iterator REnd = MBB.rend();
    while(RI->isDebugValue() && RI != REnd)
      ++RI;

    assert(RI->getOpcode() == LE1::Ret && "Last instruction isn't ret");
    MachineOperand StackImm = RI->getOperand(1);
    MachineOperand LinkReg = RI->getOperand(2);
    StackImm.setImm(StackSize);
    RI->RemoveOperand(2);
    RI->RemoveOperand(1);
    RI->addOperand(StackImm);
    RI->addOperand(LinkReg);


    /*
    MachineInstrBuilder Builder = BuildMI(MBB, MBBI, dl, TII.get(LE1::Ret), 
                                          LE1::SP);
    Builder.addReg(LE1::SP).addImm(StackSize);
    Builder.addReg(LE1::L0);*/

      // FIXME: change this when le1 goes MC".
      //if (ATUsed)
        //BuildMI(MBB, MBBI, dl, TII.get(LE1::ATMACRO));
    //}
  }

  //std::cout << "Leaving LE1FrameLowering::emitEpilogue\n";
}

void LE1FrameLowering::
processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                     RegScavenger *RS) const {
  MachineRegisterInfo& MRI = MF.getRegInfo();

  // FIXME: remove this code if register allocator can correctly mark
  //        $fp and $ra used or unused.

  // Mark $fp and $ra as used or unused.
  //if (hasFP(MF))
    //MRI.setPhysRegUsed(LE1::STRP);

  // The register allocator might determine $ra is used after seeing 
  // instruction "jr $ra", but we do not want PrologEpilogInserter to insert
  // instructions to save/restore $ra unless there is a function call.
  // To correct this, $ra is explicitly marked unused if there is no
  // function call.
  if (MF.getFrameInfo()->hasCalls())
    MRI.setPhysRegUsed(LE1::L0);
  else
    MRI.setPhysRegUnused(LE1::L0);
}

// Just delete ADJCALLSTACK pseudo instructions
void LE1FrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI) const {
  MBB.erase(MI);
}
