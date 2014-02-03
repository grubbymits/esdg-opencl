//#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

typedef struct latLong
    {
        float lat;
        float lng;
    } LatLong;

__kernel void Fan1(__global float *m_dev,
                  __global float *a_dev,
                  __global float *b_dev,
                  const int size,
                  const int t) {  unsigned __esdg_idx = 0;
for (__esdg_idx = 0; __esdg_idx < 2; ++__esdg_idx) {

    int globalId = get_group_id(0) * 2 + __esdg_idx;//get_global_id(0);
                              
    if (globalId < size-1-t) {
         *(m_dev + size * (globalId + t + 1)+t) = *(a_dev + size * (globalId + t + 1) + t) / *(a_dev + size * t + t);    
    }

__ESDG_END: ;
} __esdg_idx = 0;
}




