__kernel void square(
   __global int* input1,
   __global int* input2,
   __global int* output)
{
   __private int i = get_global_id(0);
   __private int j = i * 4;
   output[i] = input1[i] * input2[i] + j;
}


