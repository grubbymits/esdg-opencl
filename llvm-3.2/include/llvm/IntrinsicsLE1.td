let TargetPrefix = "le1" in
class LE1_Intrinsic<string GCCIntSuffix, list<LLVMType> ret_types,
                    list<LLVMType> param_types,
                    list<IntrinsicProperty> properties>
  : GCCBuiltin<!strconcat("__builtin_", GCCIntSuffix)>,
    Intrinsic<ret_types, param_types, properties>;

class LE1ReadCtrlReg_Intrinsic<string GCCIntSuffix>
  : LE1_Intrinsic<GCCIntSuffix, [llvm_i32_ty], [], [IntrNoMem]>;

/*
class LE1ReadCLReg_Intrinsic<string GCCIntSuffix>
  : LE1_Intrinsic<GCCIntSuffix, [llvm_i32_ty], [], [IntrNoMem]>;

class LE1WriteCLReg_Intrinsic<string GCCIntSuffix>
  : LE1_Intrinsic<GCCIntSuffix, [], [],
                  [IntrReadWriteArgMem]>;*/

// FIXME These needs to use chains
class LE1ReadConstKernelAttr_Intrinsic<string GCCIntSuffix>
  : LE1_Intrinsic<GCCIntSuffix, [llvm_i32_ty], [], [IntrNoMem]>;

class LE1ReadKernelAttr_Intrinsic<string GCCIntSuffix>
  : LE1_Intrinsic<GCCIntSuffix, [llvm_i32_ty], [], [IntrReadWriteArgMem]>;

class LE1SetKernelAttr_Intrinsic<string GCCIntSuffix>
  : LE1_Intrinsic<GCCIntSuffix, [], [llvm_i32_ty], [IntrReadWriteArgMem]>;

def int_le1_read_cpuid  : LE1ReadCtrlReg_Intrinsic<"le1_read_cpuid">;

def int_le1_read_work_dim : LE1ReadKernelAttr_Intrinsic<"le1_read_work_dim">;

def int_le1_num_cores     : LE1ReadConstKernelAttr_Intrinsic<"le1_num_cores">;

def int_le1_read_global_size_0 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_global_size_0">;
def int_le1_read_global_size_1 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_global_size_1">;
def int_le1_read_global_size_2 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_global_size_2">;

def int_le1_read_local_size_0 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_local_size_0">;
def int_le1_read_local_size_1 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_local_size_1">;
def int_le1_read_local_size_2 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_local_size_2">;

def int_le1_read_group_id_0 :
  LE1ReadKernelAttr_Intrinsic<"le1_read_group_id_0">;
def int_le1_read_group_id_1 :
  LE1ReadKernelAttr_Intrinsic<"le1_read_group_id_1">;
def int_le1_read_group_id_2 :
  LE1ReadKernelAttr_Intrinsic<"le1_read_group_id_2">;

def int_le1_set_group_id_0 :
  LE1SetKernelAttr_Intrinsic<"le1_set_groupd_id_0">;
def int_le1_set_group_id_1 :
  LE1SetKernelAttr_Intrinsic<"le1_set_groupd_id_1">;
def int_le1_set_group_id_2 :
  LE1SetKernelAttr_Intrinsic<"le1_set_groupd_id_2">;

def int_le1_read_num_groups_0 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_num_groups_0">;
def int_le1_read_num_groups_1 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_num_groups_1">;
def int_le1_read_num_groups_2 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_num_groups_2">;

def int_le1_read_global_offset_0 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_global_offset_0">;
def int_le1_read_global_offset_1 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_global_offset_1">;
def int_le1_read_global_offset_2 :
    LE1ReadConstKernelAttr_Intrinsic<"le1_read_global_offset_2">;

/*

def int_le1_set_barrier     : LE1_Intrinsic<"le1_set_barrier",
                                            [], [llvm_i32_ty],
                                            [IntrReadWriteArgMem]>;
def int_le1_read_barrier    : LE1_Intrinsic<"le1_read_barrier",
                                            [llvm_i32_ty], [llvm_i32_ty],
                                            [IntrReadWriteArgMem]>;

def int_le1_read_global_idx : LE1_Intrinsic<"le1_read_global_idx",
                                            [llvm_i32_ty], [],
                                            [IntrNoMem]>;
def int_le1_read_global_idy : LE1_Intrinsic<"le1_read_global_idy",
                                            [llvm_i32_ty], [],
                                            [IntrNoMem]>;
def int_le1_read_global_idz : LE1_Intrinsic<"le1_read_global_idz",
                                            [llvm_i32_ty], [],
                                            [IntrNoMem]>;

def int_le1_reset_local_idx :
  LE1WriteCLReg_Intrinsic<"le1_reset_local_idx">;
def int_le1_reset_local_idy :
  LE1WriteCLReg_Intrinsic<"le1_reset_local_idy">;
def int_le1_reset_local_idz :
  LE1WriteCLReg_Intrinsic<"le1_reset_local_idz">;

def int_le1_inc_local_idx :
  LE1WriteCLReg_Intrinsic<"le1_inc_local_idx">;
def int_le1_inc_local_idy :
  LE1WriteCLReg_Intrinsic<"le1_inc_local_idy">;
def int_le1_inc_local_idz :
  LE1WriteCLReg_Intrinsic<"le1_inc_local_idz">;
*/
