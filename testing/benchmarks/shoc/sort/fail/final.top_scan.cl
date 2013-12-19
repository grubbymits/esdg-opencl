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
top_scan(__global  int /*FPTYPE*/ * isums, 
         const int n,
         __local  int /*FPTYPE*/ * lmem)
{
  unsigned __esdg_idx = 0;
int __trans_tmp_1[256];
int s_seed;
int last_thread[256];
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    
    
    s_seed = 0; //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    
    // Decide if this is the last thread that needs to 
    // propagate the seed value
    //int last_thread =

last_thread[__esdg_idx] =  (__esdg_idx < n &&
                      (__esdg_idx+1) == n) ? 1 : 0;

   
} __esdg_idx = 0;
 int val[256];
int res[256];
for (int d = 0; d < 16; d++)
    {
  
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
       //int /*FPTYPE*/ val =

val[__esdg_idx] =  0;
        // Load each block's count for digit d
        if (__esdg_idx < n)
        {
            val[__esdg_idx] = isums[(n * d) + __esdg_idx];
        }
        // Exclusive scan the counts in local memory
        
} __esdg_idx = 0;
 __local int * lmem_arg[256];
int idx[256];
int t[256];
int val_arg[256];
int exclusive;
{
  
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
       //int val_arg =

val_arg[__esdg_idx] =  val[__esdg_idx];
         //__local int *lmem_arg =

lmem_arg[__esdg_idx] =  lmem;
         //int exclusive =

exclusive =  1;
         
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


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
             
             // Now, perform Kogge-Stone scan
              
            
} __esdg_idx = 0;
 for (int i = 1; i < 256; i *= 2)
             {
  
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
               t[__esdg_idx] = lmem_arg[__esdg_idx][idx[__esdg_idx] -  i]; //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
                 lmem_arg[__esdg_idx][idx[__esdg_idx]] += t[__esdg_idx];     //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
             
} __esdg_idx = 0;
}
 
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
            __trans_tmp_1[__esdg_idx] =  lmem_arg[__esdg_idx][idx[__esdg_idx]-exclusive];
         
} __esdg_idx = 0;
}
 
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
        
         //int /*FPTYPE*/ res =

res[__esdg_idx] =  __trans_tmp_1[__esdg_idx];
        // Write scanned value out to global
        if (__esdg_idx < n)
        {
            isums[(n * d) + __esdg_idx] = res[__esdg_idx] + s_seed;
        }
        
        if (last_thread[__esdg_idx]) 
        {
            s_seed += res[__esdg_idx] + val[__esdg_idx];
        }
        //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    
} __esdg_idx = 0;
}


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
} __esdg_idx = 0;
}





