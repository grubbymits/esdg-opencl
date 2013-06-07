
__kernel
void test_kernel(__global int *input,
                 __global int *result,
                 int a)
{
  int gid = get_global_id(0);
  int i, j;
  int x, y = 5;

  int total;

 for (i = 0; i < 32; ++i) {
   result[gid] = input[gid];
   for (j = 0; j < i; ++j) {
      result[gid] = input[gid] * input[gid + j];
      total += result[gid];

      barrier(CLK_GLOBAL_MEM_FENCE);

      if (gid > 0)
        result[gid] = result[gid-1] + total + y;
   }
 }
}
