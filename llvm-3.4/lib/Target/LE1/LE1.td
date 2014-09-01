//===- LE1.td - Describe the LE1 Target Machine ----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the LE1 target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "LE1RegisterInfo.td"
include "LE1Schedule.td"
include "LE1InstrInfo.td"
include "LE1CallingConv.td"

def LE1InstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// LE1 Subtarget features                                                    //
//===----------------------------------------------------------------------===//

//def FeatureScalar   : SubtargetFeature<"scalar", "LE1ArchVersion", "LE1Scalar",
  //                            "VThreads ISA Support", []>;
def ExpandDiv : SubtargetFeature<"expand-div", "HasExpandDiv", "true",
                                  "VThreads ISA Support, with inline division", 
                                  []>;
def NeedsNops : SubtargetFeature<"needs-nops", "NeedsNops", "true",
                                  "Inserting Nops for simulator", []>;

def LE1AsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def LE1 : Target {
  let InstructionSet = LE1InstrInfo;

  let AssemblyWriters = [LE1AsmWriter];
}

//===----------------------------------------------------------------------===//
// LE1 processors supported.
//===----------------------------------------------------------------------===//
def : ProcessorModel<"scalar", LE1ModelScalar, []>;
def : ProcessorModel<"1w1a1m1ls", LE1Model1w1a1m1ls, []>;
def : ProcessorModel<"2w1a1m1ls", LE1Model2w1a1m1ls, []>;
def : ProcessorModel<"2w1a1m2ls", LE1Model2w1a1m2ls, []>;
def : ProcessorModel<"2w1a2m1ls", LE1Model2w1a2m1ls, []>;
def : ProcessorModel<"2w1a2m2ls", LE1Model2w1a2m2ls, []>;
def : ProcessorModel<"2w2a1m1ls", LE1Model2w2a1m1ls, []>;
def : ProcessorModel<"2w2a1m2ls", LE1Model2w2a1m2ls, []>;
def : ProcessorModel<"2w2a2m1ls", LE1Model2w2a2m1ls, []>;
def : ProcessorModel<"2w2a2m2ls", LE1Model2w2a2m2ls, []>;
def : ProcessorModel<"4w2a1m1ls", LE1Model4w2a1m1ls, []>;
def : ProcessorModel<"4w2a1m2ls", LE1Model4w2a1m2ls, []>;
def : ProcessorModel<"4w2a2m1ls", LE1Model4w2a2m1ls, []>;
def : ProcessorModel<"4w2a2m2ls", LE1Model4w2a2m2ls, []>;
def : ProcessorModel<"4w3a1m1ls", LE1Model4w3a1m1ls, []>;
def : ProcessorModel<"4w3a1m2ls", LE1Model4w3a1m2ls, []>;
def : ProcessorModel<"4w3a2m1ls", LE1Model4w3a2m1ls, []>;
def : ProcessorModel<"4w3a2m2ls", LE1Model4w3a2m2ls, []>;
def : ProcessorModel<"4w4a1m1ls", LE1Model4w4a1m1ls, []>;
def : ProcessorModel<"4w4a1m2ls", LE1Model4w4a1m2ls, []>;
def : ProcessorModel<"4w4a2m1ls", LE1Model4w4a2m1ls, []>;
def : ProcessorModel<"4w4a2m2ls", LE1Model4w4a2m2ls, []>;
