#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>
#include "OpenCL.h"
#include "support.h"

bool verifySort(unsigned int *keys, const size_t size)
{
    bool passed = true;

    for (unsigned int i = 0; i < size - 1; i++)
    {
        if (keys[i] > keys[i + 1])
        {
            passed = false;
        }
        std::cout << "keys[" << i << "] = " << keys[i] << std::endl;
    }
    std::cout << "Test ";
    if (passed)
        std::cout << "Passed" << std::endl;
    else
        std::cout << "---FAILED---" << std::endl;

    return passed;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Error: device num not passed!\n");
    exit(1);
  }
  unsigned device_num = atoi(argv[1]);
  // Initialise
  OpenCL cl(1);
  cl.getDevices(CL_DEVICE_TYPE_ACCELERATOR, device_num);
  cl.buildSource("sort.cl");
  cl.createKernel("reduce");
  cl.createKernel("top_scan");
  cl.createKernel("bottom_scan");

  if (getMaxWorkGroupSize(cl.devid(0)) < 256) {
    std::cout << "Scan requires work group size of at least 256" << std::endl;
    return -1;
  }

  // Problem Sizes
  int probSizes[4] = { 1, 8, 32, 64 };
  int size = probSizes[1];

  // Convert to MiB
  //size = (size * 1024 * 1024) / sizeof(unsigned);
  // Convert to KiB
  size = (size * 1024) / sizeof(unsigned);

  // Create input data on CPU
  unsigned int bytes = size * sizeof(unsigned);

  cl_int err = CL_SUCCESS;

  // Allocate pinned host memory for input data (h_idata)
  cl_mem h_i = clCreateBuffer(cl.ctxt(),
                              CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                              bytes, NULL, &err);
  CL_CHECK_ERROR(err);

  cl_uint* h_idata = (cl_uint*)clEnqueueMapBuffer(cl.queue(),
                                                  h_i,
                                                  true,
                                                  CL_MAP_READ|CL_MAP_WRITE,
                                                  0,
                                                  bytes, 0, NULL, NULL, &err);
  CL_CHECK_ERROR(err);

  // Allocate pinned host memory for output data (h_odata)
  cl_mem h_o = clCreateBuffer(cl.ctxt(),
                              CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                              bytes, NULL, &err);
  CL_CHECK_ERROR(err);

  cl_uint* h_odata = (cl_uint*)clEnqueueMapBuffer(cl.queue(),
                                                  h_o,
                                                  true,
                                                  CL_MAP_READ|CL_MAP_WRITE,
                                                  0,
                                                  bytes, 0, NULL, NULL, &err);
  CL_CHECK_ERROR(err);

  // Initialize host memory
  std::cout << "Initializing host memory." << std::endl;
  for (int i = 0; i < size; i++) {
    h_idata[i] = i % 16; // Fill with some pattern
    h_odata[i] = -1;
  }
  // The radix width in bits
  const int radix_width = 4; // Changing this requires major kernel updates
  const int num_digits = (int)pow((double)2, radix_width); // n possible digits

  // Allocate device memory for input array
  cl_mem d_idata = clCreateBuffer(cl.ctxt(), CL_MEM_READ_WRITE, bytes,
                                  NULL, &err);
  CL_CHECK_ERROR(err);
    
  // Allocate device memory for output array
  cl_mem d_odata = clCreateBuffer(cl.ctxt(), CL_MEM_READ_WRITE, bytes,
                                  NULL, &err);
  CL_CHECK_ERROR(err);

  // Number of local work items per group
  const size_t local_wsize  = 256;
    
  // Number of global work items
  //const size_t global_wsize = 16384; // i.e. 64 work groups
  const size_t global_wsize = size;
  const size_t num_work_groups = global_wsize / local_wsize;

  // Allocate device memory for local work group intermediate sums
  cl_mem d_isums = clCreateBuffer(cl.ctxt(), CL_MEM_READ_WRITE,
                                  num_work_groups * num_digits * sizeof(cl_int),
                                  NULL, &err);
  CL_CHECK_ERROR(err);

  // Set the kernel arguments for the reduction kernel
  err = cl.setArg("reduce", 0, sizeof(cl_mem), (void*)&d_idata);
  CL_CHECK_ERROR(err);
  err = cl.setArg("reduce", 1, sizeof(cl_mem), (void*)&d_isums);
  CL_CHECK_ERROR(err);
  err = cl.setArg("reduce", 2, sizeof(cl_int), (void*)&size);
  CL_CHECK_ERROR(err);
  err = cl.setArg("reduce", 3, local_wsize * sizeof(unsigned), NULL);
  CL_CHECK_ERROR(err);

  // Set the kernel arguments for the top-level scan
  err = cl.setArg("top_scan", 0, sizeof(cl_mem), (void*)&d_isums);
  CL_CHECK_ERROR(err);
  err = cl.setArg("top_scan", 1, sizeof(cl_int), (void*)&num_work_groups);
  CL_CHECK_ERROR(err);
  err = cl.setArg("top_scan", 2, local_wsize * 2 * sizeof(unsigned), NULL);
  CL_CHECK_ERROR(err);

  // Set the kernel arguments for the bottom-level scan
  err = cl.setArg("bottom_scan", 0, sizeof(cl_mem), (void*)&d_idata);
  CL_CHECK_ERROR(err);
  err = cl.setArg("bottom_scan", 1, sizeof(cl_mem), (void*)&d_isums);
  CL_CHECK_ERROR(err);
  err = cl.setArg("bottom_scan", 2, sizeof(cl_mem), (void*)&d_odata);
  CL_CHECK_ERROR(err);
  err = cl.setArg("bottom_scan", 3, sizeof(cl_int), (void*)&size);
  CL_CHECK_ERROR(err);
  err = cl.setArg("bottom_scan", 4, local_wsize * 2 * sizeof(unsigned), NULL);
  CL_CHECK_ERROR(err);

  // Copy data to GPU
  std::cout << "Copying input data to device." << std::endl;
  err = clEnqueueWriteBuffer(cl.queue(), d_idata, true, 0, bytes, h_idata, 0,
                             NULL, NULL);
  CL_CHECK_ERROR(err);
  err = cl.finish();
  CL_CHECK_ERROR(err);

#ifdef DEBUG_SORT
  unsigned temp_data[global_wsize] = {0};
#endif

  // Run
  // Assuming an 8 bit byte.
  for (int shift = 0; shift < sizeof(unsigned)*8; shift += radix_width) {

    // Like scan, we use a reduce-then-scan approach
    // But before proceeding, update the shift appropriately
    // for each kernel. This is how many bits to shift to the
    // right used in binning.
    err = clSetKernelArg(cl.kernel("reduce"), 4, sizeof(cl_int), (void*)&shift);
    CL_CHECK_ERROR(err);

    err = clSetKernelArg(cl.kernel("bottom_scan"), 5, sizeof(cl_int),
                         (void*)&shift);
    CL_CHECK_ERROR(err);

    // Also, the sort is not in place, so swap the input and output
    // buffers on each pass.
    bool even = ((shift / radix_width) % 2 == 0) ? true : false;
    if (even) {
      // Set the kernel arguments for the reduction kernel
      err = clSetKernelArg(cl.kernel("reduce"), 0, sizeof(cl_mem),
                           (void*)&d_idata);
      CL_CHECK_ERROR(err);

      // Set the kernel arguments for the bottom-level scan
      err = clSetKernelArg(cl.kernel("bottom_scan"), 0, sizeof(cl_mem),
                           (void*)&d_idata);
      CL_CHECK_ERROR(err);

      err = clSetKernelArg(cl.kernel("bottom_scan"), 2, sizeof(cl_mem),
                           (void*)&d_odata);
      CL_CHECK_ERROR(err);
    }
    else { // i.e. odd pass
      // Set the kernel arguments for the reduction kernel
      err = clSetKernelArg(cl.kernel("reduce"), 0, sizeof(cl_mem),
                           (void*)&d_odata);
      CL_CHECK_ERROR(err);

      // Set the kernel arguments for the bottom-level scan
      err = clSetKernelArg(cl.kernel("bottom_scan"), 0, sizeof(cl_mem),
                           (void*)&d_odata);
      CL_CHECK_ERROR(err);

      err = clSetKernelArg(cl.kernel("bottom_scan"), 2, sizeof(cl_mem),
                           (void*)&d_idata);
      CL_CHECK_ERROR(err);
    }

    std::cout << "RUN REDUCE" << std::endl;
    // Each thread block gets an equal portion of the
    // input array, and computes occurrences of each digit.
    err = clEnqueueNDRangeKernel(cl.queue(), cl.kernel("reduce"), 1, NULL,
                                 &global_wsize, &local_wsize, 0, NULL, NULL);
#ifdef DEBUG_SORT
    err = clEnqueueReadBuffer(cl.queue(), d_isums, true, 0,
                              num_work_groups * num_digits * sizeof(cl_int),
                              temp_data, 0, NULL, NULL);
    for (unsigned i = 0; i < num_work_groups * num_digits; ++i)
      std::cout << "d_isums[" << i << "] = " << temp_data[i] << std::endl;
#endif

    cl.finish();

    std::cout << "RUN TOP_SCAN" << std::endl;
    // Next, a top-level exclusive scan is performed on the
    // per block histograms.  This is done by a single
    // work group (note global size here is the same as local).
    err = clEnqueueNDRangeKernel(cl.queue(), cl.kernel("top_scan"), 1, NULL,
                                 &local_wsize, &local_wsize, 0, NULL, NULL);

    cl.finish();
#ifdef DEBUG_SORT
    err = clEnqueueReadBuffer(cl.queue(), d_isums, true, 0,
                              num_work_groups * num_digits * sizeof(cl_int),
                              temp_data, 0, NULL, NULL);
    cl.finish();
    for (unsigned i = 0; i < num_work_groups * num_digits; ++i)
      std::cout << "d_isums[" << i << "] = " << temp_data[i] << std::endl;
#endif

    std::cout << "RUN BOTTOM_SCAN" << std::endl;
    // Finally, a bottom-level scan is performed by each block
    // that is seeded with the scanned histograms which rebins,
    // locally scans, then scatters keys to global memory
    err = clEnqueueNDRangeKernel(cl.queue(), cl.kernel("bottom_scan"), 1, NULL,
                                 &global_wsize, &local_wsize, 0, NULL, NULL);

    cl.finish();
#ifdef DEBUG_SORT
    if (even) {
      err = clEnqueueReadBuffer(cl.queue(), d_odata, true, 0, bytes, temp_data,
                              0, NULL, NULL);
      std::cout << "Now even, reading d_odata" << std::endl;
      std::cout << "shift = " << shift << " : " << std::endl;
      for (unsigned int i = 0; i < size - 1; i++)
        std::cout << "keys[" << i << "] = " << temp_data[i] << std::endl;
    }
    else {
      err = clEnqueueReadBuffer(cl.queue(), d_idata, true, 0, bytes, temp_data,
                              0, NULL, NULL);
      std::cout << "Now odd, reading d_idata" << std::endl;
      std::cout << "shift = " << shift << " : " << std::endl;
      for (unsigned int i = 0; i < size - 1; i++)
        std::cout << "keys[" << i << "] = " << temp_data[i] << std::endl;

    }
#endif
  }

  err = cl.finish();
  CL_CHECK_ERROR(err);

  err = clEnqueueReadBuffer(cl.queue(), d_idata, true, 0, bytes, h_odata,
                            0, NULL, NULL);
  CL_CHECK_ERROR(err);
  err = cl.finish();
  CL_CHECK_ERROR(err);

  // If answer is incorrect, stop test and do not report performance
  if (! verifySort(h_odata, size)) {
    std::cerr << "FAIL" << std::endl;
    return -1;
  }
  std::cerr << "PASS" << std::endl;

  return 0;
}
