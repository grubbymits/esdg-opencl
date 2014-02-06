//===---------------------------------------------------------------------===//
//  Instruction Itinerary Class for a 5 wide LE1 with:
//  5 ALU(s), 1 MUL(s), 1 LSU(s) and 1 BRU(s)
//===---------------------------------------------------------------------===//
def ALU_0_5w5a1m1ls1b    : FuncUnit;
def ALU_1_5w5a1m1ls1b    : FuncUnit;
def ALU_2_5w5a1m1ls1b    : FuncUnit;
def ALU_3_5w5a1m1ls1b    : FuncUnit;
def ALU_4_5w5a1m1ls1b    : FuncUnit;
def MUL_0_5w5a1m1ls1b    : FuncUnit;
def LSU_0_5w5a1m1ls1b    : FuncUnit;
def BRU_0_5w5a1m1ls1b    : FuncUnit;

def LE15w5a1m1ls1bItineraries : ProcessorItineraries< [
    
    ALU_0_5w5a1m1ls1b, ALU_1_5w5a1m1ls1b, ALU_2_5w5a1m1ls1b, ALU_3_5w5a1m1ls1b, ALU_4_5w5a1m1ls1b, 
    MUL_0_5w5a1m1ls1b, 
    LSU_0_5w5a1m1ls1b, 
    BRU_0_5w5a1m1ls1b ], [/*ByPass*/], [

    InstrItinData<IIAlu,  [InstrStage<1,
    [ALU_0_5w5a1m1ls1b, ALU_1_5w5a1m1ls1b, ALU_2_5w5a1m1ls1b, ALU_3_5w5a1m1ls1b, ALU_4_5w5a1m1ls1b]>], [2, 1]>,

    InstrItinData<IIMul,  [InstrStage<1,
    [MUL_0_5w5a1m1ls1b]>], [2, 1]>,

    InstrItinData<IILoadStore,  [InstrStage<1,
    [LSU_0_5w5a1m1ls1b]>], [2, 1]>,

    InstrItinData<IIBranch,  [InstrStage<1,
    [BRU_0_5w5a1m1ls1b]>], [6, 1]> ]>;

def LE1Model5w5a1m1ls1b : SchedMachineModel {
  let IssueWidth = 5;
  let Itineraries = LE15w5a1m1ls1bItineraries;
}
