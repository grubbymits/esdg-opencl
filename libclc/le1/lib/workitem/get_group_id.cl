#include <clc/clc.h>

_CLC_DEF size_t get_group_id(uint dim) {
  switch (dim) {
  case 0: 
    return __builtin_le1_read_cpuid()
            + (__builtin_le1_num_cores() * __builtin_le1_read_group_id_0());
  case 1:
    return __builtin_le1_read_cpuid()
            + (__builtin_le1_num_cores() * __builtin_le1_read_group_id_1());
  case 2:
    return __builtin_le1_read_cpuid()
            + (__builtin_le1_num_cores() * __builtin_le1_read_group_id_2());
  default: return 0;
  }
}
