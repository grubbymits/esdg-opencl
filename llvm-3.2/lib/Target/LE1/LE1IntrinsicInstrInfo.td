def le1_reset_local_id    : SDNode<"LE1ISD::ResetLocalID", SDTNone,
                                  [SDNPHasChain]>;
def le1_incr_local_id     : SDNode<"LE1ISD::IncrLocalID", SDTIntUnaryOp,
                                  [SDNPMayStore, SDNPMayLoad, SDNPHasChain]>;

def SDTLE1ReadReg         : SDTypeProfile<1, 0, [SDTCisVT<0, i32>]>;
def le1_cpuid             : SDNode<"LE1ISD::CPUID", SDTLE1ReadReg, []>;
def le1_read_local_size   : SDNode<"LE1ISD::LocalSize", SDTIntUnaryOp, []>;
def le1_read_global_id    : SDNode<"LE1ISD::GlobalId", SDTIntUnaryOp, []>;
def le1_group_id_addr     : SDNode<"LE1ISD::GROUP_ID_ADDR", SDTLE1ReadReg, []>;
def le1_load_group_id     : SDNode<"LE1ISD::LOAD_GROUP_ID", SDTIntUnaryOp, []>;
def le1_read_attr         : SDNode<"LE1ISD::READ_ATTR", SDTIntUnaryOp, []>;

/*
let Constraints = "$dst = $src", mayLoad = 1, mayStore = 1  in {
  class IncrReg<RegisterClass RC>
    : LE1Inst<
      (outs RC:$dst),
      (ins RC:$src),
      "add.0  $dst, $src, 1",
      [(set RC:$dst, (le1_incr_local_id RC:$src))], IIAlu>;
}

def IncLocalIdx : IncrReg<LocalIdx>;
def IncLocalIdy : IncrReg<LocalIdy>;
def IncLocalIdz : IncrReg<LocalIdz>;

def : Pat<(int_le1_inc_local_idx),
          (IncLocalIdx IdX)>;
def : Pat<(int_le1_inc_local_idy),
          (IncLocalIdx IdY)>;
def : Pat<(int_le1_inc_local_idz),
          (IncLocalIdz IdZ)>;*/


let Defs = [IdX] in
def ResetLocalIdX :
  LE1Inst<(outs), (ins),
  "mov.0  r0.53, r0.0",
  [(int_le1_reset_local_idx)], IIAlu>;

let Defs = [IdY] in
def ResetLocalIdY :
  LE1Inst<(outs), (ins),
  "mov.0  r0.54, r0.0",
  [(int_le1_reset_local_idy)], IIAlu>;

let Defs = [IdZ] in
def ResetLocalIdZ :
  LE1Inst<(outs), (ins),
  "mov.0  r0.55, r0.0",
  [(int_le1_reset_local_idz)], IIAlu>;

let Uses = [IdX], Defs = [IdX] in
def IncLocalIdX :
  LE1Inst<(outs), (ins),
  "add.0  r0.53, r0.53, 1",
  [(int_le1_inc_local_idx)], IIAlu>;

let Uses = [IdY], Defs = [IdY] in
def IncLocalIdY :
  LE1Inst<(outs), (ins),
  "add.0  r0.54, r0.54, 1",
  [(int_le1_inc_local_idy)], IIAlu>;

let Uses = [IdZ], Defs = [IdZ] in
def IncLocalIdZ :
  LE1Inst<(outs), (ins),
  "add.0  r0.55, r0.55, 1",
  [(int_le1_inc_local_idz)], IIAlu>;

def GroupIdAddr :
  LE1Inst<
  (outs CPURegs:$dst),
  (ins),
  "mov.0 $dst, group_id",
  [(set CPURegs:$dst, (le1_group_id_addr))], IIAlu>;

def LoadGroupId :
  LE1Inst<
  (outs CPURegs:$dst),
  (ins CPURegs:$addr),
  "ldbu.0 $dst, $addr[0x0]",
  [(set CPURegs:$dst, (le1_load_group_id CPURegs:$addr))], IILoadStore>;

def ReadAttr :
  LE1Inst<
  (outs CPURegs:$dst),
  (ins CPURegs:$addr),
  "ldbu.0 $dst, $addr[0x0]",
  [(set CPURegs:$dst, (le1_read_attr CPURegs:$addr))], IILoadStore>;

class LE1_READ_CTRL_REG<string opcode, Intrinsic IntID>
  : LE1Inst<(outs CPURegs:$dst), (ins),
            !strconcat(opcode, "  $dst"),
            [/*(set CPURegs:$dst, (IntID))*/], IIAlu>;

class LE1_READ_KERNEL_ATTR<string addr, Intrinsic IntID>
  : LE1Inst<(outs CPURegs:$dst), (ins),
            !strconcat("ldw.0 $dst, r0.0[(", addr, ")]"),
            [(set CPURegs:$dst, (IntID))], IILoadStore>;

def LE1_READ_CPUID : LE1_READ_CTRL_REG<"cpuid.0", int_le1_read_cpuid>;

def LE1_READ_WORK_DIM : LE1_READ_KERNEL_ATTR<"work_dim+0",
                                             int_le1_read_work_dim>;

def LE1_NUM_CORES : LE1_READ_KERNEL_ATTR<"num_cores+0",
                                         int_le1_num_cores>;

def LE1_READ_GLOBAL_SIZE_0 : LE1_READ_KERNEL_ATTR<"global_size+0",
                                                  int_le1_read_global_size_0>;
def : Pat<(int_le1_read_global_size_0),
          (LE1_READ_GLOBAL_SIZE_0)>;
def LE1_READ_GLOBAL_SIZE_1 : LE1_READ_KERNEL_ATTR<"global_size+4",
                                                  int_le1_read_global_size_1>;
def LE1_READ_GLOBAL_SIZE_2 : LE1_READ_KERNEL_ATTR<"global_size+8",
                                                  int_le1_read_global_size_2>;

def LE1_READ_LOCAL_SIZE_0 : LE1_READ_KERNEL_ATTR<"local_size+0",
                                                  int_le1_read_local_size_0>;
def LE1_READ_LOCAL_SIZE_1 : LE1_READ_KERNEL_ATTR<"local_size+4",
                                                  int_le1_read_local_size_1>;
def LE1_READ_LOCAL_SIZE_2 : LE1_READ_KERNEL_ATTR<"local_size+8",
                                                  int_le1_read_local_size_2>;
/*
def LE1_READ_GROUP_ID_0   : LE1_READ_KERNEL_ATTR<"group_id+0",
                                                 int_le1_read_group_id_0>;
def LE1_READ_GROUP_ID_1   : LE1_READ_KERNEL_ATTR<"group_id+4",
                                                 int_le1_read_group_id_1>;
def LE1_READ_GROUP_ID_2   : LE1_READ_KERNEL_ATTR<"group_id+8",
                                                 int_le1_read_group_id_2>;
*/
def LE1_READ_NUM_GROUPS_0 : LE1_READ_KERNEL_ATTR<"num_groups+0",
                                                  int_le1_read_num_groups_0>;
def LE1_READ_NUM_GROUPS_1 : LE1_READ_KERNEL_ATTR<"num_groups+4",
                                                  int_le1_read_num_groups_1>;
def LE1_READ_NUM_GROUPS_2 : LE1_READ_KERNEL_ATTR<"num_groups+8",
                                                  int_le1_read_num_groups_2>;

def LE1_READ_GLOBAL_OFFSET_0 : LE1_READ_KERNEL_ATTR<"global_offset+0",
                                                  int_le1_read_global_offset_0>;
def LE1_READ_GLOBAL_OFFSET_1 : LE1_READ_KERNEL_ATTR<"global_offset+4",
                                                  int_le1_read_global_offset_1>;
def LE1_READ_GLOBAL_OFFSET_2 : LE1_READ_KERNEL_ATTR<"global_offset+8",
                                                  int_le1_read_global_offset_2>;

def : Pat<(le1_read_local_size (i32 0)),
          (LE1_READ_LOCAL_SIZE_0)>;
def : Pat<(le1_read_local_size (i32 1)),
          (LE1_READ_LOCAL_SIZE_1)>;
def : Pat<(le1_read_local_size (i32 2)),
          (LE1_READ_LOCAL_SIZE_2)>;

def : Pat<(le1_cpuid),
          (LE1_READ_CPUID)>;

