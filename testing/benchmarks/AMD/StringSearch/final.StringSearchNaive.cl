/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

• Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
• Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

#define TOLOWER(x) (('A' <= (x) && (x) <= 'Z') ? ((x - 'A') + 'a') : (x))

/**
* @brief Compare two strings with specified length
* @param text       start position on text string
* @param pattern    start position on pattern string
* @param length     Length to compare
* @return 0-failure, 1-success
*/
int compare(__global const uchar* text, __local const uchar* pattern, uint length)
{
    for(uint l=0; l<length; ++l)
    {
#ifdef CASE_SENSITIVE
        /*if (text[l] != pattern[l]) return 0;*/
#else
        if ( ( ( 'A' <= ( text [ l ] ) && ( text [ l ] ) <= 'Z' ) ? ( ( text [ l ] - 'A' ) + 'a' ) : ( text [ l ] ) )  /*TOLOWER*/ /*(text[l])*/ != pattern[l]) return 0;
#endif
    }
    return 1;
}

/**
* @brief Naive kernel version of string search.
*        Find all pattern positions in the given text
* @param text               Input Text
* @param textLength         Length of the text
* @param pattern            Pattern string
* @param patternLength      Pattern length
* @param resultBuffer       Result of all matched positions
* @param resultCountPerWG   Result counts per Work-Group
* @param maxSearchLength    Maximum search positions for each work-group
* @param localPattern       local buffer for the search pattern
*/
__kernel void
    StringSearchNaive (
      __global uchar* text,
      const uint textLength,
      __global const uchar* pattern,
      const uint patternLength,
      __global int* resultBuffer,
      __global int* resultCountPerWG,
      const uint maxSearchLength,
      __local uchar* localPattern)
{ uint groupSuccessCounter;
int localIdx[256];
int localSize;
int groupIdx;
uint lastSearchIdx;
uint beginSearchIdx;
uint endSearchIdx;
  unsigned __esdg_idx = 0;
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
 
    

    //int localIdx =

localIdx[__esdg_idx] =  __esdg_idx;
    //int localSize =

localSize =  256;
    //int groupIdx =

groupIdx =  get_group_id(0);

    // Last search idx for all work items
    //uint lastSearchIdx =

lastSearchIdx =  textLength - patternLength + 1;

    // global idx for all work items in a WorkGroup
    //uint beginSearchIdx =

beginSearchIdx =  groupIdx * maxSearchLength;
    //uint endSearchIdx =

endSearchIdx =  beginSearchIdx + maxSearchLength;

   
} __esdg_idx = 0;
 if(beginSearchIdx > lastSearchIdx) {
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {

     
} __esdg_idx = 0;
 return;
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {

    
} __esdg_idx = 0;
}
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {


    if(endSearchIdx > lastSearchIdx) endSearchIdx = lastSearchIdx;

    // Copy the pattern from global to local buffer
    for(int idx = localIdx[__esdg_idx]; idx < patternLength; idx+=localSize)
    {
#ifdef CASE_SENSITIVE
        /*localPattern[idx] = pattern[idx];*/
#else
        localPattern[idx] =  ( ( 'A' <= ( pattern [ idx ] ) && ( pattern [ idx ] ) <= 'Z' ) ? ( ( pattern [ idx ] - 'A' ) + 'a' ) : ( pattern [ idx ] ) ) /*TOLOWER*/ /*(pattern[idx])*/;
#endif
    }

    if(localIdx[__esdg_idx] == 0) groupSuccessCounter = 0;
    //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {

    // loop over positions in global buffer
    for(uint stringPos=beginSearchIdx+localIdx[__esdg_idx]; stringPos<endSearchIdx; stringPos+=localSize)
    {
        if (compare(text+stringPos, localPattern, patternLength) == 1)
        {
            int count = groupSuccessCounter++; //atomic_inc(&groupSuccessCounter);
            resultBuffer[beginSearchIdx+count] = stringPos;
        }
    }

    //barrier( 1  /*CLK_LOCAL_MEM_FENCE*/);
} __esdg_idx = 0;


for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    if(localIdx[__esdg_idx] == 0) resultCountPerWG[groupIdx] = groupSuccessCounter;

__ESDG_END: ;
} __esdg_idx = 0;
}

/**
* @brief Load-Balance kernel version of string search.
*        Find all pattern positions in the given text
* @param text               Input Text
* @param textLength         Length of the text
* @param pattern            Pattern string
* @param patternLength      Pattern length
* @param resultBuffer       Result of all matched positions
* @param resultCountPerWG   Result counts per Work-Group
* @param maxSearchLength    Maximum search positions for each work-group
* @param localPattern       local buffer for the search pattern
* @param stack1             local stack for store initial 2-byte match 
* @param stack2             local stack for store initial 10-byte match positions
*/


