#define FPTYPE int
#define FPVECTYPE uint4

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable 


// Compute a per block histogram of the occurrences of each
// digit, using a 4-bit radix (i.e. 16 possible digits).
__kernel void
reduce(__global const  int /*FPTYPE*/ * in, 
       __global  int /*FPTYPE*/ * isums, 
       const int n,
       __local  int /*FPTYPE*/ * lmem,
       const int shift) 
{
int region_size;
int block_start;
int block_stop;
int i[256];
  unsigned __esdg_idx = 0;
int tid[256];
int digit_counts[256][16];
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    // First, calculate the bounds of the region of the array 
    // that this block will sum.  We need these regions to match
    // perfectly with those in the bottom-level scan, so we index
    // as if vector types of length 4 were in use.  This prevents
    // errors due to slightly misaligned regions.
    //int region_size =

region_size =  ((n / 4) / get_num_groups(0)) * 4;
    //int block_start =

block_start =  get_group_id(0) * region_size;

    // Give the last block any extra elements
    //int block_stop  =

block_stop =  (get_group_id(0) == get_num_groups(0) - 1) ? 
        n : block_start + region_size;

    // Calculate starting index for this thread/work item
    //int tid =

tid[__esdg_idx] =  __esdg_idx;
    //int i =

i[__esdg_idx] =  block_start + tid[__esdg_idx];
    
    // The per thread histogram, initially 0's.
    digit_counts[__esdg_idx][0] = 0;
digit_counts[__esdg_idx][1] = 0;
digit_counts[__esdg_idx][2] = 0;
digit_counts[__esdg_idx][3] = 0;
digit_counts[__esdg_idx][4] = 0;
digit_counts[__esdg_idx][5] = 0;
digit_counts[__esdg_idx][6] = 0;
digit_counts[__esdg_idx][7] = 0;
digit_counts[__esdg_idx][8] = 0;
digit_counts[__esdg_idx][9] = 0;
digit_counts[__esdg_idx][10] = 0;
digit_counts[__esdg_idx][11] = 0;
digit_counts[__esdg_idx][12] = 0;
digit_counts[__esdg_idx][13] = 0;
digit_counts[__esdg_idx][14] = 0;
digit_counts[__esdg_idx][15] = 0;


    // Reduce multiple elements per thread
    while (i[__esdg_idx] < block_stop)
    {
        // This statement 
        // 1) Loads the value in from global memory
        // 2) Shifts to the right to have the 4 bits of interest
        //    in the least significant places
        // 3) Masks any more significant bits away. This leaves us
        // with the relevant digit (which is also the index into the
        // histogram). Next increment the histogram to count this occurrence.
        digit_counts[__esdg_idx][(in[i[__esdg_idx]] >> shift) & 0xFU]++;
        i[__esdg_idx] += 256;
    }
    
    
} __esdg_idx = 0;
for (int d = 0; d < 16; d++)
    {
  
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
      // Load this thread's sum into local/shared memory
        lmem[tid[__esdg_idx]] = digit_counts[__esdg_idx][d];
        //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {

        // Reduce the contents of shared/local memory
        
} __esdg_idx = 0;
for (unsigned int s = 256 / 2; s > 0; s >>= 1)
        {
  
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
          if (tid[__esdg_idx] < s)
            {
                lmem[tid[__esdg_idx]] += lmem[tid[__esdg_idx] + s];
            }
            //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
        
} __esdg_idx = 0;
}


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
        // Write result for this block to global memory
        if (tid[__esdg_idx] == 0)
        {
            isums[(d * get_num_groups(0)) + get_group_id(0)] = lmem[0];
        }
    
} __esdg_idx = 0;
}


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
} __esdg_idx = 0;
}

// This kernel scans the contents of local memory using a work
// inefficient, but highly parallel Kogge-Stone style scan.
// Set exclusive to 1 for an exclusive scan or 0 for an inclusive scan


// This single group kernel takes the per block histograms
// from the reduction and performs an exclusive scan on them.






