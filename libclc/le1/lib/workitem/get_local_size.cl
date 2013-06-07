#include <clc/clc.h>

const _CLC_DEF size_t get_local_size(uint dim) {
  switch (dim) {
  case 0:  return __builtin_le1_read_local_size_0();
  case 1:  return __builtin_le1_read_local_size_1();
  case 2:  return __builtin_le1_read_local_size_2();
  default: return 0;
  }
}
