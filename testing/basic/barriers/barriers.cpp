#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <CL/cl.h>
#include "../util/OpenCL.h"
#include "../util/support.h"

const size_t local_wsize = 256;
const size_t global_wsize = 256;

int A[global_wsize];
int B[global_wsize];
int res[global_wsize] = {0};
int divres[global_wsize] = {0};

bool VerifySimpleBarrier() {

  int cpu_res[global_wsize] = {0};
  bool success = true;

  for (unsigned i = 0; i < global_wsize; ++i) {
    cpu_res[i] = A[i] * B[i];
  }
  for (unsigned i = 0; i < global_wsize-1; ++i) {
    cpu_res[i] = cpu_res[i+1] + cpu_res[i];
    if (cpu_res[i] != res[i]) {
      std::cout << "ERROR! idx = " << i << ": cpu_res = " << cpu_res[i]
        << " res = " << res[i] << std::endl;
      success = false;
    }
  }
  return success;
}

bool VerifyBarrierInLoop() {
  int cpu_res[global_wsize] = {0};
  bool success = true;

  for (unsigned i = 0; i < global_wsize; ++i)
    cpu_res[i] = A[i] * B[i];

  int acc[global_wsize];
  for (unsigned i = 0; i < global_wsize - 1; ++i)
    acc[i] = cpu_res[i+1] - res[i];
  
  for (unsigned i = 0; i < global_wsize - 1; ++i) {
    cpu_res[i] = res[i] * acc[i];

    if (cpu_res[i] != res[i]) {
      std::cout << "ERROR! idx = " << i << ": cpu_res = " << cpu_res[i]
        << " res = " << res[i] << std::endl;
      success = false;
    }
  }
  return success;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << "Pass kernel name" << std::endl;
    return -1;
  }

  const char *kernelName = argv[1];

  // Initialise
  OpenCL cl(1);
  cl.getDevices(CL_DEVICE_TYPE_ACCELERATOR);
  cl.buildSource("barrier_test.cl");
  cl.createKernel(kernelName);


  // Initialise data
  for (unsigned i = 0; i < global_wsize; ++i) {
    A[i] = i+1;
    B[i] = global_wsize - i;
  }

  cl_int err = CL_SUCCESS;

  unsigned bytes = sizeof(int) * global_wsize;

  // Create the buffers and copy the host data to them
  cl_mem buf_A = clCreateBuffer(cl.ctxt(),
                                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                bytes, A, &err);
  CL_CHECK_ERROR(err);

  cl_mem buf_B = clCreateBuffer(cl.ctxt(),
                                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                bytes, B, &err);
  CL_CHECK_ERROR(err);

  cl_mem buf_res = clCreateBuffer(cl.ctxt(),
                                  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                  bytes, res, &err);
  CL_CHECK_ERROR(err);


  // Set the kernel arguments for the kernel
  err = cl.setArg(kernelName, 0, sizeof(cl_mem), (void*)&buf_A);
  CL_CHECK_ERROR(err);
  err = cl.setArg(kernelName, 1, sizeof(cl_mem), (void*)&buf_B);
  CL_CHECK_ERROR(err);
  err = cl.setArg(kernelName, 2, sizeof(cl_mem), (void*)&buf_res);
  CL_CHECK_ERROR(err);

  err = clEnqueueNDRangeKernel(cl.queue(), cl.kernel(kernelName), 1, NULL,
                               &global_wsize, &local_wsize, 0, NULL, NULL);

  //err = cl.finish();
  CL_CHECK_ERROR(err);

  err = clEnqueueReadBuffer(cl.queue(), buf_res, true, 0, bytes, res,
                            0, NULL, NULL);
  CL_CHECK_ERROR(err);

  // Verify result
  bool success = false;
  if (strcmp(kernelName, "simple_barrier")  == 0)
    success = VerifySimpleBarrier();
  else if (strcmp(kernelName, "barrier_in_loop") == 0)
    success = VerifyBarrierInLoop();

  if (success)
    std::cout << "\n---------- PASS" << std::endl;
  else
    std::cout << "\n---------- FAIL" << std::endl;

  err = cl.finish();

  return 0;
}
