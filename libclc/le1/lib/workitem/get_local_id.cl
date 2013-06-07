#include <clc/clc.h>

// See get_group_id
_CLC_DEF size_t get_local_id(uint dim) {
  switch (dim) {
  case 0:  return __builtin_le1_read_local_idx();
  case 1:  return __builtin_le1_read_local_idy();
  case 2:  return __builtin_le1_read_local_idz();
  default: return 0;
  }
}
