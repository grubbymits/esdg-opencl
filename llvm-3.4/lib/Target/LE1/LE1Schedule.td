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

// Functional Units
def ALU_0   : FuncUnit;
def MUL_0   : FuncUnit;
def LSU_0   : FuncUnit;
def BRU_0    : FuncUnit;

// IntAlus  - arithmetic, boolean logic, compares, shifts : latency 1-2 cycles
// IntMuls  - multiplication : latency 2-3 cycles
// 1Branch  - branch, calls : 5 cycles
// LSU      - loads, store : latency 3-4 cycles

//===----------------------------------------------------------------------===//
// LE1 Generic instruction itineraries.
//===----------------------------------------------------------------------===//
def LE1ScalarItineraries : ProcessorItineraries<
                            [ALU_0, MUL_0, LSU_0, BRU_0], [], [

    InstrItinData<IIAlu, [
                          InstrStage<1, [ALU_0]>], [2, 1]>,

    InstrItinData<IILoadStore, [
                                InstrStage<1, [LSU_0]>], [2, 1]>,

    InstrItinData<IIMul, [
                          InstrStage<1, [MUL_0]>], [2, 1]>,

    InstrItinData<IIBranch, [
                             InstrStage<5, [BRU_0]>], [6, 1]> ]>;

def LE1ModelScalar : SchedMachineModel {
  let IssueWidth = 1;
  let LoadLatency = 3;
  let Itineraries = LE1ScalarItineraries;
}
//include "MachineModels/LE12w2a1m1ls1b.td"
//include "MachineModels/LE12w2a1m2ls1b.td"
//include "MachineModels/LE13w3a1m1ls1b.td"
//include "MachineModels/LE13w3a1m2ls1b.td"
//include "MachineModels/LE14w4a1m1ls1b.td"
//include "MachineModels/LE14w4a1m2ls1b.td"
//include "MachineModels/LE14w4a1m4ls1b.td"
//include "MachineModels/LE15w5a1m1ls1b.td"
//include "MachineModels/LE15w5a1m2ls1b.td"
//include "MachineModels/LE15w5a1m4ls1b.td"
//include "MachineModels/LE14w4a1m3ls1b.td"
include "LE1ScheduleModel.td"
