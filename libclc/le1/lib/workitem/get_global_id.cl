#include <clc/clc.h>

const _CLC_DEF size_t get_global_id(uint dim) {
  switch(dim) {
    case 0:   return __builtin_le1_read_global_idx();
    case 1:   return __builtin_le1_read_global_idy();
    case 2:   return __builtin_le1_read_global_idz();
  }
}
