typedef struct {
 int a;
 int b;
} node;

__kernel void array_mult(
   __global node* input1,
   __global node* input2,
   __global int* output)
{
  int __kernel_local_id[1];
   __kernel_local_id[0] = 0;
   while (__kernel_local_id[0] < 8) {
      int i = get_global_id(0) + __kernel_local_id[0];
   
   int a_res = input1[i].a * input2[i].a;
   int b_res = input1[i].b * input2[i].b;
   output[i] = a_res + b_res;

__kernel_local_id[0]++;
}
}


