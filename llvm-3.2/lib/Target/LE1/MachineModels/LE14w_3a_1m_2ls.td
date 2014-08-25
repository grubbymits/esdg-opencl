// --------------  LE1 Itinerary for 4w_3a_1m_2ls -------------- // 

def ALU_0_4w_3a_1m_2ls  : FuncUnit;
def ALU_1_4w_3a_1m_2ls  : FuncUnit;
def ALU_2_4w_3a_1m_2ls  : FuncUnit;
def MUL_0_4w_3a_1m_2ls  : FuncUnit;
def LSU_0_4w_3a_1m_2ls  : FuncUnit;
def LSU_1_4w_3a_1m_2ls  : FuncUnit;
def BRU_0_4w_3a_1m_2ls   : FuncUnit;

def LE14w_3a_1m_2lsItineraries : ProcessorItineraries< [
    ALU_0_4w_3a_1m_2ls, 
    ALU_1_4w_3a_1m_2ls, 
    ALU_2_4w_3a_1m_2ls, 
    MUL_0_4w_3a_1m_2ls, 
    LSU_0_4w_3a_1m_2ls, 
    LSU_1_4w_3a_1m_2ls, 
    BRU_0_4w_3a_1m_2ls ], [/*ByPass*/], [

    InstrItinData<IIAlu, [InstrStage<1, [
     ALU_0_4w_3a_1m_2ls, ALU_1_4w_3a_1m_2ls, ALU_2_4w_3a_1m_2ls]>], [2, 1]>,

    InstrItinData<IIMul, [InstrStage<1, [
     MUL_0_4w_3a_1m_2ls]>], [2, 1]>,

    InstrItinData<IILoadStore, [InstrStage<1, [
   LSU_0_4w_3a_1m_2ls, LSU_1_4w_3a_1m_2ls]>], [3, 1]>,

    InstrItinData<IIBranch, [InstrStage<1, [BRU_0_4w_3a_1m_2ls]>], [6, 1]> 
]>;

def LE1Model4w_3a_1m_2ls : SchedMachineModel {
    let IssueWidth = 4;
    let Itineraries = LE14w_3a_1m_2lsItineraries;
}
