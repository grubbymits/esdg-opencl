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
/*
def IIAluR      : InstrItinClass;
def IIAlu       : InstrItinClass;
def IICmpBr     : InstrItinClass;
def IICmpGr     : InstrItinClass;
def IISelect    : InstrItinClass;
def IIMult      : InstrItinClass;
def IILoad      : InstrItinClass;
def IILoadLr    : InstrItinClass;
def IIStore     : InstrItinClass;
def IIPft       : InstrItinClass;
def IIAsm1l     : InstrItinClass;
def IIAsm2l     : InstrItinClass;
def IIAsm3l     : InstrItinClass;
def IIAsm4l     : InstrItinClass;
def IIAsm1h     : InstrItinClass;
def IIAsm2h     : InstrItinClass;
def IIAsm3h     : InstrItinClass;
def IIAsm4h     : InstrItinClass;
def IICpGrBr    : InstrItinClass;
def IICpBrGr    : InstrItinClass;
def IICpGrLr    : InstrItinClass;
def IICpLrGr    : InstrItinClass;
def IISpill     : InstrItinClass;
def IIRestore   : InstrItinClass;
def IIRestoreLr : InstrItinClass;
*/
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
  let Itineraries = LE1ScalarItineraries;
}

include "MachineModels/LE12w_1a_1m_1ls.td"
include "MachineModels/LE12w_1a_1m_2ls.td"
include "MachineModels/LE12w_1a_2m_1ls.td"
include "MachineModels/LE12w_1a_2m_2ls.td"
include "MachineModels/LE12w_2a_1m_1ls.td"
include "MachineModels/LE12w_2a_1m_2ls.td"
include "MachineModels/LE12w_2a_2m_1ls.td"
include "MachineModels/LE12w_2a_2m_2ls.td"
include "MachineModels/LE14w_2a_1m_1ls.td"
include "MachineModels/LE14w_2a_1m_2ls.td"
include "MachineModels/LE14w_2a_2m_1ls.td"
include "MachineModels/LE14w_2a_2m_2ls.td"
include "MachineModels/LE14w_3a_1m_1ls.td"
include "MachineModels/LE14w_3a_1m_2ls.td"
include "MachineModels/LE14w_3a_2m_1ls.td"
include "MachineModels/LE14w_3a_2m_2ls.td"
include "MachineModels/LE14w_4a_1m_1ls.td"
include "MachineModels/LE14w_4a_1m_2ls.td"
include "MachineModels/LE14w_4a_2m_1ls.td"
include "MachineModels/LE14w_4a_2m_2ls.td"
