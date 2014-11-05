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

def SDTLE1LogicCmp    : SDTypeProfile<1, 2, [SDTCisInt<0>,
                                             SDTCisVT<1, i32>,
                                             SDTCisSameAs<1, 2>]>;

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
def le1_mull    : SDNode<"LE1ISD::MULL", SDTIntBinOp, [SDNPCommutative]>;
def le1_mullu   : SDNode<"LE1ISD::MULLU", SDTIntBinOp, [SDNPCommutative]>;
def le1_mulh    : SDNode<"LE1ISD::MULH", SDTIntBinOp, []>;
def le1_mulhu   : SDNode<"LE1ISD::MULHU", SDTIntBinOp, []>;
def le1_mulhs   : SDNode<"LE1ISD::MULHS", SDTIntBinOp, []>;
def le1_mulll   : SDNode<"LE1ISD::MULLL", SDTIntBinOp, []>;
def le1_mulllu  : SDNode<"LE1ISD::MULLLU", SDTIntBinOp, []>;
def le1_mullh   : SDNode<"LE1ISD::MULLH", SDTIntBinOp, []>;
def le1_mullhu  : SDNode<"LE1ISD::MULLHU", SDTIntBinOp, []>;
def le1_mulhh   : SDNode<"LE1ISD::MULHH", SDTIntBinOp, [SDNPCommutative]>;
def le1_mulhhu  : SDNode<"LE1ISD::MULHHU", SDTIntBinOp, [SDNPCommutative]>;

def le1_addcg   : SDNode<"LE1ISD::ADDCG", SDT_LE1CarryUseGen, []>;
def le1_divs    : SDNode<"LE1ISD::DIVS", SDT_LE1CarryUseGen, []>;

// Shift Adds
def le1_sh1add  : SDNode<"LE1ISD::SH1ADD", SDTIntShiftOp, []>;
def le1_sh2add  : SDNode<"LE1ISD::SH2ADD", SDTIntShiftOp, []>;
def le1_sh3add  : SDNode<"LE1ISD::SH3ADD", SDTIntShiftOp, []>;
def le1_sh4add  : SDNode<"LE1ISD::SH4ADD", SDTIntShiftOp, []>;

// Extend Nodes
def le1_sxtb    : SDNode<"LE1ISD::SXTB", SDTExtInreg, []>;
def le1_sxth    : SDNode<"LE1ISD::SXTH", SDTExtInreg, []>;
def le1_zxtb    : SDNode<"LE1ISD::ZXTB", SDTExtInreg, []>;
def le1_zxth    : SDNode<"LE1ISD::ZXTH", SDTExtInreg, []>;

// Max and Min Operators
def le1_maxs    : SDNode<"LE1ISD::MAXS", SDTIntBinOp, [SDNPCommutative]>;
def le1_maxu    : SDNode<"LE1ISD::MAXU", SDTIntBinOp, [SDNPCommutative]>;
def le1_mins    : SDNode<"LE1ISD::MINS", SDTIntBinOp, [SDNPCommutative]>;
def le1_minu    : SDNode<"LE1ISD::MINU", SDTIntBinOp, [SDNPCommutative]>;

// Boolean Operators
def le1_nandl   : SDNode<"LE1ISD::NANDL",   SDTLE1LogicCmp, [SDNPCommutative]>;
def le1_norl    : SDNode<"LE1ISD::NORL",    SDTLE1LogicCmp, [SDNPCommutative]>;
def le1_orl     : SDNode<"LE1ISD::ORL",     SDTLE1LogicCmp, [SDNPCommutative]>;
def le1_andc    : SDNode<"LE1ISD::ANDC",    SDTLE1LogicCmp, []>;
def le1_andl    : SDNode<"LE1ISD::ANDL",    SDTLE1LogicCmp, [SDNPCommutative]>;
def le1_orc     : SDNode<"LE1ISD::ORC",     SDTLE1LogicCmp, []>;
def le1_cmpeq   : SDNode<"LE1ISD::CMPEQ",   SDTLE1LogicCmp, [SDNPCommutative]>;
def le1_cmpge   : SDNode<"LE1ISD::CMPGE",   SDTLE1LogicCmp, []>;
def le1_cmpgeu  : SDNode<"LE1ISD::CMPGEU",  SDTLE1LogicCmp, []>;
def le1_cmpgt   : SDNode<"LE1ISD::CMPGT",   SDTLE1LogicCmp, []>;
def le1_cmpgtu  : SDNode<"LE1ISD::CMPGTU",  SDTLE1LogicCmp, []>;
def le1_cmple   : SDNode<"LE1ISD::CMPLE",   SDTLE1LogicCmp, []>;
def le1_cmpleu  : SDNode<"LE1ISD::CMPLEU",  SDTLE1LogicCmp, []>;
def le1_cmplt   : SDNode<"LE1ISD::CMPLT",   SDTLE1LogicCmp, []>;
def le1_cmpltu  : SDNode<"LE1ISD::CMPLTU",  SDTLE1LogicCmp, []>;
def le1_cmpne   : SDNode<"LE1ISD::CMPNE",   SDTLE1LogicCmp, [SDNPCommutative]>;

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
//def simm16_64   : Operand<i64>;
def globaladdress : Operand<i32>;

//def i32imm      : Operand<i32>;
//def uimm32      : Operand<i32> {
  //let PrintMethod = "printUnsignedImm";
//}

// Unsigned Operand
//def uimm16      : Operand<i32> {
  //let PrintMethod = "printUnsignedImm";
//}

def immediate : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

def Op_simm9 : Operand<i32>;
def Op_uimm9 : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}
def Op_simm12 : Operand<i32>;
def Op_uimm12 : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}
def Op_simm32 : Operand<i32>;
def Op_uimm32 : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

def globaladdroffset : Operand<i32> {
  let PrintMethod = "printGlobalOffset";
}

// Address operand
def mem : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops CPURegs, Op_simm32);
  let EncoderMethod = "getMemEncoding";
}
/*
def mem_ea : Operand<i32> {
  let PrintMethod = "printMemOperandEA";
  let MIOperandInfo = (ops CPURegs, simm16);
  let EncoderMethod = "getMemEncoding";
}*/

def imm9    : ImmLeaf<i32, [{ return (isUInt<9>(Imm)); }]>;
def imm12   : ImmLeaf<i32, [{ return (isUInt<12>(Imm)); }]>;
def imm16   : ImmLeaf<i32, [{ return (isUInt<16>(Imm)); }]>;
def imm32   : ImmLeaf<i32,
[{  if (isUInt<9>(Imm)) return false;
    if (isUInt<12>(Imm)) return false;
    return (isUInt<32>(Imm)); }]>;

def simm9   : ImmLeaf<i32, [{ return (isInt<9>(Imm)); }]>;
def uimm9   : ImmLeaf<i32, [{ return (isUInt<9>(Imm)); }]>;
def simm12  : ImmLeaf<i32, [{ return (isInt<12>(Imm)); }]>;
def simm16  : ImmLeaf<i32, [{ return (isInt<16>(Imm)); }]>;
def uimm12  : ImmLeaf<i32, [{ return (isUInt<12>(Imm)); }]>;
def simm32  : ImmLeaf<i32,
[{  if (isInt<9>(Imm)) return false;
    return (isInt<32>(Imm)); }]>;
def uimm32  : ImmLeaf<i32,
[{  if (isUInt<9>(Imm)) return false;
    return (isUInt<32>(Imm)); }]>;

/*
def imm9  : ImmLeaf<i32, [{ return (Imm == (Imm & 0x1FF)); }]>;
def imm12 : ImmLeaf<i32, [{ return (Imm == (Imm & 0xFFF)); }]>;
def imm16 : ImmLeaf<i32, [{ return (Imm == (Imm & 0xFFFF)); }]>;
def imm32 : ImmLeaf<i32, [{ return ((Imm != (Imm & 0x1FF)) &&
                                    (Imm != (Imm & 0xFFF))); }]>;*/

def addr_i8   : ComplexPattern<iPTR, 2, "SelectAddri8", [frameindex], []>;
def addr_i12  : ComplexPattern<iPTR, 2, "SelectAddri12", [frameindex], []>;
def addr_i32  : ComplexPattern<iPTR, 2, "SelectAddri32", [frameindex], []>;

//===----------------------------------------------------------------------===//
// Pattern fragment for load/store
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction Classes that utilise the Integer ALUs 
//===----------------------------------------------------------------------===//

// Arithmetic
class ArithLogic_R_RR<bits<5> opcode, string instr_asm, SDNode OpNode,
                      SchedWrite SC, bit isComm = 0>
  : LE1Format_R_RR<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, CPURegs:$src2),
  !strconcat(instr_asm, " $rd, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, CPURegs:$src2))], IIAlu>,
  Sched<[SC]> {
    let isCommutable = isComm;
}

class ArithLogic_R_I32R<bits<5> opcode, string instr_asm, SDNode OpNode,
                        ImmLeaf immPat, Operand immType, SchedWrite SC,
                        bit isComm = 0>
  : LE1Format_R_I32R<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src, immType:$imm32),
  !strconcat(instr_asm, " $rd, $src, $imm32"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src, immPat:$imm32))], IIAlu>,
  Sched<[SC]> {
    let isCommutable = isComm;
}

class ArithLogic_R_I9R<bits<5> opcode, string instr_asm, SDNode OpNode,
                       ImmLeaf immPat, Operand immType, SchedWrite SC,
                       bit isComm = 0>
  : LE1Format_R_I9R<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, immType:$imm9),
  !strconcat(instr_asm, " $rd, $src1, $imm9"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, immPat:$imm9))], IIAlu>,
  Sched<[SC]> {
    let isCommutable = isComm;
}

// Multiplication
class Mult_R_RR<bits<5> opcode, string instr_asm, PatFrag OpNode,
                bit isComm = 0>
  : LE1Format_R_RR<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, CPURegs:$src2),
  !strconcat(instr_asm, " $rd, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, CPURegs:$src2))], IIMul>,
  Sched<[WriteM]> {
    let isCommutable = isComm;
}

class Mult_R_I32R<bits<5> opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_R_I32R<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src, Op_simm32:$imm32),
  !strconcat(instr_asm, " $rd, $src, $imm32"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src, imm32:$imm32))], IIMul>,
  Sched<[WriteMI]>;

class Mult_R_I9R<bits<5> opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_R_I9R<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, Op_simm9:$imm9),
  !strconcat(instr_asm, " $rd, $src1, $imm9"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, imm9:$imm9))], IIMul>,
  Sched<[WriteM]>;


class LogicCompare_R_RR<bits<4> opcode, string instr_asm, SDNode OpNode>
  : LE1Format_R_RR_1<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, CPURegs:$src2),
  !strconcat(instr_asm, " $rd, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, CPURegs:$src2))], IIAlu>,
  Sched<[WriteA]>;

class LogicCompare_R_I32R<bits<4> opcode, string instr_asm, SDNode OpNode,
                          Operand immType, ImmLeaf immLeaf>
  : LE1Format_RoB_I32R<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, immediate:$imm32),
  !strconcat(instr_asm, " $rd, $src1, $imm32"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, imm32:$imm32))], IIAlu>,
  Sched<[WriteAI]> {
    let B = 0;
  }

class LogicCompare_R_I9R<bits<4> opcode, string instr_asm, SDNode OpNode,
                         Operand immType, ImmLeaf immLeaf>
  : LE1Format_RoB_I9R<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, immType:$imm9),
  !strconcat(instr_asm, "  $rd, $src1, $imm9"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, immLeaf:$imm9))], IIAlu>,
  Sched<[WriteA]> {
    let B = 0;
  }

class LogicCompare_P_RR<bits<4> opcode, string instr_asm, SDNode OpNode>
  : LE1Format_P_RR<
  opcode,
  (outs BRegs:$prd),
  (ins CPURegs:$src1, CPURegs:$src2),
  !strconcat(instr_asm, " $prd, $src1, $src2"),
  [(set BRegs:$prd, (OpNode CPURegs:$src1, CPURegs:$src2))], IIAlu>,
  Sched<[WriteA]>;

class LogicCompare_P_I9R<bits<4> opcode, string instr_asm, SDNode OpNode,
                         Operand immType, ImmLeaf immLeaf>
  : LE1Format_RoB_I9R<
  opcode,
  (outs BRegs:$rd),
  (ins CPURegs:$src1, immType:$imm9),
  !strconcat(instr_asm, "  $rd, $src1, $imm9"),
  [(set BRegs:$rd, (OpNode CPURegs:$src1, immLeaf:$imm9))], IIAlu>,
  Sched<[WriteA]> {
    let B = 1;
  }

class LogicCompare_P_I32R<bits<4> opcode, string instr_asm, SDNode OpNode,
                          Operand immType, ImmLeaf immLeaf>
  : LE1Format_RoB_I32R<
  opcode,
  (outs BRegs:$rd),
  (ins CPURegs:$src1, immType:$imm32),
  !strconcat(instr_asm, "  $rd, $src1, $imm32"),
  [(set BRegs:$rd, (OpNode CPURegs:$src1, immLeaf:$imm32))], IIAlu>,
  Sched<[WriteAI]> {
    let B = 1;
  }

//===----------------------------------------------------------------------===//
// Instruction Classes that utilise the Load/Store units
//===----------------------------------------------------------------------===//
// Loads
class LoadMem_I32<bits<5>opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_LD_I32<
    opcode,
    (outs CPURegs:$rd),
    (ins mem:$addr),
    !strconcat(instr_asm, "  $rd, $addr"),
    [(set CPURegs:$rd, (OpNode addr_i32:$addr))], IILoadStore>,
    Sched<[WriteLSI]>;

class LoadMem_I12<bits<2> opcode, bits<4> format, string instr_asm,
                  PatFrag OpNode>
  : LE1Format_ST_S<
  opcode,
  format,
  (outs CPURegs:$rd),
  (ins mem:$addr),
  !strconcat(instr_asm, " $rd, $addr"),
  [(set CPURegs:$rd, (OpNode addr_i12:$addr))], IILoadStore>,
  Sched<[WriteLS]>;

class LoadMem_I8<bits<6> opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_LDST_I8<
  opcode,
  (outs CPURegs:$rd),
  (ins mem:$addr),
  !strconcat(instr_asm, " $rd, $addr"),
  [(set CPURegs:$rd, (OpNode addr_i8:$addr))], IILoadStore>,
  Sched<[WriteLS]>;

// Stores
class StoreMem_I32<bits<5>opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_ST_I32<
    opcode,
    (outs),
    (ins CPURegs:$src1, mem:$addr),
    !strconcat(instr_asm, "  $addr, $src1"),
    [(OpNode CPURegs:$src1, addr_i32:$addr)], IILoadStore>,
    Sched<[WriteLSI]>;

class StoreMem_I12<bits<2> opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_ST_S<
  opcode,
  0x7,
  (outs),
  (ins CPURegs:$rd, mem:$addr),
  !strconcat(instr_asm, "  $addr, $rd"),
  [(OpNode CPURegs:$rd, addr_i12:$addr)], IILoadStore>,
  Sched<[WriteLS]>;

class StoreMem_I8<bits<6> opcode, string instr_asm, PatFrag OpNode>
  : LE1Format_LDST_I8<
  opcode,
  (outs),
  (ins CPURegs:$rd, mem:$addr),
  !strconcat(instr_asm, "  $addr, $rd"),
  [(OpNode CPURegs:$rd, addr_i8:$addr)], IILoadStore>,
  Sched<[WriteLS]>;

class Extend<bits<5> opcode, string instr_asm, SDNode OpNode, ValueType vt>
  : LE1Format_R_R<
    opcode,
    (outs CPURegs:$rd),
    (ins CPURegs:$src1),
    !strconcat(instr_asm, "  $rd, $src1"),
    [(set CPURegs:$rd, (OpNode CPURegs:$src1, vt))], IIAlu>,
    Sched<[WriteA]>;

class Select_R<bits<5> opcode, string instr_asm, SDNode OpNode>
  : LE1Format_R_RR_PT<
  opcode,
  (outs CPURegs:$rd),
  (ins BRegs:$prs, CPURegs:$src1, CPURegs:$src2),
  !strconcat(instr_asm, "  $rd, $prs, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode BRegs:$prs, CPURegs:$src1, CPURegs:$src2))],
  IIAlu>, Sched<[WriteA]>;

class Select_I32<bits<5> opcode, string instr_asm, SDNode OpNode>
  : LE1Format_R_I32_RP<
  opcode,
  (outs CPURegs:$rd),
  (ins BRegs:$prs, CPURegs:$src1, immediate:$src2),
  !strconcat(instr_asm, "  $rd, $prs, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode BRegs:$prs, CPURegs:$src1, imm32:$src2))],
  IIAlu>, Sched<[WriteAI]>;


// TODO Use or create a proper format
let isBranch = 1, isTerminator = 1 in //isBarrier = 1 in
class CBranch<string instr_asm, SDNode OpNode>:
      LE1Inst<(outs),
      (ins BRegs:$cond, brtarget:$dst),
      !strconcat(instr_asm, "  $cond, $dst"),
      [(OpNode BRegs:$cond, bb:$dst)], IIBranch>,
      Sched<[WriteB]>;


// ------------------- END OF NEW IMPLEMENTATIONS ----------------------- //
let isAsCheapAsAMove = 1 in
class MoveR<string instr_asm, SDNode OpNode, RegisterClass OutReg,
           RegisterClass InReg>:
      LE1Inst<(outs OutReg:$dst),(ins InReg:$src),
      !strconcat(instr_asm, "  $dst, $src"),
      [(set OutReg:$dst, (OpNode InReg:$src))], IIAlu>,
      Sched<[WriteA]>;


//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//
// Arithmetic and Logic Instructions
multiclass ARITH_LOGIC<bits<5> opcode, string instr_asm, SDNode OpNode,
                       InstrItinClass II, bit isComm = 0> {
  def NAME#r    : ArithLogic_R_RR<opcode, instr_asm, OpNode, WriteA, isComm>;

  def NAME#si32 : ArithLogic_R_I32R<opcode, instr_asm, OpNode, simm32,
                                    Op_simm32, WriteAI, 0>;

  def NAME#si9  : ArithLogic_R_I9R<opcode, instr_asm, OpNode, simm9, Op_simm9,
                                   WriteA, 0>;

  def NAME#i32 : ArithLogic_R_I32R<opcode, instr_asm, OpNode, uimm32,
                                   Op_uimm32, WriteAI, 0>;

  def NAME#i9  : ArithLogic_R_I9R<opcode, instr_asm, OpNode, uimm9, Op_uimm9,
                                  WriteA, 0>;
}

defm ADD      : ARITH_LOGIC<0x0,   "add.0", add, IIAlu, 1>;
defm AND      : ARITH_LOGIC<0x1,   "and.0", and, IIAlu, 1>;
defm ANDC     : ARITH_LOGIC<0x2,   "andc.0", le1_andc, IIAlu>;
defm MAXS     : ARITH_LOGIC<0x3,   "max.0", le1_maxs, IIAlu, 1>;
defm MAXU     : ARITH_LOGIC<0x4,   "maxu.0", le1_maxu, IIAlu, 1>;
defm MINS     : ARITH_LOGIC<0x5,   "min.0", le1_mins, IIAlu, 1>;
defm MINU     : ARITH_LOGIC<0x6,   "minu.0", le1_minu, IIAlu, 1>;
defm OR       : ARITH_LOGIC<0x7,   "or.0",  or, IIAlu, 1>;
defm ORC      : ARITH_LOGIC<0x8,   "orc.0", le1_orc, IIAlu>;
defm SH1ADD   : ARITH_LOGIC<0x9,   "sh1add.0", le1_sh1add, IIAlu>;
defm SH2ADD   : ARITH_LOGIC<0xA,   "sh2add.0", le1_sh2add, IIAlu>;
defm SH3ADD   : ARITH_LOGIC<0xB,   "sh3add.0", le1_sh3add, IIAlu>;
defm SH4ADD   : ARITH_LOGIC<0xC,   "sh4add.0", le1_sh4add, IIAlu>;
defm SHL      : ARITH_LOGIC<0xD,   "shl.0", shl, IIAlu>;
defm SHR      : ARITH_LOGIC<0xE,   "shr.0", sra, IIAlu>;
defm SHRU     : ARITH_LOGIC<0xF,   "shru.0", srl, IIAlu>;
defm SUB      : ARITH_LOGIC<0x10,  "sub.0", sub, IIAlu>;
defm XOR      : ARITH_LOGIC<0x11,  "xor.0", xor, IIAlu, 1>;

// TODO Need to sort out commutable instructions to avoid situations like this:
//  mov.0 r0.9, 0x4
//  ;
//  add.0 r0.10, r0.9, r0.16

/*
multiclass CommutePats<Instruction InstI9, Instruction InstI32, SDNode OpNode> {
  def : Pat<(OpNode imm9:$src0, CPURegs:$src1),
            (InstI9 CPURegs:$src1, imm9:$src0)>;
  def : Pat<(OpNode imm32:$src0, CPURegs:$src1),
            (InstI32 CPURegs:$src1, imm32:$src0)>;
}

defm : CommutePats<ADDi9, ADDi32, add>;
defm : CommutePats<ANDi9, ANDi32, and>;
defm : CommutePats<ORi9, ORi32, or>;
defm : CommutePats<MAXUi9, MAXUi32, le1_maxu>;
defm : CommutePats<MAXSi9, MAXSi32, le1_maxs>;
defm : CommutePats<MINUi9, MINUi32, le1_minu>;
defm : CommutePats<MINSi9, MINSi32, le1_mins>;
defm : CommutePats<XORi9, XORi32, xor>;
*/

// TODO Some of these are commutable
multiclass MULT<bits<5> opcode, string instr_asm, PatFrag OpNode,
                bit isComm = 0> {
  def NAME#r    : Mult_R_RR<opcode, instr_asm, OpNode, isComm>;

  def NAME#i32  : Mult_R_I32R<opcode, instr_asm, OpNode>;

  def NAME#i9   : Mult_R_I9R<opcode, instr_asm, OpNode>;
}

def shl16   : PatFrag<(ops node:$in),
                      (shl node:$in, (i32 16))>;
def srl16   : PatFrag<(ops node:$in),
                      (srl node:$in, (i32 16))>;
def sra16   : PatFrag<(ops node:$in),
                      (sra node:$in, (i32 16))>;
def maskU16 : PatFrag<(ops node:$in),
                      (srl (shl node:$in, (i32 16)), (i32 16))>;
def maskS16 : PatFrag<(ops node:$in),
                      (sra (shl node:$in, (i32 16)), (i32 16))>;

// i16(s1) * i16(s2)
def mulll   : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul (maskS16 node:$lhs), (maskS16 node:$rhs))>;
// ui16(s1) * ui16(s2)
def mulllu  : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul (maskU16 node:$lhs), (maskU16 node:$rhs))>;
// i16(s1) * i16(s2 >> 16)
def mullh   : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul (maskU16 node:$lhs), (srl16 node:$rhs))>;
// ui16(s1) * ui16(s2 >> 16)
def mullhu  : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul (maskU16 node:$lhs), (srl16 node:$rhs))>;
// ui16(s1 >> 16) * ui16(s2 >> 16)
def mulhhu  : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul (srl16 node:$lhs), (srl16 node:$rhs))>;
// s1 * i16(s2)
def mull    : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul node:$lhs, (maskU16 node:$rhs))>;
// s1 * ui16(s2)
def mullu   : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul node:$lhs, (srl16 node:$rhs))>;
// s1 * i16(s2 >> 16)
def mulh    : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul node:$lhs, (sra16 node:$rhs))>;
// s1 * ui16(s2 >> 16)
def mulhu_  : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul node:$lhs, (srl16 node:$rhs))>;
// i16(s1 >> 16) * i16(s2 >> 16)
def mulhh   : PatFrag<(ops node:$lhs, node:$rhs),
                      (mul (sra16 node:$lhs), (sra16 node:$rhs))>;
// s1 * ui16(s2 >> 16) << 16
def mulhs_  : PatFrag<(ops node:$lhs, node:$rhs),
                      (shl16 (mul node:$lhs, (srl16 node:$rhs)))>;

defm MULLL    : MULT<0x12,  "mulll.0", mulll>;
defm MULLLU   : MULT<0x13,  "mulllu.0", mulllu>;
defm MULLH    : MULT<0x14,  "mullh.0", mullh>;
defm MULLHU   : MULT<0x15,  "mullhu.0", mullhu>;
defm MULHH    : MULT<0x16,  "mulhh.0", mulhh>;
defm MULHHU   : MULT<0x17,  "mulhhu.0", mulhhu>;
defm MULL     : MULT<0x18,  "mull.0", mull>;
defm MULLU    : MULT<0x19,  "mullu.0", mullu>;
defm MULH     : MULT<0x1A,  "mulh.0", mulh>;
defm MULHU    : MULT<0x1B,  "mulhu.0", mulhu_>;
defm MULHS    : MULT<0x1C,  "mulhs.0", mulhs_>;

/*
// TODO Some of these are commutable
multiclass MULTS<bits<5> opcode, string instr_asm, SDNode OpNode,
                       InstrItinClass II, bit isComm = 0> {
  def NAME#r    : ArithLogic_R_RR<opcode, instr_asm, OpNode, WriteM, isComm>;

  def NAME#i32  : ArithLogic_R_I32R<opcode, instr_asm, OpNode, simm32,
                                    Op_simm32, WriteMI, isComm>;

  def NAME#i9   : ArithLogic_R_I9R<opcode, instr_asm, OpNode, simm9, Op_simm9,
                                   WriteM, isComm>;
}
multiclass MULTU<bits<5> opcode, string instr_asm, SDNode OpNode,
                       InstrItinClass II, bit isComm = 0> {
  def NAME#r    : ArithLogic_R_RR<opcode, instr_asm, OpNode, WriteM, isComm>;

  def NAME#i32  : ArithLogic_R_I32R<opcode, instr_asm, OpNode, uimm32,
                                    Op_uimm32, WriteMI, isComm>;

  def NAME#i9   : ArithLogic_R_I9R<opcode, instr_asm, OpNode, uimm9, Op_uimm9,
                                   WriteM, isComm>;
}

// i16(s1) * i16(s2)
defm MULLL    : MULTS<0x12,  "mulll.0", le1_mulll, IIMul>;
// ui16(s1) * ui16(s2)
defm MULLLU   : MULTU<0x13,  "mulllu.0", le1_mulllu, IIMul>;
// i16(s1) * i16(s2 >> 16)
defm MULLH    : MULTS<0x14,  "mullh.0", le1_mullh, IIMul>;
// ui16(s1) * ui16(s2 >> 16)
defm MULLHU   : MULTU<0x15,  "mullhu.0", le1_mullhu, IIMul>;
// i16(s1 >> 16) * i16(s2 >> 16)
defm MULHH    : MULTS<0x16,  "mulhh.0", le1_mulhh, IIMul>;
// ui16(s1 >> 16) * ui16(s2 >> 16)
//defm MULHHU   : MULTU<0x17,  "mulhhu.0", le1_mulhhu, IIMul>;
// s1 * i16(s2)
defm MULL     : MULTS<0x18,  "mull.0", le1_mull, IIMul>;
// s1 * ui16(s2)
defm MULLU    : MULTU<0x19,  "mullu.0", le1_mullu, IIMul>;
// s1 * i16(s2 >> 16)
defm MULH     : MULTS<0x1A,  "mulh.0", le1_mulh, IIMul>;
// s1 * ui16(s2 >> 16)
defm MULHU    : MULTU<0x1B,  "mulhu.0", le1_mulhu, IIMul>;
// s1 * ui16(s2 >> 16) << 16
defm MULHS    : MULTU<0x1C,  "mulhs.0", le1_mulhs, IIMul>;
*/

multiclass COMPARE_U<bits<4> opc, string instr_asm, SDNode OpNode> {
  // Results into BRegs
  def NAME#p    : LogicCompare_P_RR<opc, instr_asm, OpNode>;
  def NAME#pi9  : LogicCompare_P_I9R<opc, instr_asm, OpNode, Op_uimm9, uimm9>;
  def NAME#pi32 : LogicCompare_P_I32R<opc, instr_asm, OpNode, Op_uimm32,
                                      uimm32>;
  // Results into CPURegs
  def NAME#r    : LogicCompare_R_RR<opc, instr_asm, OpNode>;
  def NAME#i9   : LogicCompare_R_I9R<opc, instr_asm, OpNode, Op_uimm9, uimm9>;
  def NAME#i32  : LogicCompare_R_I32R<opc, instr_asm, OpNode, Op_uimm32,
                                      uimm32>;
}

multiclass COMPARE_S<bits<4> opc, string instr_asm, SDNode OpNode> {
  // Results into BRegs
  def NAME#p    : LogicCompare_P_RR<opc, instr_asm, OpNode>;
  def NAME#pi9  : LogicCompare_P_I9R<opc, instr_asm, OpNode, Op_simm9, simm9>;
  def NAME#pi32 : LogicCompare_P_I32R<opc, instr_asm, OpNode, Op_simm32,
                                      simm32>;
  // Results into CPURegs
  def NAME#r    : LogicCompare_R_RR<opc, instr_asm, OpNode>;
  def NAME#i9   : LogicCompare_R_I9R<opc, instr_asm, OpNode, Op_simm9, simm9>;
  def NAME#i32  : LogicCompare_R_I32R<opc, instr_asm, OpNode, Op_simm32,
                                      simm32>;
}

multiclass LOGIC<bits<4> opc, string instr_asm, SDNode OpNode> {
  // Results into CPURegs
  def NAME#r    : LogicCompare_R_RR<opc, instr_asm, OpNode>;
  def NAME#i9   : LogicCompare_R_I9R<opc, instr_asm, OpNode, Op_uimm9, uimm9>;
  def NAME#i32  : LogicCompare_R_I32R<opc, instr_asm, OpNode, Op_uimm32,
                                      uimm32>;
}

let isCompare = 1 in {
  defm CMPEQ  : COMPARE_S<0x0, "cmpeq.0", le1_cmpeq>;
  defm CMPGE  : COMPARE_S<0x1, "cmpge.0", le1_cmpge>;
  defm CMPGEU : COMPARE_U<0x2, "cmpgeu.0", le1_cmpgeu>;
  defm CMPGT  : COMPARE_S<0x3, "cmpgt.0", le1_cmpgt>;
  defm CMPGTU : COMPARE_U<0x4, "cmpgtu.0", le1_cmpgtu>;
  defm CMPLE  : COMPARE_S<0x5, "cmple.0", le1_cmple>;
  defm CMPLEU : COMPARE_U<0x6, "cmpleu.0", le1_cmpleu>;
  defm CMPLT  : COMPARE_S<0x7, "cmplt.0", le1_cmplt>;
  defm CMPLTU : COMPARE_U<0x8, "cmpltu.0", le1_cmpltu>;
  defm CMPNE  : COMPARE_S<0x9, "cmpne.0", le1_cmpne>;
}

defm NANDL  : LOGIC<0xA, "nandl.0", le1_nandl>;
defm NORL   : LOGIC<0xB, "norl.0", le1_norl>;
defm ORL    : LOGIC<0xC, "orl.0", le1_orl>;
defm ANDL   : LOGIC<0xD, "andl.0", le1_andl>;

//def : Pat<(zext (le1_norl CPURegs:$src1, CPURegs:$src2)),
  //        (NORLr CPURegs:$src1, CPURegs:$src2)>;

def SXTB  : Extend<0x0, "sxtb.0", le1_sxtb, i8>;
def SXTH  : Extend<0x1, "sxth.0", le1_sxth, i16>;
def ZXTB  : Extend<0x2, "zxtb.0", le1_zxtb, i8>;
def ZXTH  : Extend<0x3, "zxth.0", le1_zxth, i16>;


/*
multiclass LOADS<bits<4> opc, string instr_asm, SDNode OpNode> {
  def NAME#i32  : LoadMem_I32<opc, instr_asm, OpNode>;
  def NAME#i12  : LoadMem_I12<opc, instr_asm, OpNode>;
  def NAME#i8   : LoadMem_I8<opc, instr_asm, OpNode>;
}
*/

def LDSBi32   : LoadMem_I32<0x3, "ldb.0",  sextloadi8>;
def LDUBi32   : LoadMem_I32<0x7, "ldbu.0", zextloadi8>;
def LDSHi32   : LoadMem_I32<0xB, "ldh.0",  sextloadi16>;
def LDUHi32   : LoadMem_I32<0xF, "ldhu.0", zextloadi16>;
def LDWi32    : LoadMem_I32<0x13, "ldw.0",  load>;
def LDSBi12   : LoadMem_I12<0x0, 10, "ldb.0",  sextloadi8>;
def LDUBi12   : LoadMem_I12<0x0, 11, "ldbu.0", zextloadi8>;
def LDSHi12   : LoadMem_I12<0x0, 12, "ldh.0",  sextloadi16>;
def LDUHi12   : LoadMem_I12<0x0, 13, "ldhu.0", zextloadi16>;
def LDWi12    : LoadMem_I12<0x0, 14, "ldw.0",  load>;
def LDUBi8    : LoadMem_I8<0x0, "ldbu.0", zextloadi8>;
def LDSBi8    : LoadMem_I8<0x1,"ldb.0",  sextloadi8>;
def LDUHi8    : LoadMem_I8<0x2, "ldhu.0", zextloadi16>;
def LDSHi8    : LoadMem_I8<0x3, "ldh.0",  sextloadi16>;
def LDWi8     : LoadMem_I8<0x4, "ldw.0",  load>;


def STBi32    : StoreMem_I32<0x0, "stb.0", truncstorei8>;
def STHi32    : StoreMem_I32<0x1, "sth.0", truncstorei16>;
def STWi32    : StoreMem_I32<0x2, "stw.0", store>;
def STBi12    : StoreMem_I12<0x0, "stb.0", truncstorei8>;
def STHi12    : StoreMem_I12<0x1, "sth.0", truncstorei16>;
def STWi12    : StoreMem_I12<0x2, "stw.0", store>;
def STBi8     : StoreMem_I8<0x20, "stb.0", truncstorei8>;
def STHi8     : StoreMem_I8<0x21, "sth.0", truncstorei16>;
def STWi8     : StoreMem_I8<0x22, "stw.0", store>;


def ADDCG : LE1Format_RP_PRR<
            0x0,
            (outs CPURegs:$rd, BRegs:$prd2),
            (ins CPURegs:$src1, CPURegs:$src2, BRegs:$prs),
            "addcg.0  $rd, $prd2, $src1, $src2, $prs",
            [], IIAlu>, Sched<[WriteA, WriteP]>;

def DIVS  : LE1Format_RP_PRR<
            0x1,
            (outs CPURegs:$rd, BRegs:$prd2),
            (ins CPURegs:$src1, CPURegs:$src2, BRegs:$prs),
            "divs.0  $rd, $prd2, $src1, $src2, $prs",
            [], IIAlu>, Sched<[WriteA, WriteP]>;

def MFB   : LE1Format_RP_PRR<
            0x2,
            (outs CPURegs:$rd),
            (ins BRegs:$prs),
            "mfb.0  $rd, $prs",
            [(set CPURegs:$rd, (le1_mfb BRegs:$prs))], IIAlu>,
            Sched<[WriteA]> {
  let prd2 = 0;
  let src1 = 0;
  let src2 = 0;
}

def SLCTi   : Select_I32<0x1C, "slct.0", select>;
def SLCTFi  : Select_I32<0x1D, "slctf.0", le1_selectf>;
def SLCTr   : Select_R<0x1C, "slct.0", select>;
def SLCTFr  : Select_R<0x1D, "slctf.0", le1_selectf>;


// ------------------------- END OF NEW IMPLEMENTATION ---------------------- //


///===---------------------------------------------------------------------===//
// Move Instructions
//def MFB   : MoveR<"mfb.0",  le1_mfb,  CPURegs,  BRegs>;
def MTB   : MoveR<"mtb.0",  le1_mtb,  BRegs,    CPURegs>;
def MTBF  : MoveR<"mtbf.0", le1_mtbf, BRegs,    CPURegs>;
let isMoveImm =1 in
def MOVi32 :
  LE1Inst<(outs CPURegs:$dst),
          (ins globaladdress:$src),
          "mov.0  $dst, $src", [], IIAlu>,
          Sched<[WriteAI]>;

def PadInst : LE1Format_R_I9R <
  0x0,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, Op_uimm9:$imm9),
  "add.0, $rd, $src1, $imm9",
  [], IIAlu>, Sched<[WriteA]>;

//def NOP : LE1Pseudo<(outs), (ins), "nop", []>;

//def MOVr  : MoveR<"mov.0",  le1_mov,  CPURegs,  CPURegs>;
//def MOVi  : MoveI<"mov.0", le1_mov, CPURegs, i32imm, imm>;
//def MOVg  : MoveI<"mov.0",  le1_mov, CPURegs, globaladdress, tglobaladdr>;
//def MFL   : MoveR<"mfl.0",  le1_mfl,  CPURegs,  LReg>;
//def MTL   : MoveR<"mtl.0",  le1_mtl,  LReg,     CPURegs>;
//def MTLg  : MoveI<"mov.0", le1_mtl, LReg, globaladdress, tglobaladdr>;

//===----------------------------------------------------------------------===//
// Control-Flow Instructions
def BR     : CBranch<"br.0", le1_br>;
def BRF    : CBranch<"brf.0", le1_brf>;

// Unconditional branch
let isBranch=1, isTerminator=1, isBarrier=1 in
def GOTO :  LE1Inst<(outs), (ins brtarget:$target),
            "goto.0  $target",
            [(le1_goto bb:$target)], IIBranch>, Sched<[WriteB]>;

// Call
let isCall=1, isBarrier=1,
  // All calls clobber the non-callee saved registers...
  Defs = [B0, B1, B2, B3, B4, B5, B6, B7,
  STRP, AR0, AR1, AR2, AR3, AR4, AR5, AR6, AR7, T0, T1, T2, T3, T4, T5,
  T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21,
  T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32, T33, T34, T35, T36,
  T37, T38, T39, T40, T41, T42, T43, T44, T45], Uses = [LNK] in {

  def CALL :  LE1Inst<(outs),
              (ins CPURegs:$lnk, calltarget:$target, variable_ops),
              "call.0  $lnk, $target",
              [(le1_call CPURegs:$lnk, imm:$target)], IIBranch>, Sched<[WriteB]>;

}

//let Constraints = "$src = $dst",
let Uses = [LNK, SP],// in
Defs = [SP],
isReturn=1, isTerminator=1, isBarrier=1, hasCtrlDep=1 in
//def Ret : LE1Inst<(outs CPURegs:$dst),
def Ret : LE1Inst<(outs),
          (ins CPURegs:$src, i32imm:$frame, CPURegs:$lnk),
          "return.0  $src, $src, $frame, $lnk",
          [(le1_ret CPURegs:$src, imm:$frame, CPURegs:$lnk)],
          IIBranch>, Sched<[WriteB]>;



//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

// As stack alignment is always done with addiu, we need a 16-bit immediate
let Defs = [SP], Uses = [SP], isPseudo=1 in {
def ADJCALLSTACKDOWN : LE1Pseudo<(outs), (ins Op_uimm32:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : LE1Pseudo<(outs), (ins Op_uimm32:$amt1, Op_uimm32:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// Operations to load/store predicate register
let Defs = [T45] in {
def LDW_PRED  : LE1Pseudo<(outs BRegs:$dst), (ins mem:$addr),
                          "!LOADPRED $dst, $addr",
                          [(set (i1 BRegs:$dst), (load addr_i32:$addr))]>;
def STW_PRED  : LE1Pseudo<(outs), (ins BRegs:$src, mem:$addr),
                          "!STOREPRED $src, $addr",
                          [(store (i1 BRegs:$src), addr_i32:$addr)]>;
}

// Operation to handle function pointer
let Defs = [LNK], Uses = [LNK], isPseudo=1 in {
  def CALLPOINTER : LE1Pseudo<(outs), (ins CPURegs:$lnk, globaladdress:$src),
                              "!LINKCALL", []>;
}

def CLK : LE1Pseudo < (outs), (ins), ";;", []>;

// Instruction created to exit main
let isReturn=1, isTerminator=1, isBarrier=1, Uses = [LNK] in
def Exit : LE1Inst< (outs), (ins CPURegs:$lnk),
          "call.0  $lnk, exit", [], IIBranch>, Sched<[WriteB]>;

let isReturn=1, isTerminator=1, isBarrier=1, hasCtrlDep=1 in
def RetFlag : LE1Pseudo < (outs), (ins), ";", [(le1_retflag)]>;


include "LE1IntrinsicInstrInfo.td"
include "LE1InstrPatterns.td"
