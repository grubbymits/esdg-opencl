#include "clc/clc.h"

_CLC_DEF size_t get_global_offset(uint dim) {
  switch(dim) {
    case 0 : return __builtin_le1_read_global_offset_0();
    case 1 : return __builtin_le1_read_global_offset_1();
    case 2 : return __builtin_le1_read_global_offset_2();
    default : return 0;
  }
}
