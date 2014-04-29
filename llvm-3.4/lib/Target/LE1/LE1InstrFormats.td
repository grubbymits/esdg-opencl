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
  //let Itinerary   = itin;
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
  //let Itinerary   = itin;

  let Inst{31} = is;
  let Inst{30} = cs;
  let Inst{29-26} = fmt;
}

// Format 0
class LE1Format_R_RR<bits<5>opcode, dag outs, dag ins, string asmstr,
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

class LE1Format_R_R<bits<5>opcode, dag outs, dag ins, string asmstr,
                    list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> src1;

  let fmt = 1;
  let Inst{25-21} = opcode;
  let Inst{20-15} = rd;
  let Inst{10-6} = 1;
  let Inst{5-0} = src1;
}

class LE1Format_LD_I32<bits<5>opcode, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> src1;

  let fmt = 1;
  let Inst{25-21} = opcode;
  let Inst{20-15} = rd;
  let Inst{10-6} = 2;
  let Inst{5-0} = src1;
}

class LE1Format_ST_I32<bits<5>opcode, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> src2;
  bits<6> src1;

  let fmt = 1;
  let Inst{25-21} = opcode;
  let Inst{20-15} = src2;
  let Inst{14-9} = 0;
  let Inst{8-6} = 3;
  let Inst{5-0} = src1;
}

class LE1Format_RP_PRR<bits<2>opc2, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<3> prd2;
  bits<6> rd;
  bits<3> prs;
  bits<6> src2;
  bits<6> src1;

  let fmt = 3;
  let Inst{25-24} = opc2;
  let Inst{23-21} = prd2;
  let Inst{20-15} = rd;
  let Inst{14-12} = prs;
  let Inst{11-6} = src2;
  let Inst{5-0} = src1;
}

class LE1Format_ST_RRI<bits<2>opcode, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> src2;
  bits<6> src1;

  let fmt = 7;
  let Inst{25-24} = opcode;
  let Inst{11-6} = src2;
  let Inst{5-0} = src1;
}

class LE1Format_ST_S<bits<2>opcode, bits<4> format, dag outs, dag ins,
                     string asmstr, list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  //bits<12> imm12;
  bits<6> rd;
  bits<6> src1;

  let fmt = format;
  let Inst{25-24} = opcode;
  //let Inst{23-12} = imm12;
  let Inst{11-6} = rd;
  let Inst{5-0} = src1;
}

class LE1Format_LDST_I8<bits<6> opcode, dag outs, dag ins, string asmstr,
                        list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  //bits<8> imm8;
  bits<6> rd;
  bits<6> src1;

  let fmt = 15;
  let Inst{25-20} = opcode;
  //let Inst{19-12} = imm8;
  let Inst{11-6} = rd;
  let Inst{5-0} = src1;
}

class LE1Format_R_I32R<bits<5> opcode, dag outs, dag ins, string asmstr,
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

class LE1Format_R_I32_RP<bits<5> opc5, dag outs, dag ins, string asmstr,
                         list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<3> prs;
  bits<6> src1;
  bits<32> src2;

  let fmt = 2;
  let Inst{25-21} = opc5;
  let Inst{20-15} = rd;
  let Inst{14-12} = prs;
  let Inst{5-0} = src1;
}

class LE1Format_R_I9R<bits<5> opc5, dag outs, dag ins, string asmstr,
                 list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> src1;
  bits<9> imm9;

  let fmt = 2;
  let Inst{25-21} = opc5;
  let Inst{20-15} = rd;
  let Inst{14-6} = imm9;
  let Inst{5-0} = src1;
}

class LE1Format_R_RR_1<bits<4> opc4, dag outs, dag ins, string asmstr,
                       list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> src1;
  bits<6> src2;

  let fmt = 4;
  let Inst{25} = 0;
  let Inst{24-21} = opc4;
  let Inst{20-15} = rd;
  let Inst{14-12} = 0;
  let Inst{11-6} = src2;
  let Inst{5-0} = src1;
}

class LE1Format_P_RR<bits<4> opc4, dag outs, dag ins, string asmstr,
                     list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<3> prd;
  bits<6> src1;
  bits<6> src2;

  let fmt = 4;
  let Inst{25} = 1; // B
  let Inst{24-21} = opc4;
  let Inst{20-18} = 0;
  let Inst{17-15} = prd;
  let Inst{14-12} = 0;
  let Inst{11-6} = src2;
  let Inst{5-0} = src1;
}

class LE1Format_R_RR_PT<bits<5> opc5, dag outs, dag ins, string asmstr,
                        list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<3> prs;
  bits<6> rd;
  bits<6> src1;
  bits<6> src2;

  let fmt = 9;
  let Inst{25-21} = opc5;
  let Inst{20-15} = rd;
  let Inst{14-12} = prs;
  let Inst{11-6} = src2;
  let Inst{5-0} = src1;
}

class LE1Format_RoB<bits<4> opc4, dag outs, dag ins, string asmstr,
                    list<dag> pattern, InstrItinClass itin>
  : LE1FormatBase<outs, ins, asmstr, pattern, itin> {

  bits<6> rd;
  bits<6> src1;
  bits<1> B;
  let Inst{25} = B;
  let Inst{24-21} = opc4;
  let Inst{20-15} = rd;
  let Inst{5-0} = src1;
}

class LE1Format_RoB_I32R<bits<4> opc4, dag outs, dag ins, string asmstr,
                         list<dag> pattern, InstrItinClass itin>
  : LE1Format_RoB<opc4, outs, ins, asmstr, pattern, itin> {

  bits<32> src2;

  let fmt = 5;
  let Inst{25} = 0;
  let Inst{14-6} = 0;
}

class LE1Format_RoB_I9R<bits<4> opc4, dag outs, dag ins, string asmstr,
                        list<dag> pattern, InstrItinClass itin>
  : LE1Format_RoB<opc4, outs, ins, asmstr, pattern, itin> {

  bits<9> imm9;

  let fmt = 6;
  let Inst{25} = 0;
  let Inst{14-6} = imm9;
}
