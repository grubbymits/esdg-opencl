/***********************************************************************
 * PathFinder uses dynamic programming to find a path on a 2-D grid from
 * the bottom row to the top row with the smallest accumulated weights,
 * where each step of the path moves straight ahead or diagonally ahead.
 * It iterates row by row, each node picks a neighboring node in the
 * previous row that has the smallest accumulated weight, and adds its
 * own weight to the sum.
 *
 * This kernel uses the technique of ghost zone optimization
 ***********************************************************************/

// Other header files.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#include "OpenCL.h"

using namespace std;

// halo width along one direction when advancing to the next iteration
#define HALO     1
#define STR_SIZE 256
#define DEVICE   0
#define M_SEED   9
#define IN_RANGE(x, min, max)	((x)>=(min) && (x)<=(max))
#define CLAMP_RANGE(x, min, max) x = (x<(min)) ? min : ((x>(max)) ? max : x )
#define MIN(a, b) ((a)<=(b) ? (a) : (b))

// Program variables.
int   rows, cols;
int   Ne = rows * cols;
int*  data;
int** wall;
int*  result;

int* omp_data;
int** omp_wall;
int* omp_result;

int   pyramid_height;

void init(int argc, char** argv)
{
	if (argc == 4)
	{
		cols = atoi(argv[1]);
		rows = atoi(argv[2]);
		pyramid_height = atoi(argv[3]);
	}
	else
	{
		printf("Usage: dynproc row_len col_len pyramid_height\n");
		exit(0);
	}
	data = new int[rows * cols];
	wall = new int*[rows];
        omp_data = new int[rows *cols];
	omp_wall = new int*[rows];

	for (int n = 0; n < rows; n++)
	{
		// wall[n] is set to be the nth row of the data array.
		wall[n] = data + cols * n;
                omp_wall[n] = omp_data + cols * n;
	}
	result = new int[cols];
        omp_result = new int[cols];

	int seed = M_SEED;
	srand(seed);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			wall[i][j] = rand() % 10;
                        omp_wall[i][j] = wall[i][j];
		}
	}
        for (int j = 0; j < cols; j++)
          omp_result[j] = omp_wall[0][j];

#ifdef BENCH_PRINT
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			printf("%d ", wall[i][j]);
		}
		printf("\n");
	}
#endif
}

void run_omp(void) {
  int *src, *dst, *temp;
  int min;

  dst = omp_result;
  src = new int[cols];

  for (int t = 0; t < rows-1; t++) {
    temp = src;
    src = dst;
    dst = temp;
    #pragma omp parallel for private(min)
    for(int n = 0; n < cols; n++){
      min = src[n];
      if (n > 0)
        min = MIN(min, src[n-1]);
      if (n < cols-1)
        min = MIN(min, src[n+1]);
      dst[n] = omp_wall[t+1][n]+min;
    }
  }
}

void fatal(char *s)
{
	fprintf(stderr, "error: %s\n", s);
}

int main(int argc, char** argv)
{
	init(argc, argv);
	
	// Pyramid parameters.
	int borderCols = (pyramid_height) * HALO;
	// int smallBlockCol = ?????? - (pyramid_height) * HALO * 2;
	// int blockCols = cols / smallBlockCol + ((cols % smallBlockCol == 0) ? 0 : 1);

	
	/* printf("pyramidHeight: %d\ngridSize: [%d]\nborder:[%d]\nblockSize: %d\nblockGrid:[%d]\ntargetBlock:[%d]\n",
	   pyramid_height, cols, borderCols, NUMBER_THREADS, blockCols, smallBlockCol); */

	int size = rows * cols;

	// Create and initialize the OpenCL object.
	OpenCL cl(1);  // 1 means to display output (debugging mode).
	cl.init(1);    // 1 means to use GPU. 0 means use CPU.
	cl.gwSize(rows * cols);

	// Create and build the kernel.
	string kn = "dynproc_kernel";  // the kernel name, for future use.
	cl.createKernel(kn);

	// Allocate device memory.
	cl_mem d_gpuWall = clCreateBuffer(cl.ctxt(),
	                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
	                                  sizeof(cl_int)*(size-cols),
	                                  (data + cols),
	                                  NULL);
        std::cout << "Created d_gpuWall Buffer of size " << std::hex
          << (unsigned)(sizeof(cl_int)*(size-cols)) << " bytes" << std::endl;

	cl_mem d_gpuResult[2];

	d_gpuResult[0] = clCreateBuffer(cl.ctxt(),
	                                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	                                sizeof(cl_int)*cols,
	                                data,
	                                NULL);
        std::cout << "Created d_gpuResult Buffer 0 \n";

	d_gpuResult[1] = clCreateBuffer(cl.ctxt(),
	                                CL_MEM_READ_WRITE,
	                                sizeof(cl_int)*cols,
	                                NULL,
	                                NULL);
        std::cout << "Create d_gpuResult Buffer 1\n";

        /*
	cl_int* h_outputBuffer = (cl_int*)malloc(16384*sizeof(cl_int));
	for (int i = 0; i < 16384; i++)
	{
		h_outputBuffer[i] = 0;
	}
	cl_mem d_outputBuffer = clCreateBuffer(cl.ctxt(),
	                                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	                                       sizeof(cl_int)*16384,
	                                       h_outputBuffer,
	                                       NULL);
        std::cout << "Created d_outputBuffer\n";*/

        unsigned numWorkGroups = (rows * cols / cl.localSize());
        std::cout << "numWorkGroups = " << numWorkGroups << std::endl;

        // Create our counters
        cl_uint *h_breakCounterBuffer =
          (cl_uint*)malloc(numWorkGroups * sizeof(cl_uint));

        for (unsigned i = 0; i < numWorkGroups; ++i)
          h_breakCounterBuffer[i] = 0;

        cl_mem d_breakCounterBuffer = clCreateBuffer(cl.ctxt(),
                                                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                                     sizeof(cl_uint) * numWorkGroups,
                                                     h_breakCounterBuffer,
                                                     NULL);
        cl_uint *h_computedCounterBuffer =
          (cl_uint*)malloc(numWorkGroups * sizeof(cl_uint));

        for (unsigned i = 0; i < numWorkGroups; ++i)
          h_computedCounterBuffer[i] = 0;

        cl_mem d_computedCounterBuffer = clCreateBuffer(cl.ctxt(),
                                                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                                     sizeof(cl_uint) * numWorkGroups,
                                                     h_computedCounterBuffer,
                                                     NULL);

        cl_uint *h_workitemCounterBuffer =
          (cl_uint*)malloc(numWorkGroups * sizeof(cl_uint));

        for (unsigned i = 0; i < numWorkGroups; ++i)
          h_workitemCounterBuffer[i] = 0;

        cl_mem d_workitemCounterBuffer =
          clCreateBuffer(cl.ctxt(),
                         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                         sizeof(cl_uint) * numWorkGroups,
                         h_workitemCounterBuffer,
                         NULL);


        cl_uint h_totalWorkitemBuffer = 0;
        cl_mem d_totalWorkitemBuffer =
          clCreateBuffer(cl.ctxt(),
                         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                         sizeof(cl_uint),
                         &h_totalWorkitemBuffer,
                         NULL);

	int src = 1, final_ret = 0;
	for (int t = 0; t < rows - 1; t += pyramid_height)
	{
		int temp = src;
		src = final_ret;
		final_ret = temp;

		// Calculate this for the kernel argument...
		int arg0 = MIN(pyramid_height, rows-t-1);
		int theHalo = HALO;

		// Set the kernel arguments.
                std::cout << "arg0 = " << arg0 << std::endl;
                std::cout << "src = " << src << std::endl;
                std::cout << "final_ret = " << final_ret << std::endl;
		clSetKernelArg(cl.kernel(kn), 0,  sizeof(cl_int),
                               (void*) &arg0);
		clSetKernelArg(cl.kernel(kn), 1,  sizeof(cl_mem),
                               (void*) &d_gpuWall);
		clSetKernelArg(cl.kernel(kn), 2,  sizeof(cl_mem),
                               (void*) &d_gpuResult[src]);
		clSetKernelArg(cl.kernel(kn), 3,  sizeof(cl_mem),
                               (void*) &d_gpuResult[final_ret]);
		clSetKernelArg(cl.kernel(kn), 4,  sizeof(cl_int),
                               (void*) &cols);
		clSetKernelArg(cl.kernel(kn), 5,  sizeof(cl_int),
                               (void*) &rows);
		clSetKernelArg(cl.kernel(kn), 6,  sizeof(cl_int), (void*) &t);
		clSetKernelArg(cl.kernel(kn), 7,  sizeof(cl_int),
                               (void*) &borderCols);
		clSetKernelArg(cl.kernel(kn), 8,  sizeof(cl_int),
                               (void*) &theHalo);
		clSetKernelArg(cl.kernel(kn), 9,
                               sizeof(cl_int) * (cl.localSize()), 0);
		clSetKernelArg(cl.kernel(kn), 10,
                               sizeof(cl_int) * (cl.localSize()), 0);
                /*
		clSetKernelArg(cl.kernel(kn), 11, sizeof(cl_mem),
                               (void*) &d_outputBuffer);*/
                clSetKernelArg(cl.kernel(kn), 11, sizeof(cl_mem),
                               (void*) &d_breakCounterBuffer);
                clSetKernelArg(cl.kernel(kn), 12, sizeof(cl_mem),
                               (void*) &d_computedCounterBuffer);
                clSetKernelArg(cl.kernel(kn), 13, sizeof(cl_mem),
                              (void*) &d_workitemCounterBuffer);
                clSetKernelArg(cl.kernel(kn), 14, sizeof(cl_mem),
                               (void*) &d_totalWorkitemBuffer);
                std::cout << "Set all args\n";
		cl.launch(kn);

        // --------------------------- DEBUG ------------------------------- //
        clEnqueueReadBuffer(cl.q(),
                            d_breakCounterBuffer,
                            CL_TRUE,
                            0,
                            sizeof(cl_uint) * numWorkGroups,
                            h_breakCounterBuffer,
                            0,
                            NULL,
                            NULL);

          for (unsigned group = 0; group < numWorkGroups; ++group)
            std::cout << "breaks in group " << group << " = "
              << (unsigned)h_breakCounterBuffer[group] << std::endl;

        clEnqueueReadBuffer(cl.q(),
                            d_computedCounterBuffer,
                            CL_TRUE,
                            0,
                            sizeof(cl_uint) * numWorkGroups,
                            h_computedCounterBuffer,
                            0,
                            NULL,
                            NULL);

          for (unsigned group = 0; group < numWorkGroups; ++group)
            std::cout << "computed in group " << group << " = "
              << (unsigned)h_computedCounterBuffer[group] << std::endl;

        clEnqueueReadBuffer(cl.q(),
                            d_workitemCounterBuffer,
                            CL_TRUE,
                            0,
                            sizeof(cl_uint) * numWorkGroups,
                            h_workitemCounterBuffer,
                            0,
                            NULL,
                            NULL);

          for (unsigned group = 0; group < numWorkGroups; ++group)
            std::cout << "workitems in group " << group << " = "
              << (unsigned)h_workitemCounterBuffer[group] << std::endl;


          clEnqueueReadBuffer(cl.q(),
                              d_totalWorkitemBuffer,
                              CL_TRUE,
                              0,
                              sizeof(cl_uint),
                              &h_totalWorkitemBuffer,
                              0,
                              NULL,
                              NULL);

          std::cout << "Total workitems so far = " << h_totalWorkitemBuffer
            << std::endl;
        // ------------------------------------------------------------------ //

	}

	// Copy results back to host.
	clEnqueueReadBuffer(cl.q(),                   // The command queue.
	                    d_gpuResult[final_ret],   // The result on the device.
	                    CL_TRUE,                  // Blocking? (ie. Wait at this line until read has finished?)
	                    0,                        // Offset. None in this case.
	                    sizeof(cl_int)*cols,      // Size to copy.
	                    result,                   // The pointer to the memory on the host.
	                    0,                        // Number of events in wait list. Not used.
	                    NULL,                     // Event wait list. Not used.
	                    NULL);                    // Event object for determining status. Not used.
        std::cout << "Enqueued ReadBuffer for d_gpuResult\n";


	// Copy string buffer used for debugging from device to host.
        /*
	clEnqueueReadBuffer(cl.q(),                   // The command queue.
	                    d_outputBuffer,           // Debug buffer on the device.
	                    CL_TRUE,                  // Blocking? (ie. Wait at this line until read has finished?)
	                    0,                        // Offset. None in this case.
	                    sizeof(cl_char)*16384,    // Size to copy.
	                    h_outputBuffer,           // The pointer to the memory on the host.
	                    0,                        // Number of events in wait list. Not used.
	                    NULL,                     // Event wait list. Not used.
	                    NULL);                    // Event object for determining status. Not used.

        std::cout << "Enqueued ReadBuffer for d_outputBuffer\n";
	// Tack a null terminator at the end of the string.
	h_outputBuffer[16383] = '\0';*/

        run_omp();
        int success = 1;
        for (int i = 0; i < cols; i++) {
          if (result[i] != omp_result[i]) {
            printf("FAIL: result[%d] = %d but omp_result[%d] = %d\n",
                  i, result[i], i, omp_result[i]);
            //printf("data = %d, omp_data = %d\n", data[i], omp_data[i]);
            success = 0;
          }
          else
            printf("CORRECT: result[%d] = %d and omp_result[%d] = %d\n",
                  i, result[i], i, omp_result[i]);
        }
        if (success)
          printf("SUCCESS\n");

#ifdef BENCH_PRINT
        printf("data: \n");
	for (int i = 0; i < cols; i++)
		printf("%d\n", data[i]);
	printf("\nresult: \n");
	for (int i = 0; i < cols; i++)
		printf("%d\n", result[i]);
	printf("\n");
#endif

	// Memory cleanup here.
	delete[] data;
	delete[] wall;
	delete[] result;
	delete[] omp_data;
	delete[] omp_wall;
	delete[] omp_result;
	
	return EXIT_SUCCESS;
}
