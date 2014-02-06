//===---------------------------------------------------------------------===//
//  Instruction Itinerary Class for a 4 wide LE1 with:
//  4 ALU(s), 1 MUL(s), 1 LSU(s) and 1 BRU(s)
//===---------------------------------------------------------------------===//
def ALU_0_4w4a1m1ls1b    : FuncUnit;
def ALU_1_4w4a1m1ls1b    : FuncUnit;
def ALU_2_4w4a1m1ls1b    : FuncUnit;
def ALU_3_4w4a1m1ls1b    : FuncUnit;
def MUL_0_4w4a1m1ls1b    : FuncUnit;
def LSU_0_4w4a1m1ls1b    : FuncUnit;
def BRU_0_4w4a1m1ls1b    : FuncUnit;

def LE14w4a1m1ls1bItineraries : ProcessorItineraries< [
    
    ALU_0_4w4a1m1ls1b, ALU_1_4w4a1m1ls1b, ALU_2_4w4a1m1ls1b, ALU_3_4w4a1m1ls1b, 
    MUL_0_4w4a1m1ls1b, 
    LSU_0_4w4a1m1ls1b, 
    BRU_0_4w4a1m1ls1b ], [/*ByPass*/], [

    InstrItinData<IIAlu,  [InstrStage<1,
    [ALU_0_4w4a1m1ls1b, ALU_1_4w4a1m1ls1b, ALU_2_4w4a1m1ls1b, ALU_3_4w4a1m1ls1b]>], [2, 1]>,

    InstrItinData<IIMul,  [InstrStage<1,
    [MUL_0_4w4a1m1ls1b]>], [2, 1]>,

    InstrItinData<IILoadStore,  [InstrStage<1,
    [LSU_0_4w4a1m1ls1b]>], [2, 1]>,

    InstrItinData<IIBranch,  [InstrStage<1,
    [BRU_0_4w4a1m1ls1b]>], [6, 1]> ]>;

def LE1Model4w4a1m1ls1b : SchedMachineModel {
  let IssueWidth = 4;
  let Itineraries = LE14w4a1m1ls1bItineraries;
}
