__kernel void simple_barrier(__global int *A, __global int *B,
                             __global int *res) {

  uint idx = get_global_id(0);
  res[idx] = A[idx] * B[idx];
  barrier (CLK_GLOBAL_MEM_FENCE);

  if (idx < get_global_size(0) - 1) {
    res[idx] = res[idx + 1] + res[idx];
  }
}

__kernel void barrier_in_loop(__global int *A, __global int *B,
                              __global int *res) {

  unsigned idx = get_global_id(0);
  res[idx] = A[idx] * B[idx];
  barrier (CLK_GLOBAL_MEM_FENCE);

  for (unsigned i = 0; i < get_global_size(0)-1; ++i) {
    int acc = res[i+1] - res[i];
    barrier (CLK_GLOBAL_MEM_FENCE);
    res[i] = res[i] * acc;
  }
}
