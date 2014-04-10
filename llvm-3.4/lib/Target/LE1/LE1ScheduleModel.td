def LE1NewModel : SchedMachineModel {
  let IssueWidth = 4;
  let LoadLatency = 3;
}

// TODO May not need to use the IssueSlot as a Super resource as IssueWidth
// should be able to control this!
//def IssueSlot : ProcResource<4>;

def ALU   : ProcResource<4> {
  //let Super = IssueSlot;
}

def MULT  : ProcResource<1> {
  //let Super = IssueSlot;
}

def LSU   : ProcResource<1> {
  //let Super = IssueSlot;
}

def BRU   : ProcResource<1> {
  //let Super = IssueSlot;
}

def IMM   : ProcResource<2> {
  //let Super = IssueSlot;
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

  def : WriteRes<WriteA, [ALU]> {
    let Latency = 2;
  }
  def : WriteRes<WriteAI, [ALU, IMM]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }
  def : WriteRes<WriteM, [MULT]> {
    let Latency = 2;
  }
  def : WriteRes<WriteMI, [MULT, IMM]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }
  def : WriteRes<WriteLS, [LSU]> {
    let Latency = 3;
  }
  def : WriteRes<WriteLSI, [LSU, IMM]> {
    let Latency = 3;
    let NumMicroOps = 2;
  }
  def : WriteRes<WriteB, [BRU]> {
    let Latency = 5;
  }
  def : WriteRes<WriteBI, [BRU, IMM]> {
    let Latency = 5;
    let NumMicroOps = 2;
  }

}
