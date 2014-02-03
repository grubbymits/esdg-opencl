#define FPTYPE int
#define FPVECTYPE uint4

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable 


// Compute a per block histogram of the occurrences of each
// digit, using a 4-bit radix (i.e. 16 possible digits).


// This kernel scans the contents of local memory using a work
// inefficient, but highly parallel Kogge-Stone style scan.
// Set exclusive to 1 for an exclusive scan or 0 for an inclusive scan


// This single group kernel takes the per block histograms
// from the reduction and performs an exclusive scan on them.



__kernel void 
bottom_scan(__global const  int /*FPTYPE*/ * in,
            __global const  int /*FPTYPE*/ * isums,
            __global  int /*FPTYPE*/ * out,
            const int n,
            __local  int /*FPTYPE*/ * lmem,
            const int shift)
{
    int n4;
    int region_size;
    int block_start;
    unsigned __esdg_idx = 0;
    int __trans_tmp_2[256];
    int histogram[256][16];
    int l_scanned_seeds[16];
    int l_block_counts[16];
    __global uint4 * in4;
    int block_stop;
    int i[256];
    int window[256];

    for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    
        // Use local memory to cache the scanned seeds   
        // Keep a shared histogram of all instances seen by the current
        // block
        // Keep a private histogram as well
        histogram[__esdg_idx][0] = 0;
        histogram[__esdg_idx][1] = 0;
        histogram[__esdg_idx][2] = 0;
        histogram[__esdg_idx][3] = 0;
        histogram[__esdg_idx][4] = 0;
        histogram[__esdg_idx][5] = 0;
        histogram[__esdg_idx][6] = 0;
        histogram[__esdg_idx][7] = 0;
        histogram[__esdg_idx][8] = 0;
        histogram[__esdg_idx][9] = 0;
        histogram[__esdg_idx][10] = 0;
        histogram[__esdg_idx][11] = 0;
        histogram[__esdg_idx][12] = 0;
        histogram[__esdg_idx][13] = 0;
        histogram[__esdg_idx][14] = 0;
        histogram[__esdg_idx][15] = 0;

        // Prepare for reading 4-element vectors
        // Assume n is divisible by 4
        //__global  uint4 /*FPVECTYPE*/ *in4  =

        in4 =  (__global  uint4 /*FPVECTYPE*/*) in;
        __global  uint4 /*FPVECTYPE*/ *out4 = (__global  uint4 /*FPVECTYPE*/*) out;
        //int n4 =
        n4 =  n / 4; //vector type is 4 wide
    
        //int region_size =
        region_size =  n4 / get_num_groups(0);
        //int block_start =
        block_start =  get_group_id(0) * region_size;
        // Give the last block any extra elements
        //int block_stop  =

        block_stop =  (get_group_id(0) == get_num_groups(0) - 1) ? 
            n4 : block_start + region_size;

        // Calculate starting index for this thread/work item
        //int i =
        i[__esdg_idx] =  block_start + __esdg_idx;
        //int window =
        window[__esdg_idx] =  block_start;

        // Set the histogram in local memory to zero
        // and read in the scanned seeds from gmem
        if (__esdg_idx < 16)
        {
            l_block_counts[__esdg_idx] = 0;
            l_scanned_seeds[__esdg_idx] = 
                isums[(__esdg_idx*get_num_groups(0))+get_group_id(0)];
        }
        //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
    } __esdg_idx = 0;

    uint4 val_4[256];
    uint4 key_4[256];
    while (window[__esdg_idx] < block_stop)
    {
  
        for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
            // Reset histogram
            for (int q = 0; q < 16; q++)
                histogram[__esdg_idx][q] = 0;
         
                 
            if (i[__esdg_idx] < block_stop) // Make sure we don't read out of bounds
            {
                val_4[__esdg_idx] = in4[i[__esdg_idx]];
            
                // Mask the keys to get the appropriate digit
                key_4[__esdg_idx].x = (val_4[__esdg_idx].x >> shift) & 0xFU;
                key_4[__esdg_idx].y = (val_4[__esdg_idx].y >> shift) & 0xFU;
                key_4[__esdg_idx].z = (val_4[__esdg_idx].z >> shift) & 0xFU;
                key_4[__esdg_idx].w = (val_4[__esdg_idx].w >> shift) & 0xFU;
            
                // Update the histogram
                histogram[__esdg_idx][key_4[__esdg_idx].x]++;
                histogram[__esdg_idx][key_4[__esdg_idx].y]++;
                histogram[__esdg_idx][key_4[__esdg_idx].z]++;
                histogram[__esdg_idx][key_4[__esdg_idx].w]++;
            } 
                
            // Scan the digit counts in local memory
       
        } __esdg_idx = 0;
        for (int digit = 0; digit < 16; digit++)
        {

            __local int * lmem_arg[256];
            int idx[256];
            int t[256];
            int val_arg[256];
            int exclusive[256];
            {
  
                for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
                    //int val_arg =
                    val_arg[__esdg_idx] =  histogram[__esdg_idx][digit];
                    //__local int *lmem_arg =
                    lmem_arg[__esdg_idx] =  lmem;
                    //int exclusive =
                    exclusive[__esdg_idx] =  1;
            
                    // Set first half of local memory to zero to make room for scanning
                    //int idx =
                    idx[__esdg_idx] =  __esdg_idx;
                    lmem_arg[__esdg_idx][idx[__esdg_idx]] = 0;
                
                    // Set second half to block sums from global memory, but don't go out
                    // of bounds
                    idx[__esdg_idx] += 256;
                    lmem_arg[__esdg_idx][idx[__esdg_idx]] = val_arg[__esdg_idx];
                    //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
                } __esdg_idx = 0;


                for (int i = 1; i < 256; i *= 2)
                {
  
                    for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
                        t[__esdg_idx] = lmem_arg[__esdg_idx][idx[__esdg_idx] -  i]; //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
                    } __esdg_idx = 0;


                    for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
                        lmem_arg[__esdg_idx][idx[__esdg_idx]] += t[__esdg_idx];     //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
                    } __esdg_idx = 0;
                }
 
                for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
                    __trans_tmp_2[__esdg_idx] =  lmem_arg[__esdg_idx][idx[__esdg_idx]-exclusive[__esdg_idx]];
            
                } __esdg_idx = 0;
            }
 
            for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
           
                histogram[__esdg_idx][digit] = __trans_tmp_2[__esdg_idx];
                //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
            } __esdg_idx = 0;

        }


        for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
            if (i[__esdg_idx] < block_stop) // Make sure we don't write out of bounds
            {
                int address;
                address = histogram[__esdg_idx][key_4[__esdg_idx].x] + l_scanned_seeds[key_4[__esdg_idx].x] + l_block_counts[key_4[__esdg_idx].x];
                out[address] = val_4[__esdg_idx].x;
                histogram[__esdg_idx][key_4[__esdg_idx].x]++;
            
                address = histogram[__esdg_idx][key_4[__esdg_idx].y] + l_scanned_seeds[key_4[__esdg_idx].y] + l_block_counts[key_4[__esdg_idx].y];
                out[address] = val_4[__esdg_idx].y;
                histogram[__esdg_idx][key_4[__esdg_idx].y]++;
            
                address = histogram[__esdg_idx][key_4[__esdg_idx].z] + l_scanned_seeds[key_4[__esdg_idx].z] + l_block_counts[key_4[__esdg_idx].z];
                out[address] = val_4[__esdg_idx].z;
                histogram[__esdg_idx][key_4[__esdg_idx].z]++;
            
                address = histogram[__esdg_idx][key_4[__esdg_idx].w] + l_scanned_seeds[key_4[__esdg_idx].w] + l_block_counts[key_4[__esdg_idx].w];
                out[address] = val_4[__esdg_idx].w;
                histogram[__esdg_idx][key_4[__esdg_idx].w]++;
            }
                
            // Before proceeding, make sure everyone has finished their current
            // indexing computations.
            //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
        } __esdg_idx = 0;


        for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
            // Now update the seed array.
            if (__esdg_idx == 256-1)
            {
                for (int q = 0; q < 16; q++)
                {
                    l_block_counts[q] += histogram[__esdg_idx][q];
                }
            }
            //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
        } __esdg_idx = 0;


        for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
        
            // Advance window
            window[__esdg_idx] += 256;
            i[__esdg_idx] += 256;
    
        } __esdg_idx = 0;
    }
}


