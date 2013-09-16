/* Tests multi-level for-loops with barriers inside.

   Copyright (c) 2012 Pekka Jääskeläinen / Tampere University of Technology
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

// Enable OpenCL C++ exceptions
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#define WINDOW_SIZE 32
#define WORK_ITEMS 16
#define BUFFER_SIZE (WORK_ITEMS + WINDOW_SIZE)

// without -loop-barriers the BTR result seems more sensible
static char
kernelSourceCode[] = 
"__kernel\n"
"void test_kernel(__global int* input, \n"
"                 __global int* result,\n"
"                 int a) {\n"
" int gid = get_global_id(0);\n"
" int i;\n"
" int j;\n"
" for (i = 0; i < 32; ++i) {\n"
"   result[gid] = input[gid];\n"
"   for (j = 0; j < i; ++j) {\n"
"      result[gid] = input[gid] * input[gid + j];\n"  
"      barrier(CLK_GLOBAL_MEM_FENCE);\n"
"   }\n"
" }\n"
"}\n";

int
main(void)
{
  std::cout << "Begin\n";
    int A[BUFFER_SIZE];
    int R[WORK_ITEMS];
    cl_int err;
    int a = 2;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        A[i] = i;
    }

    for (int i = 0; i < WORK_ITEMS; i++) {
        R[i] = i;
    }

    //try {
        std::vector<cl::Platform> platformList;

        // Pick platform
        cl::Platform::get(&platformList);

        // Pick first platform
        cl_context_properties cprops[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};
        cl::Context context(CL_DEVICE_TYPE_ACCELERATOR, cprops);

        // Query the set of devices attched to the context
        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        cl_int errcode = 0;
        // Create and program from source
        cl::Program::Sources sources(1, std::make_pair(kernelSourceCode, 0));

        cl::Program program(context, sources, &errcode);
        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to create program!\n";
          exit(1);
        }

        // Build program
        errcode = program.build(devices);
        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to build program!\n";
          exit(1);
        }

        // Create buffer for A and copy host contents
        cl::Buffer aBuffer = cl::Buffer(
            context,
            CL_MEM_READ_ONLY, // | CL_MEM_COPY_HOST_PTR,
            BUFFER_SIZE * sizeof(int), NULL, &errcode);

        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to create buffer!\n";
          exit(1);
        }

        // Create buffer for that uses the host ptr C
        cl::Buffer cBuffer = cl::Buffer(
            context, 
            CL_MEM_WRITE_ONLY,
            WORK_ITEMS * sizeof(int),
            NULL, &errcode);

        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to create buffer!" << std::endl;
          exit(1);
        }

        // Create kernel object
        cl::Kernel kernel(program, "test_kernel", &errcode);
        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to create kernel!" << std::endl;
          exit(1);
        }

        // Create command queue
        cl::CommandQueue queue(context, devices[0], 0, &errcode);
        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to create command queue!" << std::endl;
          exit(1);
        }

        errcode = queue.enqueueWriteBuffer(
          aBuffer,
          CL_TRUE,
          0,
          BUFFER_SIZE * sizeof(int),
          A);

        errcode = queue.enqueueWriteBuffer(
          cBuffer,
          CL_TRUE,
          0,
          WORK_ITEMS * sizeof(int),
          R);

        if (errcode != CL_SUCCESS) {
          std::cout << "Failed to enqueue buffer write!" << std::endl;
          exit(1);
        }


        // Set kernel args
        kernel.setArg(0, sizeof(cl_mem), &aBuffer);
        kernel.setArg(1, sizeof(cl_mem), &cBuffer);
        kernel.setArg(2, a);

        std::cout << "Set kernel args" << std::endl;


        // Do the work
        queue.enqueueNDRangeKernel(
            kernel, 
            cl::NullRange, 
            cl::NDRange(WORK_ITEMS),
            cl::NullRange);

        std::cout << "Enqueued range" << std::endl;

        // Map cBuffer to host pointer. This enforces a sync with 
        // the host backing space, remember we choose GPU device.
        cl_int output = queue.enqueueReadBuffer(
            cBuffer,
            CL_TRUE, // block 
            0,
            WORK_ITEMS * sizeof(int),
            R);

        std::cout << "Enqueued ReadBuffer" << std::endl;

        bool ok = true;
        // TODO: validate results
        for (int gid = 0; gid < WORK_ITEMS; gid++) {
            int result;
            int i, j;
            for (i = 0; i < 32; ++i) {
                result = A[gid];
                for (j = 0; j < 31; ++j) {
                    result = A[gid] * A[gid + j];
                }
            }
            if ((int)result != R[gid]) {
                std::cout 
                    << "F(" << gid << ": " << (int)result << " != " << R[gid] 
                    << ")" << std::endl;
                ok = false;
            }
        }
        std::cout << std::endl << "tested results" << std::endl;
        if (ok) {
          std::cout << "Passed" << std::endl;
          return EXIT_SUCCESS; 
        }
        else {
          std::cout << "Failed" << std::endl;
          return EXIT_FAILURE;
        }

        // Finally release our hold on accessing the memory
        err = queue.enqueueUnmapMemObject(
            cBuffer,
            (void *) output);
 
        // There is no need to perform a finish on the final unmap
        // or release any objects as this all happens implicitly with
        // the C++ Wrapper API.
    /*} 
    catch (cl::Error err) {
         std::cerr
             << "ERROR: "
             << err.what()
             << "("
             << err.err()
             << ")"
             << std::endl;

         return EXIT_FAILURE;
    }*/
    std::cout << "Exit\n";

    return EXIT_SUCCESS;
}
