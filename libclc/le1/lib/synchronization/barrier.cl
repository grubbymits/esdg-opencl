#include <clc/clc.h>

_CLC_DEF void barrier(cl_mem_fence_flags flags) {
  if (flags & CLK_LOCAL_MEM_FENCE) {
    unsigned local_id = __builtin_le1_read_local_idx() +
                         __builtin_le1_read_local_idy() +
                         __builtin_le1_read_local_idz();
    __builtin_le1_set_barrier(local_id);
    while (!__builtin_le1_read_barrier(local_id));
  }
}

