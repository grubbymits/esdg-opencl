// --------------  LE1 Itinerary for 4w_2a_2m_1ls -------------- // 

def ALU_0_4w_2a_2m_1ls  : FuncUnit;
def ALU_1_4w_2a_2m_1ls  : FuncUnit;
def MUL_0_4w_2a_2m_1ls  : FuncUnit;
def MUL_1_4w_2a_2m_1ls  : FuncUnit;
def LSU_0_4w_2a_2m_1ls  : FuncUnit;
def BRU_0_4w_2a_2m_1ls   : FuncUnit;

def LE14w_2a_2m_1lsItineraries : ProcessorItineraries< [
    ALU_0_4w_2a_2m_1ls, 
    ALU_1_4w_2a_2m_1ls, 
    MUL_0_4w_2a_2m_1ls, 
    MUL_1_4w_2a_2m_1ls, 
    LSU_0_4w_2a_2m_1ls, 
    BRU_0_4w_2a_2m_1ls ], [/*ByPass*/], [

    InstrItinData<IIAlu, [InstrStage<1, [
     ALU_0_4w_2a_2m_1ls, ALU_1_4w_2a_2m_1ls]>], [2, 1]>,

    InstrItinData<IIMul, [InstrStage<1, [
     MUL_0_4w_2a_2m_1ls, MUL_1_4w_2a_2m_1ls]>], [2, 1]>,

    InstrItinData<IILoadStore, [InstrStage<1, [
   LSU_0_4w_2a_2m_1ls]>], [3, 1]>,

    InstrItinData<IIBranch, [InstrStage<1, [BRU_0_4w_2a_2m_1ls]>], [6, 1]> 
]>;

def LE1Model4w_2a_2m_1ls : SchedMachineModel {
    let IssueWidth = 4;
    let Itineraries = LE14w_2a_2m_1lsItineraries;
}
