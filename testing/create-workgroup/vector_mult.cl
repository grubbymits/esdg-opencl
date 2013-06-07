__kernel void
vector_mult (__global const int4 *a,
	     __global const int4 *b, __global int4 *c)
{
  int gid = get_global_id(0);

  c[gid] = a[gid] * b[gid];
}
