__kernel void array_mult(
   __global int* input1,
   __global int* input2,
   __global int* output)
{
  int __kernel_local_id[1];
   __kernel_local_id[0] = 0;
   while (__kernel_local_id[0] < 256) {
      int i = get_global_id(0) + __kernel_local_id[0];
   
   output[i] = input1[i] * input2[i];

__kernel_local_id[0]++;
}
}


