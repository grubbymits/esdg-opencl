//===- LE1InstrFormats.td - LE1 Instruction Formats ------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Describe LE1 instructions format
//
//  CPU INSTRUCTION FORMATS
//
//  opcode  - operation code.
//  rs      - src reg.
//  rt      - dst reg (on a 2 regs instr) or src reg (on a 3 reg instr).
//  rd      - dst reg, only used on 3 regs instr.
//  shamt   - only used on shift instructions, contains the shift amount.
//  funct   - combined with opcode field give us an operation code.
//
//===----------------------------------------------------------------------===//


class LE1Inst<dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin>: Instruction
{
  field bits<32> Inst;

  let Namespace = "LE1";

  bits<6> Opcode = 0;

  // Top 6 bits are the 'opcode' field
  let Inst{31-26} = Opcode;

  let OutOperandList = outs;
  let InOperandList  = ins;

  let AsmString   = asmstr;
  let Pattern     = pattern;
  let Itinerary   = itin;
}

// LE1 Pseudo Instructions Format
class LE1Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>:
      LE1Inst<outs, ins, asmstr, pattern, IIPseudo> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

class LE1FormatBase<dag outs, dag ins, string asmstr, list<dag> pattern,
                    InstrItinClass itin> : Instruction {
  let Namespace = "LE1";
  field bits<32> Inst;
  bits<1> is;
  bits<1> cs;
  bits<4> fmt;

  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString   = asmstr;
  let Pattern     = pattern;
  let Itinerary   = itin;

  let Inst{31} = is;
  let Inst{30} = cs;
  let Inst{29-26} = fmt;
}

// Format 0
class LE1Format0<bits<5>opcode, dag outs, dag ins, string asmstr,
                 list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> src2;
  bits<6> src1;

  let fmt = 0;
  let Inst{25-21} = opcode;
  let Inst{20-15} = rd;
  let Inst{11-6} = src2;
  let Inst{5-0} = src1;
}

class LE1Format0SubFMT<bits<5>opcode, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1Format0<opcode, outs, ins, asmstr, pattern, itin> {

  bits<3> subfmt;
  let Inst{14-12} = subfmt;
}

class LE1Format0PRS<bits<5>opcode, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1Format0<opcode, outs, ins, asmstr, pattern, itin> {

  bits<3> prs;
  let Inst{14-12} = prs;
}

// Format 1
class LE1Format1<bits<5> opcode, dag outs, dag ins, string asmstr,
                 list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> subfmt2;
  bits<6> src1;
  bits<32> src2;

  let fmt = 1;
  let subfmt2 = 0;

  let Inst{25-21} = opcode;
  let Inst{20-15} = rd;
  let Inst{11-6} = subfmt2;
  let Inst{5-0} = src1;
}

class LE1Format1SubFMT<bits<5>opcode, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1Format1<opcode, outs, ins, asmstr, pattern, itin> {

  bits<3> subfmt;
  let Inst{14-12} = subfmt;
}

class LE1Format1PRS<bits<5>opcode, dag outs, dag ins, string asmstr,
                    list<dag> pattern, InstrItinClass itin>
  : LE1Format1<opcode, outs, ins, asmstr, pattern, itin> {

  bits<3> prs;
  let Inst{14-12} = prs;
}
