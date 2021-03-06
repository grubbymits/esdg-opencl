/**********************************************************************
Copyright �2012 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

�	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
�	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/


#ifndef NBODY_H_
#define NBODY_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <SDKCommon.hpp>
#include <SDKApplication.hpp>
#include <SDKCommandArgs.hpp>
#include <SDKFile.hpp>

#define GROUP_SIZE 256

/**
* NBody 
* Class implements OpenCL  NBody sample
* Derived from SDKSample base class
*/

class NBody : public SDKSample
{
    cl_double setupTime;                /**< time taken to setup OpenCL resources and building kernel */
    cl_double kernelTime;               /**< time taken to run kernel and read result back */

    cl_float delT;                      /**< dT (timestep) */
    cl_float espSqr;                    /**< Softening Factor*/
    cl_float* initPos;                  /**< initial position */
    cl_float* initVel;                  /**< initial velocity */
    cl_float* vel;                      /**< Output velocity */
    cl_float* refPos;                   /**< Reference position */
    cl_float* refVel;                   /**< Reference velocity */
    cl_context context;                 /**< CL context */
    cl_device_id *devices;              /**< CL device list */
    cl_mem currPos;                     /**< Position of partciles */
    cl_mem currVel;                     /**< Velocity of partciles */
    cl_mem newPos;                      /**< Position of partciles */
    cl_mem newVel;                      /**< Velocity of partciles */
    cl_command_queue commandQueue;      /**< CL command queue */
    cl_program program;                 /**< CL program */
    cl_kernel kernel;                   /**< CL kernel */
    size_t groupSize;                   /**< Work-Group size */
    cl_int numParticles;
    int iterations;
    bool exchange;                                      /** Exchange current pos/vel with new pos/vel */
    streamsdk::SDKDeviceInfo deviceInfo;                /**< Structure to store device information*/
    streamsdk::KernelWorkGroupInfo kernelInfo;          /**< Structure to store kernel related info */

private:

    float random(float randMax, float randMin);

public:
	bool	isFirstLuanch;
	cl_event glEvent;
    /** 
    * Constructor 
    * Initialize member variables
    * @param name name of sample (string)
    */
    explicit NBody(std::string name)
        : SDKSample(name),
        setupTime(0),
        kernelTime(0),
        delT(0.005f),
        espSqr(500.0f),
        initPos(NULL),
        initVel(NULL),
        vel(NULL),
        refPos(NULL),
        refVel(NULL),
        devices(NULL),
        groupSize(GROUP_SIZE),
        iterations(1),
        exchange(true),
		isFirstLuanch(true),
		glEvent(NULL)
    {
        numParticles = 1024;
        verify = true;
    }

    /** 
    * Constructor 
    * Initialize member variables
    * @param name name of sample (const char*)
    */
    explicit NBody(const char* name)
        : SDKSample(name),
        setupTime(0),
        kernelTime(0),
        delT(0.005f),
        espSqr(500.0f),
        initPos(NULL),
        initVel(NULL),
        vel(NULL),
        refPos(NULL),
        refVel(NULL),
        devices(NULL),
        groupSize(GROUP_SIZE),
        iterations(1),
        exchange(true),
		isFirstLuanch(true),
		glEvent(NULL)
    {
        numParticles = 1024;
        verify = true;
    }

    ~NBody();

    /**
    * Allocate and initialize host memory array with random values
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int setupNBody();

    /**
     * Override from SDKSample, Generate binary image of given kernel 
     * and exit application
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
     */
    int genBinaryImage();

    /**
    * OpenCL related initialisations. 
    * Set up Context, Device list, Command Queue, Memory buffers
    * Build CL kernel program executable
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int setupCL();

    /**
    * Set values for kernels' arguments
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int setupCLKernels();

    /**
    * Enqueue calls to the kernels
    * on to the command queue, wait till end of kernel execution.
    * Get kernel start and end time if timing is enabled
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int runCLKernels();

    /**
    * Reference CPU implementation of Binomial Option
    * for performance comparison
    */
    void nBodyCPUReference();

    /**
    * Override from SDKSample. Print sample stats.
    */
    void printStats();

    /**
    * Override from SDKSample. Initialize 
    * command line parser, add custom options
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int initialize();

    /**
    * Override from SDKSample, adjust width and height 
    * of execution domain, perform all sample setup
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int setup();

    /**
    * Override from SDKSample
    * Run OpenCL NBody
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int run();

    /**
    * Override from SDKSample
    * Cleanup memory allocations
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int cleanup();

    /**
    * Override from SDKSample
    * Verify against reference implementation
    * @return SDK_SUCCESS on success and SDK_FAILURE on failure
    */
    int verifyResults();
};

#endif // NBODY_H_
