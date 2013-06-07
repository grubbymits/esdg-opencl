__kernel void
vector_mult (__global const ushort4 *a,
	     __global const uint4 *b, __global uint4 *c)
{
  int gid = get_global_id(0);

  c[gid] = a[gid] * b[gid];
}
