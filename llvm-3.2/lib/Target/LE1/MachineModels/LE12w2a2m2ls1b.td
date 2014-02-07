//===---------------------------------------------------------------------===//
//  Instruction Itinerary Class for a 2 wide LE1 with:
//  2 ALU(s), 2 MUL(s), 2 LSU(s) and 1 BRU(s)
//===---------------------------------------------------------------------===//
def ALU_0_2w2a2m2ls1b    : FuncUnit;
def ALU_1_2w2a2m2ls1b    : FuncUnit;
def MUL_0_2w2a2m2ls1b    : FuncUnit;
def MUL_1_2w2a2m2ls1b    : FuncUnit;
def LSU_0_2w2a2m2ls1b    : FuncUnit;
def LSU_1_2w2a2m2ls1b    : FuncUnit;
def BRU_0_2w2a2m2ls1b    : FuncUnit;

def LE12w2a2m2ls1bItineraries : ProcessorItineraries< [
    
    ALU_0_2w2a2m2ls1b, ALU_1_2w2a2m2ls1b, 
    MUL_0_2w2a2m2ls1b, MUL_1_2w2a2m2ls1b, 
    LSU_0_2w2a2m2ls1b, LSU_1_2w2a2m2ls1b, 
    BRU_0_2w2a2m2ls1b ], [/*ByPass*/], [

    InstrItinData<IIAlu,  [InstrStage<1,
    [ALU_0_2w2a2m2ls1b, ALU_1_2w2a2m2ls1b]>], [2, 1]>,

    InstrItinData<IIMul,  [InstrStage<1,
    [MUL_0_2w2a2m2ls1b, MUL_1_2w2a2m2ls1b]>], [2, 1]>,

    InstrItinData<IILoadStore,  [InstrStage<1,
    [LSU_0_2w2a2m2ls1b, LSU_1_2w2a2m2ls1b]>], [2, 1]>,

    InstrItinData<IIBranch,  [InstrStage<1,
    [BRU_0_2w2a2m2ls1b]>], [6, 1]> ]>;

def LE1Model2w2a2m2ls1b : SchedMachineModel {
  let IssueWidth = 2;
  let Itineraries = LE12w2a2m2ls1bItineraries;
}
