__kernel void array_mult(
   __global int* input1,
   __global int* input2,
   __global int* output)
{
   int i = get_global_id(0);
   output[i] = input1[i] * input2[i];
}


