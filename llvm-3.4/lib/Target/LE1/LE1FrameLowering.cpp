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

bool LE1FrameLowering::targetHandlesStackFrameRounding() const {
  return true;
}

static unsigned AlignOffset(unsigned Offset, unsigned Align) {
  return (Offset + Align - 1) / Align * Align;
}

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
                                          LE1::LNK);
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
    MRI.setPhysRegUsed(LE1::LNK);
  else
    MRI.setPhysRegUnused(LE1::LNK);
}

// Just delete ADJCALLSTACK pseudo instructions
void LE1FrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI) const {
  MBB.erase(MI);
}
