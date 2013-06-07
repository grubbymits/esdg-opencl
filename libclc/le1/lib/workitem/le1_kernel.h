#ifndef _LE1_KERNEL_H_
#define _LE1_KERNEL_H_

// Structure to hold the information based by clEnqueueNDRangeKernel
typedef struct {
  uint work_dim;
  uint global_size[3];
  uint local_size[3];
  uint num_groups[3];
  uint global_offset;
} Kernel_Attrs;

// define memory addresses here?
// label as __global const?
const __global Kernel_Attrs* kernel_attrs = 0x0;

#endif
