#include <clc/clc.h>

// local_id is added during AST transformation
const _CLC_DEF size_t get_global_id(uint dim) {
  switch(dim) {
    case 0:   return get_group_id(0) * get_local_size(0);
    case 1:   return get_group_id(1) * get_local_size(1);
    case 2:   return get_group_id(2) * get_local_size(2);
  }
}
