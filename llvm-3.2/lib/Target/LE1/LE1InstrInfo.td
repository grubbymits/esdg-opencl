//===- LE1InstrInfo.td - Target Description for LE1 Target -*- tablegen -*-=//
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

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "LE1InstrFormats.td"

//===----------------------------------------------------------------------===//
// LE1 profiles and nodes
//===----------------------------------------------------------------------===//
//def SDT_LE1Ret  : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>,
  //                                     SDTCisSameAs<1, 2>,
    //                                   SDTCisInt<1>]>;
def SDT_LE1Ret  : SDTypeProfile<0, 3, [SDTCisSameAs<0, 1>,
                                       SDTCisSameAs<1, 2>,
                                       SDTCisInt<1>]>;

def SDT_LE1Exit : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def SDT_LE1Call : SDTypeProfile<0, 2, [SDTCisInt<0>,
                                       SDTCisVT<1, iPTR>]>;
def SDT_LE1LinkCall : SDTypeProfile<0, 1, [SDTCisInt<0>]>;

def SDT_LE1CMov         : SDTypeProfile<1, 4, [SDTCisSameAs<0, 1>,
                                                SDTCisSameAs<1, 2>,
                                                SDTCisSameAs<3, 4>,
                                                SDTCisInt<4>]>;
def SDT_LE1CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_LE1CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def SDT_LE1CarryUseGen : SDTypeProfile<2, 3, [//SDTCisVT<0, i32>, 
                                              //SDTCisVT<1, i1>,
                                              SDTCisSameAs<0, 2>,
                                              SDTCisSameAs<0, 3>,
                                              SDTCisInt<0>, SDTCisVT<0, i32>,
                                              SDTCisSameAs<1,4>,
                                              SDTCisInt<1>, SDTCisVT<1, i1>]>;
                                              //SDTCisOpSmallerThanOp<1,2>,
                                              //SDTCisOpSmallerThanOp<1,3>,
                                              //SDTCisOpSmallerThanOp<4,0>]>;
def SDTLE1TargetGlobal  : SDTypeProfile<1, 1, [
                                              SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>,
                                              SDTCisPtrTy<0>]>;

def SDTLE1LoadLink    : SDTypeProfile<1, 2, [SDTCisPtrTy<1>,
                                             SDTCisInt<2>,
                                             SDTCisVT<2, i32>]>;
//def SDTLE1LoadGlobal  : SDTypeProfile<1, 2, [SDTCisPtrTy<1>,
  //                                           SDTCisInt<2>,
    //                                         SDTCisVT<2, i32>]>;

def le1_targetglobal  : SDNode<"LE1ISD::TargetGlobal", SDTLE1TargetGlobal>;
def le1_globalconst  : SDNode<"LE1ISD::TargetGlobalConst", SDTLE1TargetGlobal>;

//def le1_loadglobal    : LoadSDNode<"LE1ISD::LoadGlobal", SDTLE1LoadGlobal,
  //                                [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;

def le1_loadlink      : SDNode<"LE1ISD::LoadLink", SDTLE1LoadLink,
                              [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;

// Control flow nodes
def le1_call    : SDNode<"LE1ISD::Call",SDT_LE1Call,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;
def le1_linkcall : SDNode<"LE1ISD::LinkCall", SDT_LE1LinkCall,
                          [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                           SDNPVariadic]>;
def le1_exit    : SDNode<"LE1ISD::Exit", SDT_LE1Exit, []>;
def le1_ret     : SDNode<"LE1ISD::Ret", SDT_LE1Ret,
                        [SDNPHasChain, SDNPOptInGlue]>;
def le1_retflag : SDNode<"LE1ISD::RetFlag", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue]>;
def le1_goto    : SDNode<"LE1ISD::Goto", SDTBr, [SDNPHasChain]>;
def le1_br      : SDNode<"LE1ISD::BR", SDTBrcond, [SDNPHasChain]>;
def le1_brf     : SDNode<"LE1ISD::BRF", SDTBrcond, [SDNPHasChain]>;

def le1_selectf : SDNode<"LE1ISD::SLCTF", SDTSelect, []>;

// Move nodes
def le1_mov     : SDNode<"LE1ISD::Mov", SDTIntUnaryOp, []>;
def le1_mtb     : SDNode<"LE1ISD::MTB", SDTIntTruncOp, []>;
def le1_mfb     : SDNode<"LE1ISD::MFB", SDTIntExtendOp, []>;
def le1_mtbf    : SDNode<"LE1ISD::MTBF", SDTIntTruncOp, []>;
def le1_mtl     : SDNode<"LE1ISD::MTL", SDTIntUnaryOp, []>;
def le1_mfl     : SDNode<"LE1ISD::MFL", SDTIntUnaryOp, []>;

//def le1_mtbinv  : SDNode<"LE1ISD::MTBINV", SDTIntTruncOp, []>;

// Multiplication and Division Nodes
def le1_mull    : SDNode<"LE1ISD::Mull", SDTIntBinOp, [SDNPCommutative]>;
def le1_mullu   : SDNode<"LE1ISD::Mullu", SDTIntBinOp, [SDNPCommutative]>;
def le1_mulh    : SDNode<"LE1ISD::Mulh", SDTIntBinOp, []>;
def le1_mulhu   : SDNode<"LE1ISD::Mulhu", SDTIntBinOp, []>;
def le1_mulhs   : SDNode<"LE1ISD::Mulhs", SDTIntBinOp, []>;
def le1_mulll   : SDNode<"LE1ISD::Mulll", SDTIntBinOp, []>;
def le1_mulllu  : SDNode<"LE1ISD::Mulllu", SDTIntBinOp, []>;
def le1_mullh   : SDNode<"LE1ISD::Mullh", SDTIntBinOp, []>;
def le1_mullhu  : SDNode<"LE1ISD::Mullhu", SDTIntBinOp, []>;
def le1_mulhh   : SDNode<"LE1ISD::Mulhh", SDTIntBinOp, [SDNPCommutative]>;
def le1_mulhhu  : SDNode<"LE1ISD::Mulhhu", SDTIntBinOp, [SDNPCommutative]>;

def le1_addcg   : SDNode<"LE1ISD::Addcg", SDT_LE1CarryUseGen, []>;
def le1_divs    : SDNode<"LE1ISD::Divs", SDT_LE1CarryUseGen, []>;

// Nodes for OpenCL calculations
def le1_mulch   : SDNode<"LE1ISD::MUL_CHAIN", SDTIntBinOp, [SDNPHasChain]>;
def le1_addch   : SDNode<"LE1ISD::ADD_CHAIN", SDTIntBinOp, [SDNPHasChain]>;

// Shift Adds
def le1_sh1add  : SDNode<"LE1ISD::Sh1Add", SDTIntShiftOp, []>;
def le1_sh2add  : SDNode<"LE1ISD::Sh2Add", SDTIntShiftOp, []>;
def le1_sh3add  : SDNode<"LE1ISD::Sh3Add", SDTIntShiftOp, []>;
def le1_sh4add  : SDNode<"LE1ISD::Sh4Add", SDTIntShiftOp, []>;

// Extend Nodes
def le1_sxtb    : SDNode<"LE1ISD::SXTB", SDTExtInreg, []>;
def le1_sxth    : SDNode<"LE1ISD::SXTH", SDTExtInreg, []>;
def le1_zxtb    : SDNode<"LE1ISD::ZXTB", SDTExtInreg, []>;
def le1_zxth    : SDNode<"LE1ISD::ZXTH", SDTExtInreg, []>;

// Bit Operators
def le1_tbit    : SDNode<"LE1ISD::Tbit", SDTIntBinOp, []>;
def le1_tbitf   : SDNode<"LE1ISD::Tbitf", SDTIntBinOp, []>;
def le1_sbit    : SDNode<"LE1ISD::Sbit", SDTIntBinOp, []>;
def le1_sbitf   : SDNode<"LE1ISD::Sbitf", SDTIntBinOp, []>;

// Max and Min Operators
def le1_max     : SDNode<"LE1ISD::Max", SDTIntBinOp, [SDNPCommutative]>;
def le1_min     : SDNode<"LE1ISD::Min", SDTIntBinOp, [SDNPCommutative]>;

// Boolean Operators
def le1_nandl   : SDNode<"LE1ISD::Nandl", SDTIntBinOp, [SDNPCommutative]>;
def le1_norl    : SDNode<"LE1ISD::Norl", SDTIntBinOp, [SDNPCommutative]>;
def le1_orl     : SDNode<"LE1ISD::Orl", SDTIntBinOp, [SDNPCommutative]>;
def le1_andc    : SDNode<"LE1ISD::Andc", SDTIntBinOp, []>;
def le1_andl    : SDNode<"LE1ISD::Andl", SDTIntBinOp, [SDNPCommutative]>;
def le1_orc     : SDNode<"LE1ISD::Orc", SDTIntBinOp, []>;

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_LE1CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_LE1CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

//===----------------------------------------------------------------------===//
// LE1 Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// LE1 Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

// Instruction operand types
def brtarget    : Operand<OtherVT>;
def calltarget  : Operand<i32>;
def simm16      : Operand<i32>;
//def simm16_64   : Operand<i64>;
def shamt       : Operand<i32>;
def globaladdress : Operand<i32>;

//def i32imm      : Operand<i32>;
def uimm32      : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

// Unsigned Operand
def uimm16      : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

def globaladdroffset : Operand<i32> {
  let PrintMethod = "printGlobalOffset";
}

// Address operand
def mem : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops CPURegs, simm16);
  let EncoderMethod = "getMemEncoding";
}

def mem_ea : Operand<i32> {
  let PrintMethod = "printMemOperandEA";
  let MIOperandInfo = (ops CPURegs, simm16);
  let EncoderMethod = "getMemEncoding";
}

// size operand of ext instruction
def size_ext : Operand<i32> {
  let EncoderMethod = "getSizeExtEncoding";
}

// size operand of ins instruction
def size_ins : Operand<i32> {
  let EncoderMethod = "getSizeInsEncoding";
}

// Transformation Function - get the lower 16 bits.
def LO16 : SDNodeXForm<imm, [{
  return getI32Imm((unsigned)N->getZExtValue() & 0xFFFF);
}]>;

// Transformation Function - get the higher 16 bits.
def HI16 : SDNodeXForm<imm, [{
  return getI32Imm((unsigned)N->getZExtValue() >> 16);
}]>;

// Node immediate fits as 16-bit sign extended on target immediate.
// e.g. addi, andi
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

def immSExt32 : PatLeaf<(imm), [{ return isInt<32>(N->getSExtValue()); }]>;

def immZExt32 : PatLeaf<(imm), 
    [{ return (uint32_t)N->getZExtValue() == (unsigned int)N->getZExtValue();}]>;

def immSExt1  : ImmLeaf<i1, [{ return (int32_t)Imm->getSExtValue(); }]>;

def u16ImmPred : PatLeaf<(i32 imm), [{ return isUInt<16>(N->getSExtValue()); }]>;

// Node immediate fits as 16-bit zero extended on target immediate.
// The LO16 param means that only the lower 16 bits of the node
// immediate are caught.
// e.g. addiu, sltiu
def immZExt16  : PatLeaf<(imm), [{
  if (N->getValueType(0) == MVT::i32)
    return (uint32_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
  else
    return (uint64_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
}], LO16>;

// shamt field must fit in 5 bits.
def immZExt5 : PatLeaf<(imm), [{
  return N->getZExtValue() == ((N->getZExtValue()) & 0x1f) ;
}]>;

// LE1 Address Mode! SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], []>;
def gAddr : ComplexPattern<i32, 2, "SelectGAddr", [frameindex], []>;

//===----------------------------------------------------------------------===//
// Pattern fragment for load/store
//===----------------------------------------------------------------------===//
class UnalignedLoad<PatFrag Node> : PatFrag<(ops node:$ptr), (Node node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  return LD->getMemoryVT().getSizeInBits()/8 > LD->getAlignment();
}]>;

class AlignedLoad<PatFrag Node> : PatFrag<(ops node:$ptr), (Node node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  return LD->getMemoryVT().getSizeInBits()/8 <= LD->getAlignment();
}]>;

class UnalignedStore<PatFrag Node> : PatFrag<(ops node:$val, node:$ptr),
                                             (Node node:$val, node:$ptr), [{
  StoreSDNode *SD = cast<StoreSDNode>(N);
  return SD->getMemoryVT().getSizeInBits()/8 > SD->getAlignment();
}]>;

class AlignedStore<PatFrag Node> : PatFrag<(ops node:$val, node:$ptr),
                                           (Node node:$val, node:$ptr), [{
  StoreSDNode *SD = cast<StoreSDNode>(N);
  return SD->getMemoryVT().getSizeInBits()/8 <= SD->getAlignment();
}]>;

// Load/Store PatFrags.
def sextloadi16_a   : AlignedLoad<sextloadi16>;
def zextloadi16_a   : AlignedLoad<zextloadi16>;
def extloadi16_a    : AlignedLoad<extloadi16>;
def load_a          : AlignedLoad<load>;
def sextloadi32_a   : AlignedLoad<sextloadi32>;
def zextloadi32_a   : AlignedLoad<zextloadi32>;
def extloadi32_a    : AlignedLoad<extloadi32>;
def truncstorei16_a : AlignedStore<truncstorei16>;
def store_a         : AlignedStore<store>;
def truncstorei32_a : AlignedStore<truncstorei32>;
def sextloadi16_u   : UnalignedLoad<sextloadi16>;
def zextloadi16_u   : UnalignedLoad<zextloadi16>;
def extloadi16_u    : UnalignedLoad<extloadi16>;
def load_u          : UnalignedLoad<load>;
def sextloadi32_u   : UnalignedLoad<sextloadi32>;
def zextloadi32_u   : UnalignedLoad<zextloadi32>;
def extloadi32_u    : UnalignedLoad<extloadi32>;
def truncstorei16_u : UnalignedStore<truncstorei16>;
def store_u         : UnalignedStore<store>;
def truncstorei32_u : UnalignedStore<truncstorei32>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction Classes that utilise the Integer ALUs 
//===----------------------------------------------------------------------===//

// Arithmetic and boolean logic operators
class ArithLogicR<string instr_asm, SDNode OpNode, bit isComm = 0>:
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, CPURegs:$src2),
      !strconcat(instr_asm, "  $dst, $src1, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src1, CPURegs:$src2))], IIAlu> {
      let isCommutable = isComm;
}

let isReMaterializable = 1, neverHasSideEffects = 1 in
class ArithLogicI<string instr_asm, SDNode OpNode> :
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, i32imm:$src2),
      !strconcat(instr_asm, "  $dst, $src1, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src1, imm:$src2))], IIAlu>;

let isAsCheapAsAMove = 1, isReMaterializable = 1, neverHasSideEffects = 1 in {
class MoveR<string instr_asm, SDNode OpNode, RegisterClass OutReg,
           RegisterClass InReg>:
      LE1Inst<(outs OutReg:$dst),(ins InReg:$src),
      !strconcat(instr_asm, "  $dst, $src"),
      [(set OutReg:$dst, (OpNode InReg:$src))], IIAlu>;

let isReMaterializable = 1, isMoveImm =1, neverHasSideEffects = 1 in
class MoveI<string instr_asm, SDNode OpNode, RegisterClass OutReg, Operand Opr,
            SDNode I> :
      LE1Inst<(outs OutReg:$dst), (ins Opr:$src),
      !strconcat(instr_asm, "  $dst, $src"),
      [(set OutReg:$dst, (OpNode I:$src))], IIAlu>;
}

// Shifts
let isReMaterializable = 1 in
class ShiftI<string instr_asm, SDNode OpNode>:
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src, uimm16:$src2),
      !strconcat(instr_asm, "  $dst, $src, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src, (i32 imm:$src2)))],
      IIAlu>;

class ShiftR<string instr_asm, SDNode OpNode>:
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, CPURegs:$src2),
      !strconcat(instr_asm, "  $dst, $src1, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src1, CPURegs:$src2))], IIAlu>;

// Comparisons
class SetCCR<string instr_asm, PatFrag cond_op, RegisterClass DestRegs>:
      LE1Inst<(outs DestRegs:$res), (ins CPURegs:$lhs, CPURegs:$rhs),
      !strconcat(instr_asm, "  $res, $lhs, $rhs"),
      [(set DestRegs:$res, (cond_op CPURegs:$lhs, CPURegs:$rhs))], IIAlu>;

let isReMaterializable = 1, neverHasSideEffects = 1 in
class SetCCI<string instr_asm, PatFrag cond_op, RegisterClass DestRegs>:
      LE1Inst<(outs DestRegs:$res), (ins CPURegs:$lhs, i32imm:$rhs),
      !strconcat(instr_asm, "  $res, $lhs, $rhs"),
      [(set DestRegs:$res, (cond_op CPURegs:$lhs, imm:$rhs))], IIAlu>;

// Selects
class SelectRR<string instr_asm, SDNode OpNode>:
      LE1Inst<(outs CPURegs:$res), (ins BRegs:$cond, CPURegs:$T, CPURegs:$F),
      !strconcat(instr_asm, "  $res, $cond, $T, $F"),
      [(set CPURegs:$res, (OpNode BRegs:$cond, CPURegs:$T, CPURegs:$F))],
      IIAlu>;

class SelectRI<string instr_asm, SDNode OpNode>:
      LE1Inst<(outs CPURegs:$res), (ins BRegs:$cond, CPURegs:$T, i32imm:$F),
      !strconcat(instr_asm, "  $res, $cond, $T, $F"),
      [(set CPURegs:$res, (OpNode BRegs:$cond, CPURegs:$T, imm:$F))],
      IIAlu>;

// Maximum and Minimum
class MaxMinR<string instr_asm, SDNode OpNode> :
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, CPURegs:$src2),
      !strconcat(instr_asm, "  $dst, $src1, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src1, CPURegs:$src2))], IIAlu>;

let isReMaterializable = 1, neverHasSideEffects = 1 in
class MaxMinI<string instr_asm, SDNode OpNode> :
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, i32imm:$src2),
      !strconcat(instr_asm, "  $dst, $src1, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src1, imm:$src2))], IIAlu>;

// Extend
let isReMaterializable = 1, neverHasSideEffects = 1 in
class Extend<string instr_asm, SDNode OpNode, ValueType vt>:
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src),
      !strconcat(instr_asm, "  $dst, $src"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src, vt))], IIAlu>;

//===----------------------------------------------------------------------===//
// Instruction Classes that utilise the multiplier functional units
//===----------------------------------------------------------------------===//

class MultR<string instr_asm, SDNode OpNode> :
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, CPURegs:$src2),
     !strconcat(instr_asm, "  $dst, $src1, $src2"),
     [(set CPURegs:$dst, (OpNode CPURegs:$src1, CPURegs:$src2))], IIMul>;

class MultI<string instr_asm, SDNode OpNode, Operand Op, PatLeaf PL> :
      LE1Inst<(outs CPURegs:$dst), (ins CPURegs:$src1, i32imm:$src2),
      !strconcat(instr_asm, "  $dst, $src1, $src2"),
      [(set CPURegs:$dst, (OpNode CPURegs:$src1, PL:$src2))], IIMul>;

//===----------------------------------------------------------------------===//
// Instruction Classes that utilise the Load/Store units
//===----------------------------------------------------------------------===//
/*
class FMem<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>: FFI<op, outs, ins, asmstr, pattern, itin> {
  bits<21> addr;
  let Inst{25-21} = addr{20-16};
  let Inst{15-0}  = addr{15-0};
}*/

let Uses = [ZERO, SP] in {
let canFoldAsLoad = 1 in
class LoadM<string instr_asm, PatFrag OpNode>:
      LE1Inst<(outs CPURegs:$dst), (ins mem:$addr),
      !strconcat(instr_asm, "  $dst, $addr"),
      [(set CPURegs:$dst, (OpNode addr:$addr))], IILoadStore>;

class StoreM<string instr_asm, PatFrag OpNode>:
      LE1Inst<(outs), (ins CPURegs:$src, mem:$addr),
      !strconcat(instr_asm, "  $addr, $src"),
      [(OpNode CPURegs:$src, addr:$addr)], IILoadStore>;

let neverHasSideEffects = 1, canFoldAsLoad = 1, isReMaterializable=1 in
class LoadG<string instr_asm, PatFrag OpNode>:
      LE1Inst<(outs CPURegs:$dst),
      (ins globaladdress:$addr, globaladdroffset:$offset),
      !strconcat(instr_asm, " $dst, r0.0[($addr+$offset)]"),
      [/*(set CPURegs:$dst, (OpNode (add tglobaladdr:$addr, imm:$offset)))*/],
      IILoadStore>;

class StoreG<string instr_asm, PatFrag OpNode>:
      LE1Inst<(outs),
      (ins CPURegs:$src, globaladdress:$addr, globaladdroffset:$offset),
      !strconcat(instr_asm, "  r0.0[($addr+$offset)], $src"),
      [/*(OpNode CPURegs:$src, (add tglobaladdr:$addr, imm:$offset))*/],
      IILoadStore>;

class EffectiveAddress<string instr_asm> :
      LE1Inst<(outs CPURegs:$rt), (ins mem_ea:$addr),
      instr_asm, [(set CPURegs:$rt, addr:$addr)], IIAlu>;
}

// -----------------------------------------------------------------------//
// Instruction Class that use the Branch unit
// -----------------------------------------------------------------------//

// Conditional Branch
let isBranch = 1, isTerminator = 1, isBarrier = 1 in
class CBranch<string instr_asm, SDNode OpNode>:
      LE1Inst<(outs),
      (ins BRegs:$cond, brtarget:$dst),
      !strconcat(instr_asm, "  $cond, $dst"),
      [(OpNode BRegs:$cond, bb:$dst)], IIBranch>;

//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
/// Arithmetic Instructions 
def ADDi   : ArithLogicI<"add.0", add>;
def SUBi   : ArithLogicI<"sub.0", sub>;
def ADD    : ArithLogicR<"add.0", add, 1>;
def ADDCH  : ArithLogicR<"add.0", le1_addch, 1>;
def SUB    : ArithLogicR<"sub.0", sub>;

//===----------------------------------------------------------------------===//
/// Boolean Instructions
def AND     : ArithLogicR<"and.0", and, 1>;
def OR      : ArithLogicR<"or.0",  or, 1>;
def XOR     : ArithLogicR<"xor.0", xor, 1>;
def ANDi    : ArithLogicI<"and.0", and>;
def ORi     : ArithLogicI<"or.0", or>;
def XORi    : ArithLogicI<"xor.0", xor>;

def NANDL : ArithLogicR<"nandl.0", le1_nandl>;
def NORL  : ArithLogicR<"norl.0", le1_norl>;
def ORL   : ArithLogicR<"orl.0", le1_orl>;
def ANDC  : ArithLogicR<"andc.0", le1_andc>;
def ANDL  : ArithLogicR<"andl.0", le1_andl>;
def ORC   : ArithLogicR<"orc.0", le1_orc>;
def ANDCi : ArithLogicI<"andc.0", le1_andc>;
def ORCi  : ArithLogicI<"orc.0", le1_orc>;

///===---------------------------------------------------------------------===//
// Shift Instructions
def SHLi      : ShiftI<"shl.0", shl>;
def SHRi      : ShiftI<"shr.0", sra>;
def SHL       : ShiftR<"shl.0", shl>;
def SHR       : ShiftR<"shr.0", sra>;
def SHRU      : ShiftR<"shru.0", srl>;
def SHRUi     : ShiftI<"shru.0", srl>;

def SH1ADD   : ShiftR<"sh1add.0", le1_sh1add>;
def SH2ADD   : ShiftR<"sh2add.0", le1_sh2add>;
def SH3ADD   : ShiftR<"sh3add.0", le1_sh3add>;
def SH4ADD   : ShiftR<"sh4add.0", le1_sh4add>;
def SH1ADDi  : ShiftI<"sh1add.0", le1_sh1add>;
def SH2ADDi  : ShiftI<"sh2add.0", le1_sh2add>;
def SH3ADDi  : ShiftI<"sh3add.0", le1_sh3add>;
def SH4ADDi  : ShiftI<"sh4add.0", le1_sh4add>;

///===---------------------------------------------------------------------===//
// Compare Instructions
let isCompare = 1 in {
def CMPEQr  : SetCCR<"cmpeq.0", seteq, CPURegs>;
def CMPGEr  : SetCCR<"cmpge.0", setge, CPURegs>;
def CMPGEUr : SetCCR<"cmpgeu.0", setuge, CPURegs>;
def CMPGTr  : SetCCR<"cmpgt.0", setgt, CPURegs>;
def CMPGTUr : SetCCR<"cmpgtu.0", setugt, CPURegs>;
def CMPLEr  : SetCCR<"cmple.0", setle, CPURegs>;
def CMPLEUr : SetCCR<"cmpleu.0", setule, CPURegs>;
def CMPLTr  : SetCCR<"cmplt.0", setlt, CPURegs>;
def CMPLTUr : SetCCR<"cmpltu.0", setult, CPURegs>;
def CMPNEr  : SetCCR<"cmpne.0", setne, CPURegs>;

def CMPEQri  : SetCCI<"cmpeq.0", seteq, CPURegs>;
def CMPGEri  : SetCCI<"cmpge.0", setge, CPURegs>;
def CMPGEUri : SetCCI<"cmpgeu.0", setuge, CPURegs>;
def CMPGTri  : SetCCI<"cmpgt.0", setgt, CPURegs>;
def CMPGTUri : SetCCI<"cmpgtu.0", setugt, CPURegs>;
def CMPLEri  : SetCCI<"cmple.0", setle, CPURegs>;
def CMPLEUri : SetCCI<"cmpleu.0", setule, CPURegs>;
def CMPLTri  : SetCCI<"cmplt.0", setlt, CPURegs>;
def CMPLTUri : SetCCI<"cmpltu.0", setult, CPURegs>;
def CMPNEri  : SetCCI<"cmpne.0", setne, CPURegs>;

def CMPEQ  : SetCCR<"cmpeq.0", seteq, BRegs>;
def CMPGE  : SetCCR<"cmpge.0", setge, BRegs>;
def CMPGEU : SetCCR<"cmpgeu.0", setuge, BRegs>;
def CMPGT  : SetCCR<"cmpgt.0", setgt, BRegs>;
def CMPGTU : SetCCR<"cmpgtu.0", setugt, BRegs>;
def CMPLE  : SetCCR<"cmple.0", setle, BRegs>;
def CMPLEU : SetCCR<"cmpleu.0", setule, BRegs>;
def CMPLT  : SetCCR<"cmplt.0", setlt, BRegs>;
def CMPLTU : SetCCR<"cmpltu.0", setult, BRegs>;
def CMPNE  : SetCCR<"cmpne.0", setne, BRegs>;

def CMPEQi  : SetCCI<"cmpeq.0", seteq, BRegs>;
def CMPGEi  : SetCCI<"cmpge.0", setge, BRegs>;
def CMPGEUi : SetCCI<"cmpgeu.0", setuge, BRegs>;
def CMPGTi  : SetCCI<"cmpgt.0", setgt, BRegs>;
def CMPGTUi : SetCCI<"cmpgtu.0", setugt, BRegs>;
def CMPLEi  : SetCCI<"cmple.0", setle, BRegs>;
def CMPLEUi : SetCCI<"cmpleu.0", setule, BRegs>;
def CMPLTi  : SetCCI<"cmplt.0", setlt, BRegs>;
def CMPLTUi : SetCCI<"cmpltu.0", setult, BRegs>;
def CMPNEi  : SetCCI<"cmpne.0", setne, BRegs>;

}

///===---------------------------------------------------------------------===//
/// Bit Operators
def TBIT  : ArithLogicR<"tbit.0", le1_tbit>;
def TBITF : ArithLogicR<"tbitf.0", le1_tbitf>;
def SBIT  : ArithLogicR<"sbit.0", le1_sbit>;
def SBITF : ArithLogicR<"sbitf.0", le1_sbitf>;


///===---------------------------------------------------------------------===//
/// Max and Min Instructions
def MAX   : MaxMinR<"max.0", le1_max>;
def MIN   : MaxMinR<"min.0", le1_min>;
def MAXU  : MaxMinR<"maxu.0", le1_max>;
def MINU  : MaxMinR<"minu.0", le1_min>;
def MAXi  : MaxMinI<"max.0", le1_max>;
def MINi   : MaxMinI<"min.0", le1_min>;
def MAXUi  : MaxMinI<"maxu.0", le1_max>;
def MINUi  : MaxMinI<"minu.0", le1_min>;


///===---------------------------------------------------------------------===//
/// Select Instructions
def SLCTrr  : SelectRR<"slct.0", select>;
def SLCTFrr : SelectRR<"slctf.0", le1_selectf>;
def SLCTri  : SelectRI<"slct.0", select>;
def SLCTFri : SelectRI<"slctf.0", le1_selectf>;

///===---------------------------------------------------------------------===//
/// Extending Instructions
def SXTB  : Extend<"sxtb.0", le1_sxtb, i8>;
def SXTH  : Extend<"sxth.0", le1_sxth, i16>;
def ZXTB  : Extend<"zxtb.0", le1_zxtb, i8>;
def ZXTH  : Extend<"zxth.0", le1_zxth, i16>;

///===---------------------------------------------------------------------===//
// Move Instructions
def MFB   : MoveR<"mfb.0",  le1_mfb,  CPURegs,  BRegs>;
def MTB   : MoveR<"mtb.0",  le1_mtb,  BRegs,    CPURegs>;
def MTBF  : MoveR<"mtbf.0", le1_mtbf, BRegs,    CPURegs>;
def MOVr  : MoveR<"mov.0",  le1_mov,  CPURegs,  CPURegs>;
def MOVi  : MoveI<"mov.0", le1_mov, CPURegs, i32imm, imm>;
//def MOVg  : MoveI<"mov.0",  le1_mov, CPURegs, globaladdress, tglobaladdr>;
def MFL   : MoveR<"mfl.0",  le1_mfl,  CPURegs,  LReg>;
def MTL   : MoveR<"mtl.0",  le1_mtl,  LReg,     CPURegs>;
def MTLg  : MoveI<"mov.0", le1_mtl, LReg, globaladdress, tglobaladdr>;

///===---------------------------------------------------------------------===//
/// Instructions used for software division
//let usesCustomInserter in {
def ADDCG : LE1Inst<(outs CPURegs:$res, BRegs:$cout),
            (ins CPURegs:$src1, CPURegs:$src2, BRegs:$cin),
            "addcg.0  $res, $cout, $src1, $src2, $cin",
            [/*(set CPURegs:$res, BRegs:$cout, (le1_addcg CPURegs:$src1,
                                              CPURegs:$src2, BRegs:$cin))*/],
            IIAlu>;

def DIVS :  LE1Inst<(outs CPURegs:$res, BRegs:$cout),
            (ins CPURegs:$src1, CPURegs:$src2, BRegs:$cin),
            "divs.0  $res, $cout, $src1, $src2, $cin",
            [/*(set CPURegs:$res, BRegs:$cout, (le1_divs CPURegs:$src1,
                                              CPURegs:$src2, BRegs:$cin))*/],
            IIAlu>;

///===---------------------------------------------------------------------===//
// Multiply Instructions


def MULL    : MultR<"mull.0", le1_mull>;     // s1 * i16(s2)
def MULLU   : MultR<"mullu.0", le1_mullu>;    // s1 * ui16(s2)
def MULH    : MultR<"mulh.0", le1_mulh>;     // s1 * i16(s2 >> 16)
def MULHU   : MultR<"mulhu.0", le1_mulhu>;    // s1 * ui16(s2 >> 16)
def MULHS   : MultR<"mulhs.0", le1_mulhs>;    // s1 * ui16(s2 >> 16) << 16
def MULLL   : MultR<"mulll.0", le1_mulll>;    // i16(s1) * i16(s2)
def MULLLU  : MultR<"mulllu.0", le1_mulllu>;   // ui16(s1) * ui16(s2)
def MULLH   : MultR<"mullh.0", le1_mullh>;    // i16(s1) * i16(s2 >> 16)
def MULLHU  : MultR<"mullhu.0", le1_mullhu>;   // ui16(s1) * ui16(s2 >> 16)
def MULHH   : MultR<"mulhh.0", le1_mulhh>;    // i16(s1 >> 16) * i16(s2 >> 16)
def MULHHU  : MultR<"mulhhu.0", le1_mulhhu>;   // ui16(s1 >> 16) * ui16(s2 >> 16)

def MULLi    : MultI<"mull.0", le1_mull, simm16, immSExt16>;
def MULLUi   : MultI<"mullu.0", le1_mullu, uimm16, immZExt16>;
def MULHi    : MultI<"mulh.0", le1_mulh, simm16, immSExt16>;
def MULHUi   : MultI<"mulhu.0", le1_mulhu, uimm16, immZExt16>;
def MULHSi   : MultI<"mulhs.0", le1_mulhs, simm16, immSExt16>;
def MULLLi   : MultI<"mulll.0", le1_mulll, simm16, immSExt16>;
def MULLLUi  : MultI<"mulllu.0", le1_mulllu, uimm16, immZExt16>;
def MULLHi   : MultI<"mullh.0", le1_mullh, simm16, immSExt16>;
def MULLHUi  : MultI<"mullhu.0", le1_mullhu, uimm16, immZExt16>;
def MULHHi   : MultI<"mulhh.0", le1_mulhh, simm16, immSExt16>;
def MULHHUi  : MultI<"mulhhu.0", le1_mulhhu, uimm16, immZExt16>;

///===---------------------------------------------------------------------===//
/// Load and Store Instructions
def LDB      : LoadM<"ldb.0",  sextloadi8>;
def LDBu     : LoadM<"ldbu.0", zextloadi8>;
def LDH      : LoadM<"ldh.0",  sextloadi16>;
def LDHu     : LoadM<"ldhu.0", zextloadi16>;
def LDW      : LoadM<"ldw.0",  load>;
def STB      : StoreM<"stb.0", truncstorei8>;
def STH      : StoreM<"sth.0", truncstorei16>;
def STW      : StoreM<"stw.0", store>;

//let neverHasSideEffects = 1 in {
def LDB_G      : LoadG<"ldb.0",  sextloadi8>;
def LDBu_G     : LoadG<"ldbu.0", zextloadi8>;
def LDH_G      : LoadG<"ldh.0",  sextloadi16>;
def LDHu_G     : LoadG<"ldhu.0", zextloadi16>;
def LDW_G      : LoadG<"ldw.0",  load>;
def STB_G      : StoreG<"stb.0", truncstorei8>;
def STH_G      : StoreG<"sth.0", truncstorei16>;
def STW_G      : StoreG<"stw.0", store>;
//}

//let isReMaterializable = 1, isMoveImm = 1 in {
let AddedComplexity = 100 in {
def MOVg : LE1Inst<(outs CPURegs:$dst), (ins globaladdress:$addr),
                    "mov.0 $dst, $addr",
                    [(set CPURegs:$dst, (le1_targetglobal tglobaladdr:$addr))],
                    IIAlu>;
def MOVg2 : LE1Inst<(outs CPURegs:$dst), (ins globaladdress:$addr),
                    "mov.0 $dst, $addr",
                    [(set CPURegs:$dst, (le1_globalconst tglobaladdr:$addr))],
                    IIAlu>;
}

//let Defs = [L0] in
def LDL : LE1Inst<(outs LReg:$dst),
          (ins globaladdress:$addr, globaladdroffset:$offset),
          "ldw.0 L0.0, r0.0[($addr+$offset)]",
          [/*(set LReg:$dst, (add (le1_loadlink tglobaladdr:$addr), imm:$offset))*/],
          IILoadStore>;
//let Uses = [L0] in
def STL : LE1Inst<(outs),
          (ins LReg:$src, globaladdress:$addr, globaladdroffset:$offset),
          "stw.0 r0.0[($addr+$offset)], L0.0",
          [(store LReg:$src, (add (i32 tglobaladdr:$addr), imm:$offset))],
          IILoadStore>;

//===----------------------------------------------------------------------===//
// Control-Flow Instructions
def BR     : CBranch<"br.0", le1_br>;
def BRF    : CBranch<"brf.0", le1_brf>;

// Unconditional branch
let isBranch=1, isTerminator=1, isBarrier=1 in
def GOTO :  LE1Inst<(outs), (ins brtarget:$target),
            "goto.0  $target",
            [(le1_goto bb:$target)], IIBranch>;

// Call
let isCall=1,
  // All calls clobber the non-callee saved registers...
  Defs = [B0, B1, B2, B3, B4, B5, B6, B7,
  STRP, AR0, AR1, AR2, AR3, AR4, AR5, AR6, AR7, T0, T1, T2, T3, T4, T5,
  T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21,
  T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33, T34, T35, T36,
  T37, T38, T39, T40, T41], Uses = [L0] in {

  def CALL :  LE1Inst<(outs),
              (ins LReg:$lnk, calltarget:$target, variable_ops),
              "call.0  $lnk, $target",
              [(le1_call LReg:$lnk, imm:$target)], IIBranch>;

  def LNKCALL : LE1Inst<(outs), (ins LReg:$lnk, variable_ops),
  "call.0  $lnk, $lnk",
  [/*(le1_linkcall LReg:$lnk)*/],
  IIBranch>;
}

//let Constraints = "$src = $dst",
let Uses = [L0, SP],// in
Defs = [SP],
isReturn=1, isTerminator=1, isBarrier=1, hasCtrlDep=1 in
//def Ret : LE1Inst<(outs CPURegs:$dst),
def Ret : LE1Inst<(outs),
          (ins CPURegs:$src, i32imm:$frame, LReg:$lnk),
          "return.0  $src, $src, $frame, $lnk",
          [(le1_ret CPURegs:$src, imm:$frame, LReg:$lnk)],
          IIBranch>;


//def : Pat<(le1_call L0, (load (i32 tglobaladdr:$addr))),
  //        (CALLPOINTER L0, tglobaladdr:$addr)>;

//def : Pat<(le1_call L0, (le1_targetglobal tglobaladdr:$addr)),
  //        (CALLPOINTER L0, tglobaladdr:$addr)>;

//def : Pat<(le1_call L0, (load (le1_targetglobal tglobaladdr:$addr))),
  //        (CALLPOINTER L0, tglobaladdr:$addr)>;




// FrameIndexes are legalized when they are operands from load/store
// instructions. The same not happens for stack address copies, so an
// add op with mem ComplexPattern is used and the stack address copy
// can be matched. It's similar to Sparc LEA_ADDRi
//def LEA_ADDiu : EffectiveAddress<"addiu  $rt, $addr">;
def EA_ADD  : EffectiveAddress<"add.0  $rt, $addr">;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

// As stack alignment is always done with addiu, we need a 16-bit immediate
let Defs = [SP], Uses = [SP], isPseudo=1 in {
def ADJCALLSTACKDOWN : LE1Pseudo<(outs), (ins uimm16:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : LE1Pseudo<(outs), (ins uimm16:$amt1, uimm16:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// Operations to load/store predicate register
let Defs = [T45] in {
def LDW_PRED  : LE1Pseudo<(outs BRegs:$dst), (ins mem:$addr),
                          "!LOADPRED $dst", []>;
def STW_PRED  : LE1Pseudo<(outs), (ins BRegs:$src, mem:$addr),
                          "!STOREPRED $src", []>;
}

// Operation to handle function pointer
let Defs = [L0], Uses = [L0], isPseudo=1 in {
  def CALLPOINTER : LE1Pseudo<(outs), (ins LReg:$lnk, globaladdress:$src),
                              "!LINKCALL", []>;
}

def CLK : LE1Pseudo < (outs), (ins), "NOP", []>;

// Instruction created to exit main
let isReturn=1, isTerminator=1, isBarrier=1 in
def Exit : LE1Inst< (outs), (ins LReg:$lnk),
          "call.0  $lnk, exit", [], IIBranch>;

let isReturn=1, isTerminator=1, isBarrier=1, hasCtrlDep=1 in
def RetFlag : LE1Pseudo < (outs), (ins), ";", [(le1_retflag)]>;

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// Carry patterns
def : Pat<(subc CPURegs:$lhs, CPURegs:$rhs),
          (SUB CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(addc CPURegs:$lhs, CPURegs:$rhs),
          (ADD CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(addc  CPURegs:$src, imm:$imm),
          (ADDi CPURegs:$src, imm:$imm)>;

// Call
def : Pat<(le1_call LReg:$lnk, (i32 tglobaladdr:$dst)),
          (CALL LReg:$lnk, tglobaladdr:$dst)>;
def : Pat<(le1_call LReg:$lnk, (i32 texternalsym:$dst)),
          (CALL LReg:$lnk, texternalsym:$dst)>;

// Calls using function pointers
def : Pat<(le1_call LReg:$lnk, (i32 (load addr:$src))),
          (CALLPOINTER LReg:$lnk, addr:$src)>;

// Shift Add Patterns
def : Pat<(add (shl CPURegs:$lhs, (i32 1)), CPURegs:$rhs),
          (SH1ADD CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(add (shl CPURegs:$lhs, (i32 2 )), CPURegs:$rhs),
          (SH2ADD CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(add (shl CPURegs:$lhs, (i32 3)), CPURegs:$rhs),
          (SH3ADD CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(add (shl CPURegs:$lhs, (i32 4)), CPURegs:$rhs),
          (SH4ADD CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(add (shl CPURegs:$lhs, (i32 1)), (i32 imm:$rhs)),
          (SH1ADDi CPURegs:$lhs, imm:$rhs)>;
def : Pat <(add (shl CPURegs:$lhs, (i32 2)), (i32 imm:$rhs)),
          (SH2ADDi CPURegs:$lhs, imm:$rhs)>;
def : Pat<(add (shl CPURegs:$lhs, (i32 3)), (i32 imm:$rhs)),
          (SH3ADDi CPURegs:$lhs, imm:$rhs)>;
def : Pat<(add (shl CPURegs:$lhs, (i32 4)), (i32 imm:$rhs)),
          (SH4ADDi CPURegs:$lhs, imm:$rhs)>;

// Extending Patterns
def : Pat<(sra (shl CPURegs:$src, (i32 16)), (i32 16)),
          (SXTH CPURegs:$src)>;
def : Pat<(sra (shl CPURegs:$src, (i32 24)), (i32 24)),
          (SXTB CPURegs:$src)>;
def : Pat<(srl (shl CPURegs:$src, (i32 16)), (i32 16)),
          (ZXTH CPURegs:$src)>;
def : Pat<(srl (shl CPURegs:$src, (i32 24)), (i32 24)),
          (ZXTB CPURegs:$src)>;
def : Pat<(and CPURegs:$src, (i32 0x000000FF)),
          (ZXTB CPURegs:$src)>;
def : Pat<(and CPURegs:$src, (i32 0x0000FFFF)),
          (ZXTH CPURegs:$src)>;

// Extend in reg patterns
def : Pat<(sext_inreg CPURegs:$src, i8),
          (SXTB CPURegs:$src)>;
def : Pat<(sext_inreg CPURegs:$src, i16),
          (SXTH CPURegs:$src)>;

// Patterns to deal with extending branch registers
def : Pat<(sext BRegs:$src),
          (SLCTFri BRegs:$src, ZERO, (i32 0xFFFFFFFF))>;

// Bit Operator Patterns
// TBIT  = ((s1) & ((unsigned int) 1 << (s2))) ? 1 : 0
def : Pat<(selectcc (and CPURegs:$lhs, (shl (i32 1), CPURegs:$rhs)),
                     (i32 0), (i32 1), (i32 0), SETNE),
          (TBIT CPURegs:$lhs, CPURegs:$rhs)>;
// TBITF = ((s1) & ((unsigned int) 1 << (s2))) ? 0 : 1
def : Pat<(zext (setcc (and CPURegs:$lhs, (shl (i32 1), CPURegs:$rhs)),
                       (i32 0), SETEQ)),
          (TBITF CPURegs:$lhs, CPURegs:$rhs)>;
// SBIT = (s1) | ((unsigned int) 1 << (s2))
def : Pat<(or CPURegs:$lhs, (shl (i32 1), CPURegs:$rhs)),
          (SBIT CPURegs:$lhs, CPURegs:$rhs)>;
// SBITF = (s1) & ~((unsigned int) 1 << (s2))
def : Pat<(and CPURegs:$lhs, (xor (shl (i32 1), CPURegs:$rhs), (i32 -1))),
          (SBITF CPURegs:$lhs, CPURegs:$rhs)>;

// Boolean Patterns
// NANDL = (((s1) == 0) | ((s2) == 0)) ? 1 : 0
def : Pat<(selectcc (or (setcc CPURegs:$lhs, (i32 0), SETEQ),
                        (setcc CPURegs:$rhs, (i32 0), SETEQ)),
                    (i1 0), (i32 1), (i32 0), SETNE),
          (NANDL CPURegs:$lhs, CPURegs:$rhs)>;

// NORL = (((s1) == 0) & ((s2) == 0)) ? 1 : 0
def : Pat<(selectcc (i32 (or CPURegs:$lhs, CPURegs:$rhs)),
                    (i32 0), (i32 1), (i32 0), SETEQ),
          (NORL CPURegs:$lhs, CPURegs:$rhs)>;

// ORL = (((s1) == 0) & ((s2) == 0)) ? 0 : 1
def : Pat<(zext (setcc (or CPURegs:$lhs, CPURegs:$rhs), (i32 0), SETNE)),
          (ORL CPURegs:$lhs, CPURegs:$rhs)>;

// ANDC = ~(s1) & (s2)
def : Pat<(and (not CPURegs:$lhs), CPURegs:$rhs),
          (ANDC CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(and (not CPURegs:$lhs), imm:$rhs),
          (ANDCi CPURegs:$lhs, imm:$rhs)>;

// ANDL = ((((s1) == 0) | ((s2) == 0)) ? 0 : 1)
def : Pat<(zext (and (setcc CPURegs:$lhs, (i32 0), SETNE),
               (setcc CPURegs:$rhs, (i32 0), SETNE))),
          (ANDL CPURegs:$lhs, CPURegs:$rhs)>;

// ORC = (~(s1)) | (s2)
def : Pat<(or (xor CPURegs:$lhs, (i32 -1)), CPURegs:$rhs),
          (ORC CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(or (not CPURegs:$lhs), CPURegs:$rhs),
          (ORC CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(or (not CPURegs:$lhs), imm:$rhs),
          (ORCi CPURegs:$lhs, imm:$rhs)>;


// Multiplications Patterns
//def MULHHU  // ui16(s1 >> 16) * ui16(s2 >> 16)
def : Pat<(mul (srl CPURegs:$lhs, (i32 16)), (srl CPURegs:$rhs, (i32 16))),
          (MULHHU CPURegs:$lhs, CPURegs:$rhs)>;

//def MULLHU  // ui16(s1) * ui16(s2 >> 16)
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (srl CPURegs:$rhs, (i32 16))),
          (MULLHU CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (and CPURegs:$lhs, (i32 0xffff)), (srl CPURegs:$rhs, (i32 16))),
          (MULLHU CPURegs:$lhs, CPURegs:$rhs)>;

//def MULHH   // i16(s1 >> 16) * i16(s2 >> 16)
def : Pat<(mul (sra CPURegs:$lhs, (i32 16)), (sra CPURegs:$rhs, (i32 16))),
          (MULHH CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (sra CPURegs:$lhs, (i32 16)), (sra imm:$rhs, (i32 16))),
          (MULHH CPURegs:$lhs, imm:$rhs)>;

//def MULLH   // i16(s1) * i16(s2 >> 16)
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (sra CPURegs:$rhs, (i32 16))),
          (MULLH CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (sra imm:$rhs, (i32 16))),
          (MULLH CPURegs:$lhs, imm:$rhs)>;

//def MULHS   // s1 * ui16(s2 >> 16) << 16
def : Pat<(shl (mul CPURegs:$lhs, (srl CPURegs:$rhs, (i32 16))), (i32 16)),
          (MULHS CPURegs:$lhs, CPURegs:$rhs)>;

//def MULH    // s1 * i16(s2 >> 16)
def : Pat<(mul CPURegs:$lhs, (sra CPURegs:$rhs, (i32 16))),
          (MULH CPURegs:$lhs, CPURegs:$rhs)>;

//def MULHU   // s1 * ui16(s2 >> 16)
def : Pat<(mul CPURegs:$lhs, (srl CPURegs:$rhs, (i32 16))),
          (MULHU CPURegs:$lhs, CPURegs:$rhs)>;

//def MULLL   // i16(s1) * i16(s2)
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (sra (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLL CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (sext_inreg CPURegs:$lhs, i16), (sext_inreg CPURegs:$rhs, i16)),
          (MULLL CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)), immSExt16:$rhs),
          (MULLLi CPURegs:$lhs, immSExt16:$rhs)>; 

//def MULLLU  // ui16(s1) * ui16(s2)
def : Pat<(mul (and CPURegs:$lhs, (i32 0xffff)), (and CPURegs:$rhs, (i32 0xffff))),
          (MULLLU CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)), 
               (srl (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLLU CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)), immZExt16:$rhs),
          (MULLLUi CPURegs:$lhs, immZExt16:$rhs)>;

//def MULLU   // s1 * ui16(s2)
def : Pat<(mul CPURegs:$lhs, (and CPURegs:$rhs, (i32 0xFFFF))),
          (MULLU CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, (srl (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLU CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, immZExt16:$rhs),
          (MULLUi CPURegs:$lhs, immZExt16:$rhs)>;

//def MULL    // s1 * i16(s2)
def : Pat<(mul CPURegs:$lhs, (sra (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULL CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, (sext_inreg CPURegs:$rhs, i16)),
          (MULL CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, immSExt16:$rhs),
          (MULLi CPURegs:$lhs, immSExt16:$rhs)>;

def : Pat<(mul CPURegs:$lhs, CPURegs:$rhs),
          (ADD (MULLU CPURegs:$lhs, CPURegs:$rhs),
                (MULHS CPURegs:$lhs, CPURegs:$rhs))>;
def : Pat<(mul CPURegs:$lhs, imm:$rhs),
          (ADD (MULLUi CPURegs:$lhs, imm:$rhs),
                (MULHS CPURegs:$lhs, CPURegs:$rhs))>;
/*
// MUL patterns that use a chain for when accessing OpenCL regs
def : Pat<(le1_mulch CPURegs:$lhs, CPURegs:$rhs),
          (ADDCH (MULLU CPURegs:$lhs, CPURegs:$rhs),
                 (MULHS CPURegs:$lhs, CPURegs:$rhs))>;

def : Pat<(le1_mulch CPURegs:$lhs, imm:$rhs),
          (ADDCH (MULLUi CPURegs:$lhs, imm:$rhs),
                 (MULHS CPURegs:$lhs, CPURegs:$rhs))>;*/

// LE1 does not have "not", so we expand our way
def : Pat<(not CPURegs:$in),
          (ORC CPURegs:$in, ZERO)>;

// extended load and stores
def : Pat<(extloadi1  addr:$src), (LDBu addr:$src)>;
def : Pat<(extloadi8  addr:$src), (LDBu addr:$src)>;
def : Pat<(extloadi16 addr:$src), (LDHu addr:$src)>;

// Load and Store Branch Registers
def : Pat<(store BRegs:$src, addr:$dst),
          (STW (MFB BRegs:$src), addr:$dst)>;
def : Pat<(extloadi1 addr:$src),
          (MTB (LDBu addr:$src))>;

multiclass GlobalLoadPats<Instruction MemOp, PatFrag LoadFrag> {
  def : Pat<(i32 (LoadFrag (le1_targetglobal tglobaladdr:$global))),
            (i32 (MemOp tglobaladdr:$global, 0))>;

  def : Pat<(i32 (LoadFrag (add (le1_targetglobal tglobaladdr:$global),
                                                  u16ImmPred:$offset))),
            (i32 (MemOp tglobaladdr:$global, u16ImmPred:$offset))>;

  def : Pat<(i32 (LoadFrag (le1_globalconst tglobaladdr:$global))),
            (i32 (MemOp tglobaladdr:$global, 0))>;

  def : Pat<(i32 (LoadFrag (add (le1_globalconst tglobaladdr:$global),
                                                u16ImmPred:$offset))),
            (i32 (MemOp tglobaladdr:$global, u16ImmPred:$offset))>;

}
defm : GlobalLoadPats<LDBu_G, extloadi8>;
defm : GlobalLoadPats<LDBu_G, zextloadi8>;
defm : GlobalLoadPats<LDHu_G, extloadi16>;
defm : GlobalLoadPats<LDHu_G, zextloadi16>;
defm : GlobalLoadPats<LDB_G, sextloadi8>;
defm : GlobalLoadPats<LDH_G, sextloadi16>;
defm : GlobalLoadPats<LDW_G, load>;

multiclass GlobalStorePats<Instruction MemOp, PatFrag StoreFrag> {
  def : Pat<(StoreFrag CPURegs:$src, (le1_targetglobal tglobaladdr:$dst)),
            (MemOp CPURegs:$src, tglobaladdr:$dst, 0)>;

  def : Pat<(StoreFrag CPURegs:$src, (add (le1_targetglobal tglobaladdr:$dst), imm:$offset)),
            (MemOp CPURegs:$src, tglobaladdr:$dst, imm:$offset)>;

  def : Pat<(StoreFrag CPURegs:$src, (le1_globalconst tglobaladdr:$dst)),
            (MemOp CPURegs:$src, tglobaladdr:$dst, 0)>;

  def : Pat<(StoreFrag CPURegs:$src, (add (le1_globalconst tglobaladdr:$dst), imm:$offset)),
            (MemOp CPURegs:$src, tglobaladdr:$dst, imm:$offset)>;

}
defm : GlobalStorePats<STB_G, truncstorei8>;
defm : GlobalStorePats<STH_G, truncstorei16>;
defm : GlobalStorePats<STW_G, store>;

/*
def : Pat<(extloadi8 (le1_targetglobal tglobaladdr:$global)),
          (LDBu_G tglobaladdr:$global, 0)>;
def : Pat<(extloadi8 (add (le1_targetglobal tglobaladdr:$global), imm:$offset)),
          (LDBu_G tglobaladdr:$global, imm:$offset)>;

def : Pat<(extloadi16 (le1_targetglobal tglobaladdr:$global)),
          (LDHu_G tglobaladdr:$global, 0)>;
def : Pat<(extloadi16 (add (le1_targetglobal tglobaladdr:$global), imm:$offset)),
          (LDHu_G tglobaladdr:$global, imm:$offset)>;

def : Pat<(zextloadi8 (le1_targetglobal tglobaladdr:$global)),
          (LDBu_G tglobaladdr:$global, 0)>;
def : Pat<(zextloadi8 (add (le1_targetglobal tglobaladdr:$global), imm:$offset)),
          (LDBu_G tglobaladdr:$global, imm:$offset)>;

def : Pat<(zextloadi16 (le1_targetglobal tglobaladdr:$global)),
          (LDHu_G tglobaladdr:$global, 0)>;
def : Pat<(zextloadi16 (add (le1_targetglobal tglobaladdr:$global), imm:$offset)),
          (LDHu_G tglobaladdr:$global, imm:$offset)>;

def : Pat<(sextloadi8 (le1_targetglobal tglobaladdr:$global)),
          (LDB_G tglobaladdr:$global, 0)>;
def : Pat<(sextloadi8 (add (le1_targetglobal tglobaladdr:$global), imm:$offset)),
          (LDB_G tglobaladdr:$global, imm:$offset)>;

def : Pat<(sextloadi16 (le1_targetglobal tglobaladdr:$global)),
          (LDH_G tglobaladdr:$global, 0)>;
def : Pat<(sextloadi16 (add (le1_targetglobal tglobaladdr:$global), imm:$offset)),
          (LDH_G tglobaladdr:$global, imm:$offset)>;

def : Pat<(load (add (le1_targetglobal (i32 tglobaladdr:$global)), imm:$offset)),
          (LDW_G tglobaladdr:$global, imm:$offset)>;
def : Pat<(load (le1_targetglobal (i32 tglobaladdr:$global))),
          (LDW_G tglobaladdr:$global, 0)>;

def : Pat<(truncstorei8 CPURegs:$src, (le1_targetglobal tglobaladdr:$dst)),
          (STB_G CPURegs:$src, tglobaladdr:$dst, 0)>;
def : Pat<(truncstorei16 CPURegs:$src, (le1_targetglobal tglobaladdr:$dst)),
          (STH_G CPURegs:$src, tglobaladdr:$dst, 0)>;
def : Pat<(store CPURegs:$src, (add (le1_targetglobal (i32 tglobaladdr:$dst)), imm:$offset)),
          (STW_G CPURegs:$src, tglobaladdr:$dst, imm:$offset)>;
def : Pat<(store CPURegs:$src, (le1_targetglobal (i32 tglobaladdr:$dst))),
          (STW_G CPURegs:$src, tglobaladdr:$dst, 0)>;
*/
def : Pat<(le1_loadlink tglobaladdr:$addr, imm:$offset),
          (LDL tglobaladdr:$addr, imm:$offset)>;

// immediates
def : Pat<(i32 0),
          (MOVr ZERO)>;
def : Pat<(i32 imm:$in),
          (MOVi imm:$in)>;
//def : Pat<(le1_globalconst tglobaladdr:$addr),
  //        (MOVg tglobaladdr:$addr)>;
//def : Pat<(le1_targetglobal tglobaladdr:$addr),
  //        (MOVg tglobaladdr:$addr)>;

//def : Pat<(i32 tglobaladdr:$addr),
  //        (MOVg tglobaladdr:$addr)>;
//def : Pat<(le1_targetglobal tglobaladdr:$addr),
  //        (MOVg tglobaladdr:$addr)>;
//def : Pat<(le1_mtl (le1_targetglobal tglobaladdr:$addr)),
  //        (MTLi tglobaladdr:$addr)>;

// peepholes
def : Pat<(store (i32 0), addr:$dst), (STW ZERO, addr:$dst)>;


// Branch Patterns
multiclass BranchPats<Instruction BrOp, Instruction CmpOpR, Instruction CmpOpI, 
                      PatFrag cond> {
  def : Pat<(brcond (i1 (cond CPURegs:$lhs, CPURegs:$rhs)), bb:$dst),
            (BrOp (CmpOpR CPURegs:$lhs, CPURegs:$rhs), bb:$dst)>;
  def : Pat<(brcond (i1 (cond CPURegs:$lhs, imm:$rhs)), bb:$dst),
            (BrOp (CmpOpI CPURegs:$lhs, imm:$rhs), bb:$dst)>;
  def : Pat<(brcond (i1 (cond CPURegs:$lhs, 0)), bb:$dst),
            (BrOp (CmpOpR CPURegs:$lhs, ZERO), bb:$dst)>;
}

defm : BranchPats<BR, CMPEQ,   CMPEQi,   seteq>;
defm : BranchPats<BR, CMPNE,   CMPNEi,   setne>;
defm : BranchPats<BR, CMPLT,   CMPLTi,   setlt>;
defm : BranchPats<BR, CMPLTU,  CMPLTUi,  setult>;
defm : BranchPats<BR, CMPLE,   CMPLEi,   setle>;
defm : BranchPats<BR, CMPLEU,  CMPLEUi,  setule>;
defm : BranchPats<BR, CMPGT,   CMPGTi,   setgt>;
defm : BranchPats<BR, CMPGTU,  CMPGTUi,  setugt>;
defm : BranchPats<BR, CMPGE,   CMPGEi,   setge>;
defm : BranchPats<BR, CMPGEU,  CMPGEUi,  setuge>;

defm : BranchPats<BRF, CMPEQ,   CMPEQi,   setne>;
defm : BranchPats<BRF, CMPNE,   CMPNEi,   seteq>;
defm : BranchPats<BRF, CMPLT,   CMPLTi,   setge>;
defm : BranchPats<BRF, CMPLTU,  CMPLTUi,  setuge>;
defm : BranchPats<BRF, CMPLE,   CMPLEi,   setgt>;
defm : BranchPats<BRF, CMPLEU,  CMPLEUi,  setugt>;
defm : BranchPats<BRF, CMPGT,   CMPGTi,   setle>;
defm : BranchPats<BRF, CMPGTU,  CMPGTUi,  setule>;
defm : BranchPats<BRF, CMPGE,   CMPGEi,   setlt>;
defm : BranchPats<BRF, CMPGEU,  CMPGEUi,  setult>;

def : Pat<(le1_call L0, L0),
          (LNKCALL L0)>;
def : Pat<(le1_call L0, (le1_targetglobal tglobaladdr:$addr)),
          (CALL L0, tglobaladdr:$addr)>;
def : Pat<(br bb:$brtarget),
          (GOTO bb:$brtarget)>;

def : Pat<(brcond (i1 (setne BRegs:$src, (i1 0))), bb:$offset),
          (BRF BRegs:$src, bb:$offset)>;
def : Pat<(brcond (i1 (setne BRegs:$src, (i1 -1))), bb:$offset),
          (BRF BRegs:$src, bb:$offset)>;
def : Pat<(brcond (i1 (seteq BRegs:$src, (i1 0))), bb:$offset),
          (BR BRegs:$src, bb:$offset)>;
def : Pat<(brcond (i1 (seteq BRegs:$src, (i1 -1))), bb:$offset),
          (BR BRegs:$src, bb:$offset)>;

def : Pat<(brcond (i1 (seteq BRegs:$cond, (i1 1))), bb:$offset),
          (BR BRegs:$cond, bb:$offset)>;
def : Pat<(brcond (i1 (setne BRegs:$cond, (i1 0))), bb:$offset),
          (BRF BRegs:$cond, bb:$offset)>;

def : Pat<(brcond BRegs:$cond, bb:$dst),
          (BR BRegs:$cond, bb:$dst)>;

// Max and Min Patterns
multiclass MaxMinPats<Instruction OpR, Instruction OpI, CondCode CC,
                      CondCode AltCC> {
  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, CPURegs:$lhs, CPURegs:$rhs,
                                                                          CC),
            (OpR CPURegs:$lhs, CPURegs:$rhs)>;
  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, CPURegs:$lhs, imm:$rhs, CC),
            (OpI CPURegs:$lhs, imm:$rhs)>;

  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, CPURegs:$rhs, CPURegs:$lhs, AltCC),
            (OpR CPURegs:$lhs, imm:$rhs)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, imm:$rhs, CPURegs:$lhs, AltCC),
            (OpI CPURegs:$lhs, imm:$rhs)>;

}

defm : MaxMinPats<MAX,  MAXi,   SETGT,  SETLE>;
defm : MaxMinPats<MAX,  MAXi,   SETGE,  SETLT>;
defm : MaxMinPats<MIN,  MINi,   SETLT,  SETGE>;
defm : MaxMinPats<MIN,  MINi,   SETLE,  SETGT>;
defm : MaxMinPats<MAXU, MAXUi,  SETUGT, SETULE>;
defm : MaxMinPats<MAXU, MAXUi,  SETUGE, SETULT>;
defm : MaxMinPats<MINU, MINUi,  SETULT, SETUGE>;
defm : MaxMinPats<MINU, MINUi,  SETULE, SETUGT>;


// setcc patterns
multiclass ExtendCmpPats<Instruction CmpOpR, Instruction CmpOpI, CondCode CC> {
  def : Pat<(anyext (setcc CPURegs:$lhs, CPURegs:$rhs, CC)),
            (CmpOpR CPURegs:$lhs, CPURegs:$rhs)>; 
  def : Pat<(zext (setcc CPURegs:$lhs, CPURegs:$rhs, CC)),
            (CmpOpR CPURegs:$lhs, CPURegs:$rhs)>; 
  def : Pat<(sext (setcc CPURegs:$lhs, CPURegs:$rhs, CC)),
            (CmpOpR CPURegs:$lhs, CPURegs:$rhs)>; 
  def : Pat<(anyext (setcc CPURegs:$lhs, imm:$rhs, CC)),
            (CmpOpI CPURegs:$lhs, imm:$rhs)>;
  def : Pat<(zext (setcc CPURegs:$lhs, imm:$rhs, CC)),
            (CmpOpI CPURegs:$lhs, imm:$rhs)>;
  def : Pat<(sext (setcc CPURegs:$lhs, imm:$rhs, CC)),
            (CmpOpI CPURegs:$lhs, imm:$rhs)>;
  // Peepholes
  def : Pat<(anyext (setcc CPURegs:$lhs, 0, CC)),
            (CmpOpR CPURegs:$lhs, 0)>;
  def : Pat<(zext (setcc CPURegs:$lhs, 0, CC)),
            (CmpOpR CPURegs:$lhs, 0)>;
  def : Pat<(sext (setcc CPURegs:$lhs, 0, CC)),
            (CmpOpR CPURegs:$lhs, 0)>;

}

multiclass CmpPats<Instruction CmpOpR, Instruction CmpOpI, CondCode CC> {
  // Peepholes
  def : Pat<(setcc CPURegs:$lhs, 0, CC),
            (CmpOpR CPURegs:$lhs, ZERO)>;

  def : Pat<(setcc CPURegs:$lhs, CPURegs:$rhs, CC),
            (CmpOpR CPURegs:$lhs, CPURegs:$rhs)>;
  def : Pat<(setcc CPURegs:$lhs, imm:$rhs, CC),
            (CmpOpI CPURegs:$lhs, imm:$rhs)>;

}

defm : ExtendCmpPats<CMPEQr,  CMPEQri,  SETEQ>;
defm : ExtendCmpPats<CMPNEr,  CMPNEri,  SETNE>;
defm : ExtendCmpPats<CMPLTr,  CMPLTri,  SETLT>;
defm : ExtendCmpPats<CMPLTUr, CMPLTUri, SETULT>;
defm : ExtendCmpPats<CMPLEr,  CMPLEri,  SETLE>;
defm : ExtendCmpPats<CMPLEUr, CMPLEUri, SETULE>;
defm : ExtendCmpPats<CMPGTr,  CMPGTri,  SETGT>;
defm : ExtendCmpPats<CMPGTUr, CMPGTUri, SETUGT>;
defm : ExtendCmpPats<CMPGEr,  CMPGEri,  SETGE>;
defm : ExtendCmpPats<CMPGEUr, CMPGEUri, SETUGE>;

defm : CmpPats<CMPEQ,   CMPEQi,   SETEQ>;
defm : CmpPats<CMPNE,   CMPNEi,   SETNE>;
defm : CmpPats<CMPLT,   CMPLTi,   SETLT>;
defm : CmpPats<CMPLTU,  CMPLTUi,  SETULT>;
defm : CmpPats<CMPLE,   CMPLEi,   SETLE>;
defm : CmpPats<CMPLEU,  CMPLEUi,  SETULE>;
defm : CmpPats<CMPGT,   CMPGTi,   SETGT>;
defm : CmpPats<CMPGTU,  CMPGTUi,  SETUGT>;
defm : CmpPats<CMPGE,   CMPGEi,   SETGE>;
defm : CmpPats<CMPGEU,  CMPGEUi,  SETUGE>;

// Setcc Patterns to handle trying to compare i1s
def : Pat<(setcc BRegs:$lhs, (i1 0), SETEQ),
          (CMPEQ (MFB BRegs:$lhs), ZERO)>;
def : Pat<(setcc BRegs:$lhs, (i1 0), SETNE),
          (CMPNE (MFB BRegs:$lhs), ZERO)>;
def : Pat<(setcc BRegs:$lhs, (i1 -1), SETEQ),
          (CMPEQ (MFB BRegs:$lhs), (i32 -1))>;
def : Pat<(setcc BRegs:$lhs, (i1 -1), SETNE),
          (CMPNE (MFB BRegs:$lhs), (i32 -1))>;

def : Pat<(setcc BRegs:$lhs, (i1 0), SETLT),
          (CMPLT (MFB BRegs:$lhs), ZERO)>;
def : Pat<(setcc BRegs:$lhs, (i1 -1), SETLT),
          (CMPLT (MFB BRegs:$lhs), (i32 -1))>;
def : Pat<(setcc BRegs:$lhs, (i1 0), SETGT),
          (CMPGT (MFB BRegs:$lhs), ZERO)>;
def : Pat<(setcc BRegs:$lhs, (i1 -1), SETGT),
          (CMPGT (MFB BRegs:$lhs), (i32 -1))>;

multiclass SlctPats<Instruction CmpOpR, Instruction CmpOpI, CondCode CC> {
  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, CPURegs:$T, CPURegs:$F, CC),
            (SLCTrr (i1 (CmpOpR CPURegs:$lhs, CPURegs:$rhs)),
                                                      CPURegs:$T, CPURegs:$F)>;

  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, CPURegs:$T, imm:$F, CC),
            (SLCTri (i1 (CmpOpR CPURegs:$lhs, CPURegs:$rhs)),
                                                      CPURegs:$T, imm:$F)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, CPURegs:$T, CPURegs:$F, CC),
            (SLCTrr (i1 (CmpOpI CPURegs:$lhs, imm:$rhs)),
                                                      CPURegs:$T, CPURegs:$F)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, CPURegs:$T, imm:$F, CC),
            (SLCTri (i1 (CmpOpI CPURegs:$lhs, imm:$rhs)),
                                                      CPURegs:$T, imm:$F)>;

  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, imm:$T, CPURegs:$F, CC),
            (SLCTFrr (i1 (CmpOpR CPURegs:$lhs, CPURegs:$rhs)),
                                                      CPURegs:$F, imm:$T)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, imm:$T, CPURegs:$F, CC),
            (SLCTFri (i1 (CmpOpI CPURegs:$lhs, imm:$rhs)),
                                                      CPURegs:$F, imm:$T)>;

  // Peephole optimisations
  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, 0, imm:$F, CC),
            (SLCTrr (i1 (CmpOpR CPURegs:$lhs, CPURegs:$rhs)), ZERO, imm:$F)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, 0, imm:$F, CC),
            (SLCTri (i1 (CmpOpI CPURegs:$lhs, imm:$rhs)), ZERO, imm:$F)>;

  def : Pat<(selectcc CPURegs:$lhs, CPURegs:$rhs, imm:$T, 0, CC),
            (SLCTFrr (i1 (CmpOpR CPURegs:$rhs, CPURegs:$lhs)), ZERO, imm:$T)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, imm:$T, 0, CC),
            (SLCTFri (i1 (CmpOpI CPURegs:$lhs, imm:$rhs)), ZERO, imm:$T)>;

  def : Pat<(selectcc CPURegs:$lhs, imm:$rhs, 0, CPURegs:$F, CC),
            (SLCTrr (i1 (CmpOpI CPURegs:$lhs, imm:$rhs)), ZERO, CPURegs:$F)>;


}

defm : SlctPats<CMPEQ,  CMPEQi,   SETEQ>;
defm : SlctPats<CMPNE,  CMPNEi,   SETNE>;
defm : SlctPats<CMPLT,  CMPLTi,   SETLT>;
defm : SlctPats<CMPLTU, CMPLTUi,  SETULT>;
defm : SlctPats<CMPLE,  CMPLEi,   SETLE>;
defm : SlctPats<CMPLEU, CMPLEUi,  SETULE>;
defm : SlctPats<CMPGT,  CMPGTi,   SETGT>;
defm : SlctPats<CMPGTU, CMPGTUi,  SETUGT>;
defm : SlctPats<CMPGE,  CMPGEi,   SETGE>;
defm : SlctPats<CMPGEU, CMPGEUi,  SETUGE>;

//Select Patterns
def : Pat<(select (i1 BRegs:$cond), (i32 CPURegs:$T), (i32 CPURegs:$F)),
          (SLCTrr BRegs:$cond, CPURegs:$T, CPURegs:$F)>;
def : Pat<(select (i1 BRegs:$cond), (i32 CPURegs:$T), (imm:$F)),
          (SLCTri BRegs:$cond, CPURegs:$T, imm:$F)>;
// Peepholes
def : Pat<(select (i1 BRegs:$cond), (i32 CPURegs:$T), (i32 0)),
          (SLCTrr BRegs:$cond, CPURegs:$T, ZERO)>;
def : Pat<(select (i1 BRegs:$cond), (i32 0), (i32 CPURegs:$F)),
          (SLCTrr BRegs:$cond, ZERO, CPURegs:$F)>;
def : Pat<(select (i1 BRegs:$cond), imm:$T, (i32 0)),
          (SLCTFri BRegs:$cond, ZERO, imm:$T)>;
def : Pat<(select (i1 BRegs:$cond), (i32 0), imm:$F),
          (SLCTri BRegs:$cond, ZERO, imm:$F)>;

// Branch Regs Move Patterns
def : Pat<(zext BRegs:$src),
          (MFB BRegs:$src)>;
def : Pat<(trunc CPURegs:$src),
          (MTB CPURegs:$src)>;
def : Pat<(not (trunc CPURegs:$src)),
          (MTBF CPURegs:$src)>;

// FIXME don't understand the i1 Constant<-1> thing and I think i1 0 is also
// causing inefficient code gen
def : Pat<(i1 0),
          (MTB ZERO)>;
def : Pat<(i1 -1),
          (MTBF ZERO)>;
def : Pat<(anyext BRegs:$in),
          (MFB BRegs:$in)>;


// Peepholes using the zero reg
//def : Pat<(i32 0),
  //        (MOVr ZERO)>;
def : Pat<(sub (i32 0), CPURegs:$in),
          (SUB ZERO, CPURegs:$in)>;

//include "LE1InstrInfoNew.td"
include "LE1IntrinsicInstrInfo.td"
