//===---------------------------------------------------------------------===//
//  Instruction Itinerary Class for a 3 wide LE1 with:
//  3 ALU(s), 3 MUL(s), 3 LSU(s) and 1 BRU(s)
//===---------------------------------------------------------------------===//
def ISS_0_3w3a3m3ls1b    : FuncUnit;
def ISS_1_3w3a3m3ls1b    : FuncUnit;
def ISS_2_3w3a3m3ls1b    : FuncUnit;
def ALU_0_3w3a3m3ls1b    : FuncUnit;
def ALU_1_3w3a3m3ls1b    : FuncUnit;
def ALU_2_3w3a3m3ls1b    : FuncUnit;
def MUL_0_3w3a3m3ls1b    : FuncUnit;
def MUL_1_3w3a3m3ls1b    : FuncUnit;
def MUL_2_3w3a3m3ls1b    : FuncUnit;
def LSU_0_3w3a3m3ls1b    : FuncUnit;
def LSU_1_3w3a3m3ls1b    : FuncUnit;
def LSU_2_3w3a3m3ls1b    : FuncUnit;
def BRU_0_3w3a3m3ls1b    : FuncUnit;

def LE13w3a3m3ls1bItineraries : ProcessorItineraries< [
    ISS_0_3w3a3m3ls1b, ISS_1_3w3a3m3ls1b, ISS_2_3w3a3m3ls1b, 
    ALU_0_3w3a3m3ls1b, ALU_1_3w3a3m3ls1b, ALU_2_3w3a3m3ls1b, 
    MUL_0_3w3a3m3ls1b, MUL_1_3w3a3m3ls1b, MUL_2_3w3a3m3ls1b, 
    LSU_0_3w3a3m3ls1b, LSU_1_3w3a3m3ls1b, LSU_2_3w3a3m3ls1b, 
    BRU_0_3w3a3m3ls1b ], [/*ByPass*/], [

    InstrItinData<IIAlu,  [InstrStage<1,
    [ISS_0_3w3a3m3ls1b, ISS_1_3w3a3m3ls1b, ISS_2_3w3a3m3ls1b], 0>,

                           InstrStage<1,
    [ALU_0_3w3a3m3ls1b, ALU_1_3w3a3m3ls1b, ALU_2_3w3a3m3ls1b]>], [2, 1]>,

    InstrItinData<IIMul,  [InstrStage<1,
    [ISS_0_3w3a3m3ls1b, ISS_1_3w3a3m3ls1b, ISS_2_3w3a3m3ls1b], 0>,

                           InstrStage<1,
    [MUL_0_3w3a3m3ls1b, MUL_1_3w3a3m3ls1b, MUL_2_3w3a3m3ls1b]>], [2, 1]>,

    InstrItinData<IILoadStore,  [InstrStage<1,
    [ISS_0_3w3a3m3ls1b, ISS_1_3w3a3m3ls1b, ISS_2_3w3a3m3ls1b], 0>,

                                 InstrStage<1,
    [LSU_0_3w3a3m3ls1b, LSU_1_3w3a3m3ls1b, LSU_2_3w3a3m3ls1b]>], [2, 1]>,

    InstrItinData<IIBranch,  [InstrStage<1,
    [ISS_0_3w3a3m3ls1b, ISS_1_3w3a3m3ls1b, ISS_2_3w3a3m3ls1b], 0>,

                              InstrStage<6,
    [BRU_0_3w3a3m3ls1b]>], [6, 1]> ]>;

def LE1Model3w3a3m3ls1b : SchedMachineModel {
  let IssueWidth = 3;
  let Itineraries = LE13w3a3m3ls1bItineraries;
}
