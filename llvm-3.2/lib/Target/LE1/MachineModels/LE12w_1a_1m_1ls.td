// --------------  LE1 Itinerary for 2w_1a_1m_1ls -------------- // 

def ALU_0_2w_1a_1m_1ls  : FuncUnit;
def MUL_0_2w_1a_1m_1ls  : FuncUnit;
def LSU_0_2w_1a_1m_1ls  : FuncUnit;
def BRU_0_2w_1a_1m_1ls   : FuncUnit;

def LE12w_1a_1m_1lsItineraries : ProcessorItineraries< [
    ALU_0_2w_1a_1m_1ls, 
    MUL_0_2w_1a_1m_1ls, 
    LSU_0_2w_1a_1m_1ls, 
    BRU_0_2w_1a_1m_1ls ], [/*ByPass*/], [

    InstrItinData<IIAlu, [InstrStage<1, [
     ALU_0_2w_1a_1m_1ls]>], [3, 1]>,

    InstrItinData<IIMul, [InstrStage<1, [
     MUL_0_2w_1a_1m_1ls]>], [3, 1]>,

    InstrItinData<IILoadStore, [InstrStage<1, [
   LSU_0_2w_1a_1m_1ls]>], [3, 1]>,

    InstrItinData<IIBranch, [InstrStage<1, [BRU_0_2w_1a_1m_1ls]>], [6, 1]> 
]>;

def LE1Model2w_1a_1m_1ls : SchedMachineModel {
    let IssueWidth = 2;
    let Itineraries = LE12w_1a_1m_1lsItineraries;
}
