def sh1add  : PatFrag<(ops node:$lhs, node:$rhs),
                      (add (shl node:$lhs, (i32 1)), node:$rhs)>;
def sh2add  : PatFrag<(ops node:$lhs, node:$rhs),
                      (add (shl node:$lhs, (i32 2)), node:$rhs)>;
def sh3add  : PatFrag<(ops node:$lhs, node:$rhs),
                      (add (shl node:$lhs, (i32 3)), node:$rhs)>;
def sh4add  : PatFrag<(ops node:$lhs, node:$rhs),
                      (add (shl node:$lhs, (i32 4)), node:$rhs)>;

multiclass SHADD_PATS<Instruction InstR, Instruction InstI9,
                      Instruction InstI32, PatFrag Op> {
  def : Pat<(Op CPURegs:$src0, CPURegs:$src1),
            (InstR CPURegs:$src0, CPURegs:$src1)>;
  def : Pat<(Op CPURegs:$src0, simm9:$src1),
            (InstI9 CPURegs:$src0, simm9:$src1)>;
  def : Pat<(Op CPURegs:$src0, simm32:$src1),
            (InstI32 CPURegs:$src0, simm32:$src1)>;
}

defm : SHADD_PATS<SH1ADDr, SH1ADDi9, SH1ADDi32, sh1add>;
defm : SHADD_PATS<SH2ADDr, SH2ADDi9, SH2ADDi32, sh2add>;
defm : SHADD_PATS<SH3ADDr, SH3ADDi9, SH4ADDi32, sh3add>;
defm : SHADD_PATS<SH4ADDr, SH4ADDi9, SH4ADDi32, sh4add>;

/*
// TODO Must be way of using the specific shift value in the variable pattern.
multiclass SH1ADD_PATS<Instruction InstR, Instruction InstI9,
                      Instruction InstI32> {
  def : Pat<(sh1add CPURegs:$src0, CPURegs:$src1),
            (InstR CPURegs:$src0, CPURegs:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 1)), simm9:$src1),
            (InstI9 CPURegs:$src0, simm9:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 1)), simm32:$src1),
            (InstI32 CPURegs:$src0, simm32:$src1)>;
}

multiclass SH2ADD_PATS<Instruction InstR, Instruction InstI9,
                      Instruction InstI32> {
  def : Pat<(add (shl CPURegs:$src0, (i32 2)), CPURegs:$src1),
            (InstR CPURegs:$src0, CPURegs:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 2)), simm9:$src1),
            (InstI9 CPURegs:$src0, simm9:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 2)), simm32:$src1),
            (InstI32 CPURegs:$src0, simm32:$src1)>;
}

multiclass SH3ADD_PATS<Instruction InstR, Instruction InstI9,
                      Instruction InstI32> {
  def : Pat<(add (shl CPURegs:$src0, (i32 3)), CPURegs:$src1),
            (InstR CPURegs:$src0, CPURegs:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 3)), simm9:$src1),
            (InstI9 CPURegs:$src0, simm9:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 3)), simm32:$src1),
            (InstI32 CPURegs:$src0, simm32:$src1)>;
}

multiclass SH4ADD_PATS<Instruction InstR, Instruction InstI9,
                      Instruction InstI32> {
  def : Pat<(add (shl CPURegs:$src0, (i32 4)), CPURegs:$src1),
            (InstR CPURegs:$src0, CPURegs:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 4)), simm9:$src1),
            (InstI9 CPURegs:$src0, simm9:$src1)>;
  def : Pat<(add (shl CPURegs:$src0, (i32 4)), simm32:$src1),
            (InstI32 CPURegs:$src0, simm32:$src1)>;
}

defm : SH1ADD_PATS<SH1ADDr, SH1ADDi9, SH1ADDi32>;
defm : SH2ADD_PATS<SH2ADDr, SH2ADDi9, SH2ADDi32>;
defm : SH3ADD_PATS<SH3ADDr, SH3ADDi9, SH3ADDi32>;
defm : SH4ADD_PATS<SH4ADDr, SH4ADDi9, SH4ADDi32>;
*/
// Multiplication Patterns
def : Pat<(mul CPURegs:$src1, CPURegs:$src2),
          (ADDr (MULLUr CPURegs:$src1, CPURegs:$src2),
                (MULHSr CPURegs:$src1, CPURegs:$src2))>;
def : Pat<(mul CPURegs:$src1, imm9:$src2),
          (ADDr (MULLUi9 CPURegs:$src1, imm9:$src2),
                (MULHSi9 CPURegs:$src1, imm9:$src2))>;
def : Pat<(mul CPURegs:$src1, imm32:$src2),
          (ADDr (MULLUi32 CPURegs:$src1, imm32:$src2),
                (MULHSi32 CPURegs:$src1, imm32:$src2))>;
/*
//def MULHHU  // ui16(s1 >> 16) * ui16(s2 >> 16)
//def : Pat<(mul (srl CPURegs:$lhs, (i32 16)), (srl CPURegs:$rhs, (i32 16))),
  //        (MULHHUr CPURegs:$lhs, CPURegs:$rhs)>;

//def MULLHU  // ui16(s1) * ui16(s2 >> 16)
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (srl CPURegs:$rhs, (i32 16))),
          (MULLHUr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (and CPURegs:$lhs, (i32 0xffff)), (srl CPURegs:$rhs, (i32 16))),
          (MULLHUr CPURegs:$lhs, CPURegs:$rhs)>;

//def MULHH   // i16(s1 >> 16) * i16(s2 >> 16)
def : Pat<(mul (sra CPURegs:$lhs, (i32 16)), (sra CPURegs:$rhs, (i32 16))),
          (MULHHr CPURegs:$lhs, CPURegs:$rhs)>;
//def : Pat<(mul (sra CPURegs:$lhs, (i32 16)), (sra simm:$rhs, (i32 16))),
  //        (MULHH CPURegs:$lhs, imm:$rhs)>;

//def MULLH   // i16(s1) * i16(s2 >> 16)
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (sra CPURegs:$rhs, (i32 16))),
          (MULLHr CPURegs:$lhs, CPURegs:$rhs)>;
//def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //             (sra imm:$rhs, (i32 16))),
    //      (MULLH CPURegs:$lhs, imm:$rhs)>;

//def MULHS   // s1 * ui16(s2 >> 16) << 16
def : Pat<(shl (mul CPURegs:$lhs, (srl CPURegs:$rhs, (i32 16))), (i32 16)),
          (MULHSr CPURegs:$lhs, CPURegs:$rhs)>;

//def MULH    // s1 * i16(s2 >> 16)
def : Pat<(mul CPURegs:$lhs, (sra CPURegs:$rhs, (i32 16))),
          (MULHr CPURegs:$lhs, CPURegs:$rhs)>;

//def MULHU   // s1 * ui16(s2 >> 16)
def : Pat<(mul CPURegs:$lhs, (srl CPURegs:$rhs, (i32 16))),
          (MULHUr CPURegs:$lhs, CPURegs:$rhs)>;

//def MULLL   // i16(s1) * i16(s2)
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (sra (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLLr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (sext_inreg CPURegs:$lhs, i16), (sext_inreg CPURegs:$rhs, i16)),
          (MULLLr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)), simm9:$rhs),
          (MULLLi32 CPURegs:$lhs, simm9:$rhs)>;
def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)), simm16:$rhs),
          (MULLLi32 CPURegs:$lhs, simm16:$rhs)>;

//def MULLLU  // ui16(s1) * ui16(s2)
def : Pat<(mul (and CPURegs:$lhs, (i32 0xffff)),
               (and CPURegs:$rhs, (i32 0xffff))),
          (MULLLUr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)),
               (srl (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLLUr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)), imm9:$rhs),
          (MULLLUi9 CPURegs:$lhs, imm9:$rhs)>;
def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)), imm16:$rhs),
          (MULLLUi32 CPURegs:$lhs, imm16:$rhs)>;

//def MULLU   // s1 * ui16(s2)
def : Pat<(mul CPURegs:$lhs, (and CPURegs:$rhs, (i32 0xFFFF))),
          (MULLUr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, (srl (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLUr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, imm9:$rhs),
          (MULLUi9 CPURegs:$lhs, imm9:$rhs)>;
def : Pat<(mul CPURegs:$lhs, imm16:$rhs),
          (MULLUi32 CPURegs:$lhs, imm16:$rhs)>;

//def MULL    // s1 * i16(s2)
def : Pat<(mul CPURegs:$lhs, (sra (shl CPURegs:$rhs, (i32 16)), (i32 16))),
          (MULLr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, (sext_inreg CPURegs:$rhs, i16)),
          (MULLr CPURegs:$lhs, CPURegs:$rhs)>;
def : Pat<(mul CPURegs:$lhs, simm9:$rhs),
          (MULLi9 CPURegs:$lhs, simm9:$rhs)>;
def : Pat<(mul CPURegs:$lhs, simm16:$rhs),
          (MULLi32 CPURegs:$lhs, simm16:$rhs)>;
*/

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

def : Pat<(sext_inreg CPURegs:$src, i8),
          (SXTB CPURegs:$src)>;
def : Pat<(sext_inreg CPURegs:$src, i16),
          (SXTH CPURegs:$src)>;

// Patterns to happen extending loads
def : Pat<(extloadi8 addr_i8:$addr),
          (LDUBi8 addr_i8:$addr)>;
def : Pat<(extloadi8 addr_i12:$addr),
          (LDUBi12 addr_i12:$addr)>;
def : Pat<(extloadi16 addr_i8:$addr),
          (LDUHi8 addr_i8:$addr)>;
def : Pat<(extloadi16 addr_i12:$addr),
          (LDUHi12 addr_i12:$addr)>;

// Return pattern to handle the fact that we lower (i32 0) to ZERO
def : Pat<(le1_ret CPURegs:$src, ZERO, CPURegs:$lnk),
          (Ret CPURegs:$src, (i32 0), CPURegs:$lnk)>;

def : Pat<(i32 imm9:$in),
          (ADDi9 ZERO, imm9:$in)>;
def : Pat<(i32 imm32:$in),
          (ADDi32 ZERO, imm32:$in)>;
def : Pat<(i32 simm9:$in),
          (ADDsi9 ZERO, simm9:$in)>;
def : Pat<(i32 simm32:$in),
          (ADDsi32 ZERO, simm32:$in)>;

// Call
def : Pat<(le1_call CPURegs:$lnk, (i32 tglobaladdr:$dst)),
          (CALL CPURegs:$lnk, tglobaladdr:$dst)>;
def : Pat<(le1_call CPURegs:$lnk, (i32 texternalsym:$dst)),
          (CALL CPURegs:$lnk, texternalsym:$dst)>;

// Calls using function pointers
def : Pat<(le1_call CPURegs:$lnk, (i32 (load addr_i32:$src))),
          (CALLPOINTER CPURegs:$lnk, addr_i32:$src)>;

// LE1 does not have "not", so we expand our way
def : Pat<(not CPURegs:$in),
          (ORCr CPURegs:$in, ZERO)>;

def : Pat<(zext (trunc CPURegs:$src)),
          (ORLr CPURegs:$src, ZERO)>;
def : Pat<(anyext (trunc CPURegs:$src)),
          (ORLr CPURegs:$src, ZERO)>;

def : Pat<(le1_call LNK, (le1_targetglobal tglobaladdr:$addr)),
          (CALL LNK, tglobaladdr:$addr)>;
def : Pat<(br bb:$brtarget),
          (GOTO bb:$brtarget)>;

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
