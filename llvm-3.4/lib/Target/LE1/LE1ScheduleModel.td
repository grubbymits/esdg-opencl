def LE1NewModel : SchedMachineModel {
  let IssueWidth = 4;
  let LoadLatency = 3;
}

// TODO May not need to use the IssueSlot as a Super resource as IssueWidth
// should be able to control this!
def IssueSlot : ProcResource<4>;

def ALU   : ProcResource<4> {
  let Super = IssueSlot;
}

def MULT  : ProcResource<1> {
  let Super = IssueSlot;
}

def LSU   : ProcResource<1> {
  let Super = IssueSlot;
}

def BRU   : ProcResource<1> {
  let Super = IssueSlot;
}

def IMM   : ProcResource<2> {
  let Super = IssueSlot;
}

def WriteA    : SchedWrite;
def WriteAI   : SchedWrite;
def WriteM    : SchedWrite;
def WriteMI   : SchedWrite;
def WriteLS   : SchedWrite;
def WriteLSI  : SchedWrite;
def WriteB    : SchedWrite;
def WriteBI   : SchedWrite;

let SchedModel = LE1NewModel in {

  def : WriteRes<WriteA, [ALU]>;
  def : WriteRes<WriteAI, [ALU, IMM]>;
  def : WriteRes<WriteM, [MULT]>;
  def : WriteRes<WriteMI, [MULT, IMM]>;
  def : WriteRes<WriteLS, [LSU]>;
  def : WriteRes<WriteLSI, [LSU, IMM]>;
  def : WriteRes<WriteB, [BRU]>;
  def : WriteRes<WriteBI, [BRU, IMM]>;

}
