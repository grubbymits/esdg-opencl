def s32imm  : Operand<i32>;
def u32imm  : Operand<i32>;

class Mult_RRR<string instr_asm, bits<5> opcode, SDNode OpNode>
  : LE1Format0SubFMT<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, CPURegs:$src2),
  !strconcat(instr_asm, "  $rd, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, CPURegs:$src2))],
  IIMul> {
  let subfmt = 1;
}

class Mult_RRI<string instr_asm, bits<5> opcode, SDNode OpNode>
  : LE1Format1SubFMT<
  opcode,
  (outs CPURegs:$rd),
  (ins CPURegs:$src1, i32imm:$src2),
  !strconcat(instr_asm, "  $rd, $src1, $src2"),
  [(set CPURegs:$rd, (OpNode CPURegs:$src1, (i32 imm:$src2)))],
  IIMul> {
  let subfmt = 1;
}

def MULTL     : Mult_RRR<"multl.0", 0x18, mul>;
def MULTHU    : Mult_RRR<"multhu.0", 0x19, mulhu>;
def MULTHS    : Mult_RRR<"mulths.0", 0x1b, mulhs>;
def MULTLi    : Mult_RRI<"multl.i.0", 0x18, mul>;
def MULTHUi   : Mult_RRI<"multhu.i.0", 0x19, mulhu>;
def MULTHSi   : Mult_RRI<"mulths.i.0", 0x1b, mulhs>;
