
__kernel
void test_kernel(__global int *input1,
                 __global int *input2,
                 __global int *result,
                 int a)
{
  int gid = get_global_id(0);
  result[gid] = input1[gid] * input2[gid];
}
