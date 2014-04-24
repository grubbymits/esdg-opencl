//===- LE1InstrInfo.cpp - LE1 Instruction Information ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LE1 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "LE1InstrInfo.h"
#include "LE1TargetMachine.h"
#include "LE1MachineFunction.h"
#include "InstPrinter/LE1InstPrinter.h"
#include "llvm/CodeGen/DFAPacketizer.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/ScoreboardHazardRecognizer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/STLExtras.h"

#include <string>
#include <iostream>

#define GET_INSTRINFO_CTOR_DTOR
#include "LE1GenInstrInfo.inc"
//#include "LE1GenDFAPacketizer.inc"

using namespace llvm;

LE1InstrInfo::LE1InstrInfo(LE1Subtarget &ST)
  : LE1GenInstrInfo(LE1::ADJCALLSTACKDOWN, LE1::ADJCALLSTACKUP),
    RI(ST, *this), Subtarget(ST) {}


const LE1RegisterInfo &LE1InstrInfo::getRegisterInfo() const { 
  return RI;
}

static bool isZeroImm(const MachineOperand &op) {
  return op.isImm() && op.getImm() == 0;
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned LE1InstrInfo::
isLoadFromStackSlot(const MachineInstr *MI, int &FrameIndex) const
{
  unsigned Opc = MI->getOpcode();

  //if (Opc == LE1::LDW) {
  if (MI->mayLoad()) {
    if ((MI->getOperand(1).isFI()) && // is a stack slot
        (MI->getOperand(2).isImm()) &&  // the imm is zero
        (isZeroImm(MI->getOperand(2)))) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }

  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned LE1InstrInfo::
isStoreToStackSlot(const MachineInstr *MI, int &FrameIndex) const
{
  unsigned Opc = MI->getOpcode();

  //if (Opc == LE1::STW) {
  if (MI->mayStore()) {
    if ((MI->getOperand(1).isFI()) && // is a stack slot
        (MI->getOperand(2).isImm()) &&  // the imm is zero
        (isZeroImm(MI->getOperand(2)))) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }
  return 0;
}

void LE1InstrInfo::
copyPhysReg(MachineBasicBlock &MBB,
            MachineBasicBlock::iterator I, DebugLoc DL,
            unsigned DestReg, unsigned SrcReg,
            bool KillSrc) const {
  //std::cout << "Entering copyPhysReg\n";
  unsigned Opc = 0, ZeroReg = 0;

  if (LE1::CPURegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
    if (LE1::CPURegsRegClass.contains(SrcReg))
      Opc = LE1::MOVr;
    else if (LE1::BRegsRegClass.contains(SrcReg)) {
      //std::cout << "Moving from branch\n";
      Opc = LE1::MFB;
    }
    else if (LE1::LRegRegClass.contains(SrcReg)) {
      //std::cout << "Moving from Link\n";
      Opc = LE1::MFL;
    }
  }
  else if (LE1::CPURegsRegClass.contains(SrcReg)) { // Copy from CPU Reg.
    if(LE1::BRegsRegClass.contains(DestReg)) {
      //std::cout << "Moving to branch\n";
      Opc = LE1::MTB;
    }
    else 
    if(LE1::LRegRegClass.contains(DestReg)) {
      //std::cout << "Moving from link\n";
      Opc = LE1::MTL;
    }
  }
  else if(LE1::BRegsRegClass.contains(SrcReg, DestReg)) {
    /*unsigned TmpReg = MBB.getParent()->getRegInfo().
      createVirtualRegister(&LE1::CPURegsRegClass);*/
    unsigned TmpReg = LE1::T45;
    MachineInstrBuilder MIB1 = BuildMI(MBB, I, DL, get(LE1::MFB), TmpReg);
    MIB1.addReg(SrcReg, getKillRegState(KillSrc));
    MachineInstrBuilder MIB2 = BuildMI(MBB, I, DL, get(LE1::MTB), DestReg);
    MIB2.addReg(TmpReg, RegState::Kill);
    return;
  }
  
  assert(Opc && "Cannot copy registers");

  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));
  
  if (DestReg)
    MIB.addReg(DestReg, RegState::Define);

  if (ZeroReg)
    MIB.addReg(ZeroReg);

  if (SrcReg)
    MIB.addReg(SrcReg, getKillRegState(KillSrc));

  //std::cout << "Leaving copyPhysReg\n";
}

static MachineMemOperand*
GetMemOperand(MachineBasicBlock &MBB, int FI, unsigned MemOp) {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();
  unsigned Align = MFI.getObjectAlignment(FI);

  return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI), MemOp,
                                 MFI.getObjectSize(FI), Align);
}

void LE1InstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);
  unsigned Opc = 0;

  if (LE1::CPURegsRegClass.hasSubClassEq(RC) ||
      LE1::LRegRegClass.hasSubClassEq(RC)) {
    Opc = LE1::STWi8;
    BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
  }
  else if (LE1::BRegsRegClass.hasSubClassEq(RC)) {
    Opc = LE1::STW_PRED;
    BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
                                 .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
  }
  assert(Opc && "Register class not handled!");
}

void LE1InstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const
{
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
  unsigned Opc = 0;

  if (LE1::CPURegsRegClass.hasSubClassEq(RC) ||
      LE1::LRegRegClass.hasSubClassEq(RC)) {
    if (FI == (FI & 0xFF))
      Opc = LE1::LDWi8;
    else if (FI == (FI & 0xFFF))
      Opc = LE1::LDWi12;
    else
      Opc = LE1::LDWi32;
    BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  }
  else if (LE1::BRegsRegClass.hasSubClassEq(RC)) {
    Opc = LE1::LDW_PRED;
    BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  }
  assert(Opc && "Register class not handled!");
}

MachineInstr*
LE1InstrInfo::emitFrameIndexDebugValue(MachineFunction &MF, int FrameIx,
                                        uint64_t Offset, const MDNode *MDPtr,
                                        DebugLoc DL) const {
  MachineInstrBuilder MIB = BuildMI(MF, DL, get(LE1::DBG_VALUE))
    .addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
  return &*MIB;
}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//

static unsigned GetAnalyzableBrOpc(unsigned Opc) {
  return (Opc == LE1::BR || Opc == LE1::BRF || Opc == LE1::GOTO) ? Opc : 0;
}

/// GetOppositeBranchOpc - Return the inverse of the specified
/// opcode, e.g. turning BEQ to BNE.
unsigned LE1::GetOppositeBranchOpc(unsigned Opc)
{
  switch (Opc) {
  default: llvm_unreachable("Illegal opcode!");
  case LE1::BR    : return LE1::BRF;
  case LE1::BRF    : return LE1::BR;
  }
}

static void AnalyzeCondBr(const MachineInstr* Inst, unsigned Opc,
                          MachineBasicBlock *&BB,
                          SmallVectorImpl<MachineOperand>& Cond) {
  assert(GetAnalyzableBrOpc(Opc) && "Not an analyzable branch");
  int NumOp = Inst->getNumExplicitOperands();

  // for both int and fp branches, the last explicit operand is the
  // MBB.
  BB = Inst->getOperand(NumOp-1).getMBB();
  Cond.push_back(MachineOperand::CreateImm(Opc));

  for (int i=0; i<NumOp-1; i++)
    Cond.push_back(Inst->getOperand(i));
}

bool LE1InstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const
{
  MachineBasicBlock::reverse_iterator I = MBB.rbegin(), REnd = MBB.rend();

  // Skip all the debug instructions.
  while (I != REnd && I->isDebugValue())
    ++I;

  if (I == REnd || !isUnpredicatedTerminator(&*I)) {
    // If this block ends with no branches (it just falls through to its succ)
    // just return false, leaving TBB/FBB null.
    TBB = FBB = NULL;
    return false;
  }

  MachineInstr *LastInst = &*I;
  unsigned LastOpc = LastInst->getOpcode();

  // Not an analyzable branch (must be an indirect jump).
  if (!GetAnalyzableBrOpc(LastOpc)) {
    return true;
  }

  // Get the second to last instruction in the block.
  unsigned SecondLastOpc = 0;
  MachineInstr *SecondLastInst = NULL;

  if (++I != REnd) {
    SecondLastInst = &*I;
    SecondLastOpc = GetAnalyzableBrOpc(SecondLastInst->getOpcode());

    // Not an analyzable branch (must be an indirect jump).
    if (isUnpredicatedTerminator(SecondLastInst) && !SecondLastOpc) {
      return true;
    }
  }

  // If there is only one terminator instruction, process it.
  if (!SecondLastOpc) {
    // Unconditional branch
    if (LastOpc == LE1::GOTO) {
      TBB = LastInst->getOperand(0).getMBB();
      // If the basic block is next, remove the GOTO inst
      if(MBB.isLayoutSuccessor(TBB)) {
        LastInst->eraseFromParent();
      }

      return false;
    }

    // Conditional branch
    AnalyzeCondBr(LastInst, LastOpc, TBB, Cond);
    return false;
  }

  // If we reached here, there are two branches.
  // If there are three terminators, we don't know what sort of block this is.
  if (++I != REnd && isUnpredicatedTerminator(&*I)) {
    return true;
  }

  // If second to last instruction is an unconditional branch,
  // analyze it and remove the last instruction.
  if (SecondLastOpc == LE1::GOTO) {
    // Return if the last instruction cannot be removed.
    if (!AllowModify) {
      return true;
    }

    TBB = SecondLastInst->getOperand(0).getMBB();
    LastInst->eraseFromParent();
    return false;
  }

  // Conditional branch followed by an unconditional branch.
  // The last one must be unconditional.
  if (LastOpc != LE1::GOTO) {
    return true;
  }

  AnalyzeCondBr(SecondLastInst, SecondLastOpc, TBB, Cond);
  FBB = LastInst->getOperand(0).getMBB();

  return false;
}

void LE1InstrInfo::BuildCondBr(MachineBasicBlock &MBB,
                                MachineBasicBlock *TBB, DebugLoc DL,
                                const SmallVectorImpl<MachineOperand>& Cond)
  const {
  unsigned Opc = Cond[0].getImm();
  const MCInstrDesc &MCID = get(Opc);
  MachineInstrBuilder MIB = BuildMI(&MBB, DL, MCID);

  for (unsigned i = 1; i < Cond.size(); ++i)
    MIB.addReg(Cond[i].getReg());

  MIB.addMBB(TBB);
}

unsigned LE1InstrInfo::
InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
             MachineBasicBlock *FBB,
             const SmallVectorImpl<MachineOperand> &Cond,
             DebugLoc DL) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");

  // # of condition operands:
  //  Unconditional branches: 0
  //  Floating point branches: 1 (opc)
  //  Int BranchZero: 2 (opc, reg)
  //  Int Branch: 3 (opc, reg0, reg1)
  assert((Cond.size() <= 3) &&
         "# of LE1 branch conditions must be <= 3!");

  // Two-way Conditional branch.
  if (FBB) {
    BuildCondBr(MBB, TBB, DL, Cond);
    BuildMI(&MBB, DL, get(LE1::GOTO)).addMBB(FBB);
    return 2;
  }

  // One way branch.
  // Unconditional branch.
  if (Cond.empty())
    BuildMI(&MBB, DL, get(LE1::GOTO)).addMBB(TBB);
  else // Conditional branch.
    BuildCondBr(MBB, TBB, DL, Cond);
  return 1;
}

unsigned LE1InstrInfo::
RemoveBranch(MachineBasicBlock &MBB) const
{
  MachineBasicBlock::reverse_iterator I = MBB.rbegin(), REnd = MBB.rend();
  MachineBasicBlock::reverse_iterator FirstBr;
  unsigned removed;

  // Skip all the debug instructions.
  while (I != REnd && I->isDebugValue())
    ++I;

  FirstBr = I;

  // Up to 2 branches are removed.
  // Note that indirect branches are not removed.
  for(removed = 0; I != REnd && removed < 2; ++I, ++removed)
    if (!GetAnalyzableBrOpc(I->getOpcode()))
      break;

  MBB.erase(I.base(), FirstBr.base());

  return removed;
}

/// ReverseBranchCondition - Return the inverse opcode of the
/// specified Branch instruction.
bool LE1InstrInfo::
ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const
{
  assert( (Cond.size() && Cond.size() <= 3) &&
          "Invalid LE1 branch condition!");
  Cond[0].setImm(LE1::GetOppositeBranchOpc(Cond[0].getImm()));
  return false;
}

void LE1InstrInfo::insertNoop(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI) const {
  DebugLoc dl;
  BuildMI(MBB, MI, dl, get(LE1::CLK));
}

/*
DFAPacketizer *LE1InstrInfo::
CreateTargetScheduleState(const TargetMachine *TM,
                           const ScheduleDAG *DAG) const {
  //const InstrItineraryData *II = Subtarget.getInstrItineraryData();
  const InstrItineraryData *II = TM->getInstrItineraryData();
  return TM->getSubtarget<LE1GenSubtargetInfo>().createDFAPacketizer(II);
}
*/

ScheduleHazardRecognizer *LE1InstrInfo::
CreateTargetHazardRecognizer(const TargetMachine *TM,
                             const ScheduleDAG *DAG) const {
  if (usePreRAHazardRecognizer()) {
    const InstrItineraryData *II = Subtarget.getInstrItineraryData();
    return new ScoreboardHazardRecognizer(II, DAG, "sched-instrs");
  }
  return TargetInstrInfo::CreateTargetHazardRecognizer(TM, DAG);
}

ScheduleHazardRecognizer *LE1InstrInfo::
CreateTargetMIHazardRecognizer(const InstrItineraryData *II,
                               const ScheduleDAG *DAG) const {
  return new ScoreboardHazardRecognizer(II, DAG, "sched-instrs");
}

ScheduleHazardRecognizer *LE1InstrInfo::
CreateTargetPostRAHazardRecognizer(const InstrItineraryData *II,
                                   const ScheduleDAG *DAG) const {
  DEBUG(dbgs() << "LE1InstrInfo::CreateTargetPostRAHazardRecognizer\n");
  return TargetInstrInfo::CreateTargetPostRAHazardRecognizer(II, DAG);
}

bool LE1InstrInfo::
isSchedulingBoundary(const MachineInstr *MI,
                     const MachineBasicBlock *MBB,
                     const MachineFunction &MF) const {
  if(MI->isDebugValue())
    return false;
  if(MI->isLabel())
    return true;
  if(MI->isTerminator())
    return true;
  else
    return false;
}


/// getGlobalBaseReg - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
//unsigned LE1InstrInfo::getGlobalBaseReg(MachineFunction *MF) const {
  //LE1FunctionInfo *LE1FI = MF->getInfo<LE1FunctionInfo>();
  //unsigned GlobalBaseReg = LE1FI->getGlobalBaseReg();
  //if (GlobalBaseReg != 0)
    //return GlobalBaseReg;

  // Insert the set of GlobalBaseReg into the first MBB of the function
  //MachineBasicBlock &FirstMBB = MF->front();
  //MachineBasicBlock::iterator MBBI = FirstMBB.begin();
  //MachineRegisterInfo &RegInfo = MF->getRegInfo();
  //const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();

  //GlobalBaseReg = RegInfo.createVirtualRegister(LE1::CPURegsRegisterClass);
  //BuildMI(FirstMBB, MBBI, DebugLoc(), TII->get(TargetOpcode::COPY),
          //GlobalBaseReg).addReg(LE1::GP);
    //        GlobalBaseReg).addReg(LE1::ZERO);
  //RegInfo.addLiveIn(LE1::GP);
  //RegInfo.addLiveIn(LE1::ZERO);

  //LE1FI->setGlobalBaseReg(GlobalBaseReg);
  //return GlobalBaseReg;
//}
