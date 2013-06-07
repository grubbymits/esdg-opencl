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
/*
def 1HContext : SubtargetFeature<"1hc", "HyperContext1", "true",
                                 "Single Hyper Context - Default", []>;
def 2HContext : SubtargetFeature<"2hc", "HyperContext2", "true",
                                 "Two Hyper Contexts", []>;
def 3HContext : SubtargetFeature<"3hc", "HyperContext3", "true",
                                 "Three Hyper Contexts", []>;
def 4HContext : SubtargetFeature<"4hc", "HyperContext4", "true",
                                 "Four Hyper Contexts", []>;
def 5HContext : SubtargetFeature<"5hc", "HyperContext5", "true",
                                 "Five Hyper Contexts", []>;
def 6HContext : SubtargetFeature<"6hc", "HyperContext6", "true",
                                 "Six Hyper Contexts", []>;
def 7HContext : SubtargetFeature<"7hc", "HyperContext7", "true",
                                 "Seven Hyper Contexts", []>;
def 8HContext : SubtargetFeature<"8hc", "HyperContext8", "true",
                                 "Eight Hyper Contexts", []>;
def 9HContext : SubtargetFeature<"9hc", "HyperContext9", "true",
                                 "Nine Hyper Contexts", []>;
def 10HContext : SubtargetFeature<"10hc", "HyperContext10", "true",
                                 "Ten Hyper Contexts", []>;
def 11HContext : SubtargetFeature<"11hc", "HyperContext5", "true",
                                 "Eleven Hyper Contexts", []>;
def 12HContext : SubtargetFeature<"12hc", "HyperContext12", "true",
                                 "Twelve Hyper Contexts", []>;
def 13HContext : SubtargetFeature<"13hc", "HyperContext13", "true",
                                 "Thirteen Hyper Contexts", []>;*/

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
def : Processor< "scalar", LE1ScalarItineraries, []>;
def : Processor< "2w2a2m2ls1b", LE12w2a2m2ls1bItineraries, []>;
def : Processor< "3w3a3m3ls1b", LE13w3a3m3ls1bItineraries, []>;
