kernel
void test_kernel(__global int *input,
                 __global int *result,
                 int a) {
	int gid = get_global_id(0);
	int i, j;
	for (i = 0; i < 32; ++i) {
 		// open loop?

   		//result[gid] = input[gid];
   		result[0..15] = input[0..15];

   		// close loop?
		for (j = 0; j < i; ++j) {
      		//result[gid] = input[gid] * input[gid + j];
			// open loop?
      		result[0..15] = input[0..15] * input[0..15 + j];
      		// close loop?
      		barrier(CLK_GLOBAL_MEM_FENCE);
   		}
 	}
}

// First Iteration [ i = 0, j = 0]
// result[0..15] = input[0..15]
// No barrier is reached since j == i

// The second iteration [ i = 1, j = 0]
// result[0..15] = input[0..15]
// j < 1, so
// result[0..15] = input[0..15] * input[(0..15 + j)]

// The third iteration [i = 2, j = 0]
// result[0..15] = input[0..15]
// j = 0
// result[0..15] = input[0..15] * input[(0..15 + j)]
// j = 1
// result[0..15] = input[0..15] * input[(0..15 + j)]

// Fourth iteration [ i = 3, j = 0]
 // result[0..15] = input[0..15]
// j = 0
// result[0..15] = input[0..15] * input[(0..15 + j)]
// j = 1
// result[0..15] = input[0..15] * input[(0..15 + j)]
// j = 2
// result[0..15] = input[0..15] * input[(0..15 + j)]


// - visit loops
// - find barriers, find their scope
// - open a loop at the beginning of the scope, and close it
// 	 at the end
// - continue to search within the scope (loop) and if
//	 another barrier is encountered, close and re-open
//	 the loop.
// - if the loop is nested, insert loop(s) in the body of the
//	 parent loop.



for() {
	init_workitem()
	for() {
		workitem_work()
		barrier()
		more_workitem_work()
	}
	finalise_workitem()
}



