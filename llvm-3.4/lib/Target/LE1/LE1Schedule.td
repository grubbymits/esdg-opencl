//===- LE1Schedule.td - LE1 Scheduling Definitions -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Functional units across LE1 chips sets. Based on GCC/LE1 backend files.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction Itinerary classes used for LE1
//===----------------------------------------------------------------------===//
def IIAlu       : InstrItinClass;
def IIMul       : InstrItinClass;
def IILoadStore : InstrItinClass;
def IIBranch    : InstrItinClass;
def IIPseudo    : InstrItinClass;

def ALU1  : ProcResource<1>;
def ALU2  : ProcResource<2>;
def ALU3  : ProcResource<3>;
def ALU4  : ProcResource<4>;
def MULT1 : ProcResource<1>;
def MULT2 : ProcResource<2>;
def LSU1  : ProcResource<1>;
def LSU2  : ProcResource<2>;
def BRU   : ProcResource<1>;
def PRED1 : ProcResource<1>;
def PRED2 : ProcResource<2>;
def PRED3 : ProcResource<3>;
def PRED4 : ProcResource<4>;

def WriteA    : SchedWrite;
def WriteAI   : SchedWrite;
def WriteM    : SchedWrite;
def WriteMI   : SchedWrite;
def WriteLS   : SchedWrite;
def WriteLSI  : SchedWrite;
def WriteB    : SchedWrite;
def WriteBI   : SchedWrite;
def WriteP    : SchedWrite;

def LE1ModelScalar : SchedMachineModel {
  let IssueWidth = 1;
  let LoadLatency = 3;
}

let SchedModel = LE1ModelScalar in {

  def : WriteRes<WriteP, [PRED1]> {
    let NumMicroOps = 0;
  }
  def : WriteRes<WriteA, [ALU1]> {
    let Latency = 2;
  }
  def : WriteRes<WriteAI, [ALU1]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }
  def : WriteRes<WriteM, [MULT1]> {
    let Latency = 2;
  }
  def : WriteRes<WriteMI, [MULT1]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }
  def : WriteRes<WriteLS, [LSU1]> {
    let Latency = 3;
  }
  def : WriteRes<WriteLSI, [LSU1]> {
    let Latency = 3;
    let NumMicroOps = 2;
  }
  def : WriteRes<WriteB, [BRU]> {
    let Latency = 5;
  }
  def : WriteRes<WriteBI, [BRU]> {
    let Latency = 5;
    let NumMicroOps = 2;
  }
}

include "MachineModels/LE1Schedule2w1m1ls.td"
include "MachineModels/LE1Schedule3w1m1ls.td"
include "MachineModels/LE1Schedule4w1m1ls.td"
include "MachineModels/LE1Schedule4w2m1ls.td"
include "MachineModels/LE1Schedule4w2m2ls.td"
