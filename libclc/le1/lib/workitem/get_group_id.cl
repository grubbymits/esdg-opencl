#include <clc/clc.h>

// Based on the number of global work-items. Work-items will be merged into a
// single workgroup. The number of workgroups will be based on the number of
// cores. So maybe group and local will be the same, depends how pocl merges
// the work-items and indexes the instances. If this is the case, just reading
// the CPU ID should be enough.
_CLC_DEF size_t get_group_id(uint dim) {
  switch (dim) {
  case 0:  return __builtin_le1_read_cpuid();
  case 1:  return __builtin_le1_read_cpuid();
  case 2:  return __builtin_le1_read_cpuid();
  default: return 0;
  }
}
