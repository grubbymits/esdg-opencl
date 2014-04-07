def LE1Model : SchedMachineModel {
  let IssueWidth = 4;
  let LoadLatency = 3;
}

def IssueSlot : ProcResource<4>;

def ALU : ProcResource<4> {
  let Super = IssueSlot;
}

def MULT : ProcResource<1> {
  let Super = IssueSlot;
}

def LSU : ProcResource<1> {
  let Super = IssueSlot;
}

def BR : ProcResource<1> {
  let Super = IssueSlot;
}

def IMM : ProcResource<2> {
  let Super = IssueSlot;
}
