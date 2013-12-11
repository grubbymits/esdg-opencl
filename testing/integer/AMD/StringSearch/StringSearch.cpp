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

#include <iostream>
#include <algorithm>
#include "StringSearch.hpp"
#include "limits.h"

int StringSearch::initialize()
{
    // Call base class Initialize to get default configuration
    if(this->SDKSample::initialize())
        return SDK_FAILURE;

    streamsdk::Option* iteration_option = new streamsdk::Option;
    CHECK_ALLOCATION(iteration_option, "Memory allocation error.\n");

    iteration_option->_sVersion = "i";
    iteration_option->_lVersion = "iterations";
    iteration_option->_description = "Number of iterations to execute kernel";
    iteration_option->_type = streamsdk::CA_ARG_INT;
    iteration_option->_value = &iterations;

    sampleArgs->AddOption(iteration_option);
    delete iteration_option;

    streamsdk::Option* substr_option = new streamsdk::Option;
    CHECK_ALLOCATION(substr_option, "Memory allocation error.\n");

    substr_option->_sVersion = "s";
    substr_option->_lVersion = "substr";
    substr_option->_description = "Sub string to search";
    substr_option->_type = streamsdk::CA_ARG_STRING;
    substr_option->_value = &subStr;

    sampleArgs->AddOption(substr_option);
    delete substr_option;

    streamsdk::Option* file_option = new streamsdk::Option;
    CHECK_ALLOCATION(file_option, "Memory allocation error.\n");

    file_option->_sVersion = "f";
    file_option->_lVersion = "file";
    file_option->_description = "File name";
    file_option->_type = streamsdk::CA_ARG_STRING;
    file_option->_value = &file;

    sampleArgs->AddOption(file_option);
    delete file_option;

    streamsdk::Option* case_option = new streamsdk::Option;
    CHECK_ALLOCATION(case_option, "Memory allocation error.\n");

    case_option->_sVersion = "c";
    case_option->_lVersion = "sensitive";
    case_option->_description = "Case Sensitive";
    case_option->_type = streamsdk::CA_NO_ARGUMENT;
    case_option->_value = &caseSensitive;

    sampleArgs->AddOption(case_option);
    delete case_option;
    
    return SDK_SUCCESS;
}

int StringSearch::genBinaryImage()
{
    streamsdk::bifData binaryData;
    binaryData.kernelName = std::string("StringSearch_Kernels.cl");
    binaryData.flagsStr = std::string("");
    if(isComplierFlagsSpecified())
        binaryData.flagsFileName = std::string(flags.c_str());

    binaryData.binaryName = std::string(dumpBinary.c_str());
    int status = sampleCommon->generateBinaryImage(binaryData);
    return status;
}

int StringSearch::setupStringSearch()
{
    // Check input text-file specified.
    if(file.length() == 0) 
    {
        std::cout << "\n Error: Input File not specified..." << std::endl;
        return SDK_FAILURE;
    }

    // Read the content of the file
    std::ifstream textFile(file.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if(! textFile.is_open())
    {
        std::cout << "\n Unable to open file: " << file << std::endl;
        return SDK_FAILURE;
    }

    textLength = (cl_uint)(textFile.tellg());
    text = (cl_uchar*)malloc(textLength+1);
    memset(text, 0, textLength+1);
    CHECK_ALLOCATION(text, "Failed to allocate memory! (text)");
    textFile.seekg(0, std::ios::beg);
    if (!textFile.read ((char*)text, textLength)) 
    {
        std::cout << "\n Reading file failed " << std::endl;
        textFile.close();
        return SDK_FAILURE;
    }
    textFile.close();
    
    if(subStr.length() == 0) 
    {
        std::cout << "\nError: Sub-String not specified..." << std::endl;
        return SDK_FAILURE;
    }

    if (textLength < subStr.length())
    {
        std::cout << "\nText size less than search pattern (" << textLength 
                  << " < " << subStr.length() << ")" << std::endl;
        return SDK_FAILURE;
    }
    
    if(!quiet)
        std::cout << "Search Pattern : " << subStr << std::endl;
  
    return SDK_SUCCESS;
}

int StringSearch::setupCL()
{
    cl_int status = 0;
    cl_device_type dType;
    
    if(deviceType.compare("cpu") == 0)
    {
        dType = CL_DEVICE_TYPE_CPU;
    }
    else //deviceType = "gpu" 
    {
        dType = CL_DEVICE_TYPE_ACCELERATOR;
        if(isThereGPU() == false)
        {
            std::cout << "GPU not found. Falling back to CPU device" << std::endl;
            dType = CL_DEVICE_TYPE_CPU;
        }
    }

    // Get platform
    cl_platform_id platform = NULL;
    int retValue = sampleCommon->getPlatform(platform, platformId, isPlatformEnabled());
    CHECK_ERROR(retValue, SDK_SUCCESS, "sampleCommon::getPlatform() failed");

    // Display available devices.
    retValue = sampleCommon->displayDevices(platform, dType);
    CHECK_ERROR(retValue, SDK_SUCCESS, "sampleCommon::displayDevices() failed");

    // If we could find our platform, use it. Otherwise use just available platform.
    cl_context_properties cps[3] = 
    {
        CL_CONTEXT_PLATFORM, 
        (cl_context_properties)platform, 
        0
    };

    context = clCreateContextFromType(cps,
                                      dType,
                                      NULL,
                                      NULL,
                                      &status);
    CHECK_OPENCL_ERROR(status, "clCreateContextFromType() failed.");

    status = sampleCommon->getDevices(context, &devices, deviceId, isDeviceIdEnabled());
    CHECK_ERROR(status, SDK_SUCCESS, "sampleCommon::getDevices() failed");
    
    // Set device info of given deviceId
    retValue = deviceInfo.setDeviceInfo(devices[deviceId]);
    CHECK_ERROR(retValue, SDK_SUCCESS, "SDKDeviceInfo::setDeviceInfo() failed");

    // Create command queue
    commandQueue = clCreateCommandQueue(context,
                                        devices[deviceId],
                                        0,
                                        &status);
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");

    textBuf = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,
        textLength,
        NULL, 
        &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (textBuf)");

    subStrBuf = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,
        subStr.length(),
        NULL, 
        &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (subStrBuf)");

    cl_uint totalSearchPos = textLength - (cl_uint)subStr.length() + 1;
    searchLenPerWG = SEARCH_BYTES_PER_WORKITEM * LOCAL_SIZE;
    workGroupCount = (totalSearchPos + searchLenPerWG - 1) / searchLenPerWG;

    resultCountBuf = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY,
        sizeof(cl_uint) * workGroupCount,
        NULL, 
        &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (resultCountBuf)");

    resultBuf = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY,
        sizeof(cl_uint) * (textLength - subStr.length() + 1),
        NULL,
        &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (resultBuf)");

    availableLocalMemory = (cl_uint)deviceInfo.localMemSize;
    availableLocalMemory -= (sizeof(cl_int) * LOCAL_SIZE * 2);  // substract stack size
    availableLocalMemory -= (int)subStr.length();               // substract local pattern size
    availableLocalMemory -= 256;                                // substract local variables used
    if(subStr.length() > availableLocalMemory)
    {
        std::cout << "\n Available device local memory is not suffient for make a match" << std::endl;
        return SDK_FAILURE;
    }

    // create a CL program using the kernel source 
    streamsdk::buildProgramData buildData;
    buildData.kernelName = std::string("StringSearch_Kernels.cl");
    buildData.devices = devices;
    buildData.deviceId = deviceId;
    buildData.flagsStr = std::string("");

    if(caseSensitive)
        buildData.flagsStr.append(" -DCASE_SENSITIVE");

    if(subStr.length() > 16)
    {
        buildData.flagsStr.append(" -DENABLE_2ND_LEVEL_FILTER");
        enable2ndLevelFilter = true;
    }

    if(isLoadBinaryEnabled())
        buildData.binaryName = std::string(loadBinary.c_str());

    if(isComplierFlagsSpecified())
        buildData.flagsFileName = std::string(flags.c_str());

    retValue = sampleCommon->buildOpenCLProgram(program, context, buildData);
    CHECK_ERROR(retValue, 0, "sampleCommon::buildOpenCLProgram() failed");

    // get kernel object handle for a kernel with the given name 
    //kernelLoadBalance = clCreateKernel(program, "StringSearchLoadBalance", &status);
    //CHECK_OPENCL_ERROR(status, "clCreateKernel(StringSearchLoadBalance) failed.");
    kernelNaive = clCreateKernel(program, "StringSearchNaive", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel(StringSearchNaive) failed.");

    cl_uchar *ptr;
    // Move text data host to device
    status = mapBuffer( textBuf, ptr, textLength, CL_MAP_WRITE);
    CHECK_ERROR(status, SDK_SUCCESS, "Failed to map device buffer.(textBuf)");
    memcpy(ptr, text, textLength);
    status = unmapBuffer(textBuf, ptr);
    CHECK_ERROR(status, SDK_SUCCESS, "Failed to unmap device buffer.(inputBuffer)");

    // Move subStr data host to device
    status = mapBuffer( subStrBuf, ptr, subStr.length(),
                        CL_MAP_WRITE);
    CHECK_ERROR(status, SDK_SUCCESS, "Failed to map device buffer.(subStrBuf)");
    memcpy(ptr, subStr.c_str(), subStr.length());
    status = unmapBuffer(subStrBuf, ptr);
    CHECK_ERROR(status, SDK_SUCCESS, "Failed to unmap device buffer.(inputBuffer)");

    devResults.reserve(textLength - subStr.length() + 1);
    memset(devResults.data(), 0, devResults.capacity() * sizeof(cl_uint));
    
    return SDK_SUCCESS;
}

int StringSearch::setup()
{
    if(iterations < 1)
    {
        std::cout<<"Error, iterations cannot be 0 or negative. Exiting..\n";
        exit(0);
    }
    if(setupStringSearch() != SDK_SUCCESS)
        return SDK_FAILURE;

    // create and initialize timers
    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);

    if(setupCL() != SDK_SUCCESS)
        return SDK_FAILURE;
    
    sampleCommon->stopTimer(timer);
    // Compute setup time
    setupTime = (double)(sampleCommon->readTimer(timer));

    return SDK_SUCCESS;
}

int StringSearch::runCLKernels()
{
    cl_int status;

    status = clSetKernelArg(*kernel, 0, sizeof(cl_mem), (void*)&textBuf); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (textBuf)");

    status = clSetKernelArg(*kernel, 1, sizeof(cl_uint), (void*)&textLength); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (textLength)");

    status = clSetKernelArg(*kernel, 2, sizeof(cl_mem), (void*)&subStrBuf); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (subStrBuf)");

    int length = (int)subStr.length();
    status = clSetKernelArg(*kernel, 3, sizeof(cl_uint), (void*)&(length));
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (subStr.length())");

    status = clSetKernelArg(*kernel, 4, sizeof(cl_mem), (void*)&resultBuf); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (resultBuf)");

    status = clSetKernelArg(*kernel, 5, sizeof(cl_mem), (void*)&resultCountBuf); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (resultCountBuf)");

    status = clSetKernelArg(*kernel, 6, sizeof(cl_int), (void*)&searchLenPerWG); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (searchLenPerWG)");

    status = clSetKernelArg(*kernel, 7, subStr.length(), NULL); 
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (local subStr.length())");

    if(kernelType == KERNEL_LOADBALANCE)
    {
        status = clSetKernelArg(*kernel, 8, (sizeof(cl_int) * LOCAL_SIZE * 2), NULL); 
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (stack)");

        if(enable2ndLevelFilter)
        {
            status = clSetKernelArg(*kernel, 9, (sizeof(cl_int) * LOCAL_SIZE * 2), NULL); 
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (stack2)");
        }
    }

    // Enqueue a kernel run call.
    size_t localThreads = LOCAL_SIZE;
    size_t globalThreads = localThreads * workGroupCount;
    cl_event ndrEvt;
    status = clEnqueueNDRangeKernel(
        commandQueue,
        *kernel,
        1,
        NULL,
        &globalThreads,
        &localThreads,
        0,
        NULL,
        &ndrEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

    status = clFlush(commandQueue);
    CHECK_OPENCL_ERROR(status, "clFlush failed.");

    status = sampleCommon->waitForEventAndRelease(&ndrEvt);
    CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(ndrEvt) Failed");

    return SDK_SUCCESS;
}

int StringSearch::runKernel(std::string kernelName)
{
    std::cout << "\nExecuting " << kernelName << " for " << 
        iterations << " iterations" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    
    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);

    for(int i = 0; i < iterations; i++)
    {
        // Arguments are set and execution call is enqueued on command buffer 
        if(runCLKernels() != SDK_SUCCESS)
            return SDK_FAILURE;
    }

    sampleCommon->stopTimer(timer);
    kernelTime = (double)(sampleCommon->readTimer(timer));

    // Verify Results
    if(verifyResults() != SDK_SUCCESS)
        return SDK_FAILURE;

    // Print performance statistics
    printStats();

    return SDK_SUCCESS;
}

int StringSearch::run()
{
    kernelType = KERNEL_NAIVE;
    kernel = &kernelNaive;

    for(int i = 0; i < 2 && iterations != 1; i++)
    {
        // Arguments are set and execution call is enqueued on command buffer 
        if(runCLKernels() != SDK_SUCCESS)
            return SDK_FAILURE;
    }

    if(subStr.length() == 1)
        std::cout << "\nRun only Naive-Kernel version of String Search for pattern size = 1" << std::endl;

    if (runKernel("Naive-Kernel") != SDK_SUCCESS)
        return SDK_FAILURE;
   /* 
    if(subStr.length() > 1)
    {
        kernelType = KERNEL_LOADBALANCE;
        kernel = &kernelLoadBalance;
        if (runKernel("LoadBalance-Kernel") != SDK_SUCCESS)
            return SDK_FAILURE;
    }*/
    return SDK_SUCCESS;
}

void StringSearch::cpuReferenceImpl()
{
    static bool refImplemented = false;
    if(refImplemented) return;

    cl_uint hlen = textLength;
    cl_uint last = (cl_uint)subStr.length() - 1;
    cl_uint badCharSkip[UCHAR_MAX + 1];

    // Initialize the table with default values
    cl_uint scan = 0;
    for(scan = 0; scan <= UCHAR_MAX; ++scan)
        badCharSkip[scan] = (cl_uint)subStr.length();

    // populate the table with analysis on pattern
    for(scan = 0; scan < last; ++scan)
    {
        if(caseSensitive)
        {
            badCharSkip[subStr[scan]] = last - scan;
        }
        else
        {
            badCharSkip[toupper(subStr[scan])] = last - scan;
            badCharSkip[tolower(subStr[scan])] = last - scan;
        }
    }

    // search the text
    cl_uint curPos = 0;
    while((textLength - curPos) > last)
    {
        int p=last;
        for(scan=(last+curPos); COMPARE(text[scan], subStr[p--]); scan -= 1)
        {
            if (scan == curPos)
            {
                cpuResults.push_back(curPos);
                break;
            }
        }
        curPos += (scan == curPos) ? 1 : badCharSkip[text[last+curPos]];
    }
    refImplemented = true;
}

int StringSearch::verifyResults()
{
    // Read Results Count per workGroup
    cl_uint *ptrCountBuff;
    int status = mapBuffer( resultCountBuf, ptrCountBuff, workGroupCount * sizeof(cl_uint), CL_MAP_READ);
    CHECK_ERROR(status, SDK_SUCCESS, "Failed to map device buffer.(resultCountBuf)");

    // Read the result buffer
    cl_uint *ptrBuff;
    status = mapBuffer( resultBuf, ptrBuff,  (textLength - subStr.length() + 1) * sizeof(cl_uint), CL_MAP_READ);
    CHECK_ERROR(status, SDK_SUCCESS, "Failed to map device buffer.(resultBuf)");

    cl_uint count = ptrCountBuff[0];
    for(cl_uint i=1; i<workGroupCount; ++i) {
        cl_uint found = ptrCountBuff[i];
        if(found > 0) {
            memcpy((ptrBuff + count), (ptrBuff + (i * searchLenPerWG)), found * sizeof(cl_uint));
            count += found;
        }
    }
    std::sort(ptrBuff, ptrBuff+count);

    if(verify)
    {
        // Rreference implementation on host device
        cpuReferenceImpl();

        // compare the results and see if they match 
        bool result = (count == cpuResults.size());
        std::cout << "cpuResults.size = " << cpuResults.size() << std::endl;
        result = result && std::equal (ptrBuff, ptrBuff+count, cpuResults.begin());
        if(result)
        {
            std::cout << "Passed!\n" << std::endl;
            status = SDK_SUCCESS;
        }
        else
        {
            std::cout << "Failed\n" << std::endl;
            status = SDK_FAILURE;
        }
    }

	sampleCommon->printArray<cl_uint>("Number of matches : ", &count, 1, 1);
    if(!quiet)
        sampleCommon->printArray<cl_uint>("Positions : ", ptrBuff, count, 1);

    // un-map resultCountBuf
    int result = unmapBuffer(resultCountBuf, ptrCountBuff);
    CHECK_ERROR(result, SDK_SUCCESS, "Failed to unmap device buffer.(resultCountBuf)");

    // un-map resultBuf
    result = unmapBuffer(resultBuf, ptrBuff);
    CHECK_ERROR(result, SDK_SUCCESS, "Failed to unmap device buffer.(resultBuf)");

    return status;
}

int StringSearch::cleanup()
{
    // Releases OpenCL resources (Context, Memory etc.) 
    cl_int status;

    status = clReleaseMemObject(textBuf);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject(textBuf) failed.");

    status = clReleaseMemObject(subStrBuf);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject(subStrBuf) failed.");

    status = clReleaseMemObject(resultCountBuf);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject(resultCountBuf) failed.");

    status = clReleaseMemObject(resultBuf);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject(resultBuf) failed.");

    //status = clReleaseKernel(kernelLoadBalance);
    //CHECK_OPENCL_ERROR(status, "clReleaseKernel(kernelLoadBalance) failed.");

    status = clReleaseProgram(program);
    CHECK_OPENCL_ERROR(status, "clReleaseProgram(program) failed.");

    status = clReleaseCommandQueue(commandQueue);
    CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue(readKernel) failed.");

    status = clReleaseContext(context);
    CHECK_OPENCL_ERROR(status, "clReleaseContext(context) failed.");

    FREE(devices);
    return SDK_SUCCESS;
}

void StringSearch::printStats()
{
    std::string strArray[4] = 
    {
        "Source Text size (bytes)", 
        "Setup Time(sec)", 
        "Avg. kernel time (sec)", 
        "Bytes/sec"
    };
    std::string stats[4];
    double avgKernelTime = kernelTime / iterations;

    stats[0] = sampleCommon->toString(textLength, std::dec);
    stats[1] = sampleCommon->toString(setupTime, std::dec);
    stats[2] = sampleCommon->toString(avgKernelTime, std::dec);
    stats[3] = sampleCommon->toString((textLength/avgKernelTime), std::dec);

    this->SDKSample::printStats(strArray, stats, 4);
}

template<typename T>
int StringSearch::mapBuffer(cl_mem deviceBuffer, T* &hostPointer, size_t sizeInBytes, cl_map_flags flags)
{
    cl_int status;
    hostPointer = (T*) clEnqueueMapBuffer(commandQueue,
                                     deviceBuffer,
                                     CL_TRUE,
                                     flags,
                                     0,
                                     sizeInBytes,
                                     0,
                                     NULL,
                                     NULL,
                                     &status);
    CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer failed");

    status = clFinish(commandQueue);
    CHECK_OPENCL_ERROR(status, "clFinish failed.");

    return SDK_SUCCESS;
}

int 
StringSearch::unmapBuffer(cl_mem deviceBuffer, void* hostPointer)
{
    cl_int status;
    status = clEnqueueUnmapMemObject(commandQueue, 
                                     deviceBuffer, 
                                     hostPointer, 
                                     0, 
                                     NULL, 
                                     NULL);
    CHECK_OPENCL_ERROR(status, "clEnqueueUnmapMemObject failed");

    status = clFinish(commandQueue);
    CHECK_OPENCL_ERROR(status, "clFinish failed.");

    return SDK_SUCCESS;
}
int main(int argc, char* argv[])
{
    int status = 0;
    // Create StringSearch object
    StringSearch clStringSearch("StringSearch OpenCL sample");

    // Initialization
    if(clStringSearch.initialize() != SDK_SUCCESS)
        return SDK_FAILURE;

    // Parse command line options 
    if(clStringSearch.parseCommandLine(argc, argv) != SDK_SUCCESS)
        return SDK_FAILURE;

    if(clStringSearch.isDumpBinaryEnabled())
        return clStringSearch.genBinaryImage();

    // Setup
    status = clStringSearch.setup();
    if(status != SDK_SUCCESS)
        return (status == SDK_EXPECTED_FAILURE)? SDK_SUCCESS : SDK_FAILURE;

    // Run
    if(clStringSearch.run() != SDK_SUCCESS)
        return SDK_FAILURE;

    // Cleanup resources created
    if(clStringSearch.cleanup() != SDK_SUCCESS)
        return SDK_FAILURE;

    return SDK_SUCCESS;
}
