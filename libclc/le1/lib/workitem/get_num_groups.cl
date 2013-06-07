#include <clc/clc.h>

_CLC_DEF size_t get_num_groups(uint dim) {

  // = global_size / local_size
  // Where global size is defined by the call to clEnqueueNDRangeKernel as too
  // is local_size, or the implementation can choose an appropriate size.

  // The generic get_global_size calculates it by:
  // get_num_groups(dim) * get_local_size(dim)
  // So that would cause a circular bug. Global size needs to saved in memory
  // and accessed and returned in that function.

  // get_global_size(dim) / get_local_size(dim) will be known at compile time so
  // it should be stored in memory and save us from doing a division at runtime!

  switch(dim) {
    case 0: return __builtin_le1_read_num_groups_0();
    case 1: return __builtin_le1_read_num_groups_1();
    case 2: return __builtin_le1_read_num_groups_2();
    default: return 0;
  }
}
