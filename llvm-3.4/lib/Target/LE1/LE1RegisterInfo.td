//===- LE1RegisterInfo.td - LE1 Register defs ----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the LE1 register file
//===----------------------------------------------------------------------===//

// Base class for GPR and aliased FPR
class LE1Reg<string n> : Register<n> {
  field bits<6> Num;
  let Namespace = "LE1";
}

// LE1 General Purpose Registers
class GPR<bits<6> num, string n> : LE1Reg<n> {
  let Num = num;
}

// LE1 1-bit Branch Registers
class BRR<bits<6> num, string n> : LE1Reg<n> {
  field bits<6> Num;
  let Namespace = "LE1";
}

//===---------------------------------------------------------------------===//
//  Registers
//===---------------------------------------------------------------------===//

let Namespace = "LE1" in {

  // General Purpose Registers
  def ZERO  : GPR<0, "r0.0">,  DwarfRegNum<[0]>;
  def SP    : GPR<1, "r0.1">,  DwarfRegNum<[1]>;
  def STRP  : GPR<2, "r0.2">,  DwarfRegNum<[2]>;
  def AR0    : GPR<3, "r0.3">,  DwarfRegNum<[3]>;
  def AR1   : GPR<4, "r0.4">,  DwarfRegNum<[4]>;
  def AR2   : GPR<5, "r0.5">,  DwarfRegNum<[5]>;
  def AR3   : GPR<6, "r0.6">,  DwarfRegNum<[6]>;
  def AR4   : GPR<7, "r0.7">,  DwarfRegNum<[7]>;
  def AR5   : GPR<8, "r0.8">,  DwarfRegNum<[8]>;
  def AR6   : GPR<9, "r0.9">,  DwarfRegNum<[9]>;
  def AR7   : GPR<10, "r0.10">, DwarfRegNum<[10]>;
  def T0    : GPR<11, "r0.11">, DwarfRegNum<[11]>;
  def T1   : GPR<12, "r0.12">, DwarfRegNum<[12]>;
  def T2   : GPR<13, "r0.13">, DwarfRegNum<[13]>;
  def T3   : GPR<14, "r0.14">, DwarfRegNum<[14]>;
  def T4   : GPR<15, "r0.15">, DwarfRegNum<[15]>;
  def T5   : GPR<16, "r0.16">, DwarfRegNum<[16]>;
  def T6   : GPR<17, "r0.17">, DwarfRegNum<[17]>;
  def T7   : GPR<18, "r0.18">, DwarfRegNum<[18]>;
  def T8   : GPR<19, "r0.19">, DwarfRegNum<[19]>;
  def T9   : GPR<20, "r0.20">, DwarfRegNum<[20]>;
  def T10   : GPR<21, "r0.21">, DwarfRegNum<[21]>;
  def T11   : GPR<22, "r0.22">, DwarfRegNum<[22]>;
  def T12   : GPR<23, "r0.23">, DwarfRegNum<[23]>;
  def T13   : GPR<24, "r0.24">, DwarfRegNum<[24]>;
  def T14   : GPR<25, "r0.25">, DwarfRegNum<[25]>;
  def T15   : GPR<26, "r0.26">, DwarfRegNum<[26]>;
  def T16   : GPR<27, "r0.27">, DwarfRegNum<[27]>;
  def T17   : GPR<28, "r0.28">, DwarfRegNum<[28]>;
  def T18   : GPR<29, "r0.29">, DwarfRegNum<[29]>;
  def T19   : GPR<30, "r0.30">, DwarfRegNum<[30]>;
  def T20   : GPR<31, "r0.31">, DwarfRegNum<[31]>;
  def T21   : GPR<32, "r0.32">, DwarfRegNum<[32]>;
  def T22   : GPR<33, "r0.33">, DwarfRegNum<[33]>;
  def T23   : GPR<34, "r0.34">, DwarfRegNum<[34]>;
  def T24   : GPR<35, "r0.35">, DwarfRegNum<[35]>;
  def T25   : GPR<36, "r0.36">, DwarfRegNum<[36]>;
  def T26   : GPR<37, "r0.37">, DwarfRegNum<[37]>;
  def T27   : GPR<38, "r0.38">, DwarfRegNum<[38]>;
  def T28   : GPR<39, "r0.39">, DwarfRegNum<[39]>;
  def T29   : GPR<40, "r0.40">, DwarfRegNum<[40]>;
  def T30   : GPR<41, "r0.41">, DwarfRegNum<[41]>;
  def T31   : GPR<42, "r0.42">, DwarfRegNum<[42]>;
  def T32   : GPR<43, "r0.43">, DwarfRegNum<[43]>;
  def T33   : GPR<44, "r0.44">, DwarfRegNum<[44]>;
  def T34   : GPR<45, "r0.45">, DwarfRegNum<[45]>;
  def T35   : GPR<46, "r0.46">, DwarfRegNum<[46]>;
  def T36   : GPR<47, "r0.47">, DwarfRegNum<[47]>;
  def T37   : GPR<48, "r0.48">, DwarfRegNum<[48]>;
  def T38   : GPR<49, "r0.49">, DwarfRegNum<[49]>;
  def T39   : GPR<50, "r0.50">, DwarfRegNum<[50]>;
  def T40   : GPR<51, "r0.51">, DwarfRegNum<[51]>;
  def T41   : GPR<52, "r0.52">, DwarfRegNum<[52]>;
  def T42   : GPR<53, "r0.53">, DwarfRegNum<[53]>;
  def T43   : GPR<54, "r0.54">, DwarfRegNum<[54]>;
  def T44   : GPR<55, "r0.55">, DwarfRegNum<[55]>;
  def T45   : GPR<56, "r0.56">, DwarfRegNum<[56]>;
  def TS0   : GPR<57, "r0.57">, DwarfRegNum<[57]>;
  def TS1   : GPR<58, "r0.58">, DwarfRegNum<[58]>;
  def TS2   : GPR<59, "r0.59">, DwarfRegNum<[59]>;
  def TS3   : GPR<60, "r0.60">, DwarfRegNum<[60]>;
  def TS4   : GPR<61, "r0.61">, DwarfRegNum<[61]>;
  def TS5   : GPR<62, "r0.62">, DwarfRegNum<[62]>;
  def TS6   : GPR<63, "r0.63">, DwarfRegNum<[63]>;
  
  def LNK   : GPR<0, "l0.0">, DwarfRegNum<[64]>;

  // Branch Registers
  def B0    : BRR<0, "b0.0">, DwarfRegNum<[65]>;
  def B1    : BRR<1, "b0.1">, DwarfRegNum<[66]>;
  def B2    : BRR<2, "b0.2">, DwarfRegNum<[67]>;
  def B3    : BRR<3, "b0.3">, DwarfRegNum<[68]>;
  def B4    : BRR<4, "b0.4">, DwarfRegNum<[69]>;
  def B5    : BRR<5, "b0.5">, DwarfRegNum<[70]>;
  def B6    : BRR<6, "b0.6">, DwarfRegNum<[71]>;
  def B7    : BRR<7, "b0.7">, DwarfRegNum<[72]>;
}

//===---------------------------------------------------------------------===//
// Register Classes
//===---------------------------------------------------------------------===//

// First argument is the namespace
// Second is a list of ValueType register type values
// Third specifies the alignment required of the regs when they're stored or
// or loaded into memory
// The final specifies which regs are in the class

def CPURegs : RegisterClass<"LE1", [i32], 32, (add
  // Argument/return value regs
  STRP, AR0, AR1, AR2, AR3, AR4, AR5, AR6, AR7,
  // Temp regs that aren't saved over procedure calls
  T0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
  T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
  T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
  T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
  T40, T41, T42, T43, T44,
  // Callee save
  TS0, TS1, TS2, TS3, TS4, TS5, TS6,
  // Reserved
  ZERO, SP, LNK)>;

def BRegs : RegisterClass<"LE1", [i1], 32, (add
  B0, B1, B2, B3, B4, B5, B6, B7)> {
    let Size = 32;
  }
