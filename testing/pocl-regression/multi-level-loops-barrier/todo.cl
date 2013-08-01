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


kernel() {
  kernel_init()
  for() {
    init_workitem()
    for() {
      workitem_work()
      barrier()
      more_workitem_work()
    }
    finalise_workitem()
  }
}

// All work items need to have reached the barrier before the for loop is able
// to loop back. workitem_work needs to be replicated, but that also means that
// all the data for it also needs to have been calculated as well.

kernel() {
  open_loop()
    kernel_init()
  close_loop()

  for() {
    open_loop()
      init_workitem()
    close_loop()

    for() {
      open_loop()
        workitem_work()
      close_loop()

      barrier()

      open_loop()
        more_workitem_work()
      close_loop()
    }

    open_loop()
      finalise_workitem()
    close_loop()
  }
}



