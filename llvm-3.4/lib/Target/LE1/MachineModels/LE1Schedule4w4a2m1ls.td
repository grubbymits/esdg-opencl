def LE1Model4w4a2m1ls : SchedMachineModel {
  let IssueWidth = 4;
  let LoadLatency = 2;
}
let SchedModel = LE1Model4w4a2m1ls in {
  def : WriteRes<WriteP, [PRED4]> {
    let NumMicroOps = 0;
  }  def : WriteRes<WriteA, [ALU4]> {
    let Latency = 2;
  }  def : WriteRes<WriteAI, [ALU4]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }  def : WriteRes<WriteM, [MULT2]> {
    let Latency = 2;
  }  def : WriteRes<WriteMI, [MULT2]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }  def : WriteRes<WriteLS, [LSU1]> {
    let Latency = 2;
  }  def : WriteRes<WriteLSI, [LSU1]> {
    let Latency = 2;
    let NumMicroOps = 2;
  }  def : WriteRes<WriteB, [BRU]> {
    let Latency = 5;
  }  def : WriteRes<WriteBI, [BRU]> {
    let Latency = 5;
    let NumMicroOps = 2;
  }}
