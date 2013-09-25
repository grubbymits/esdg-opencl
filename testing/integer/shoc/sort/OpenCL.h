#include <iostream>
#include <stdio.h>
#include <map>
#include <string>
#include <cstring>

// OpenCL header files
#ifdef __APPLE__
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_ext.h>
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>
#include <CL/cl_ext.h>
#endif

class OpenCL
{
public:
	OpenCL(int displayOutput);
	~OpenCL();
	void createKernel(std::string kernelName);
	void getDevices(cl_device_type deviceType, unsigned device_num);
	void buildSource(const char *filename);

        int setArg(std::string name, unsigned num, size_t size, void* data) {
          return clSetKernelArg(kernelArray[name], num, size, data);
        }

        int finish() { return clFinish(command_queue); }

	cl_kernel kernel(std::string kernelName);
	void gwSize(size_t theSize);
	cl_context ctxt();
        cl_device_id devid(unsigned id) { return device_id[id]; }
	cl_command_queue queue();
	void launch(std::string toLaunch);
	size_t localSize();
	
private:
	int                     VERBOSE;           // Display output text from various functions?
	size_t                  lwsize;            // Local work size.
	size_t                  gwsize;            // Global work size.
	cl_int                  ret;               // Holds the error code returned by cl functions.
	cl_platform_id          platform_id[100];
	cl_device_id            device_id[100];
        unsigned                device;
        std::map<std::string, cl_kernel>  kernelArray;
	cl_context              context;
	cl_command_queue        command_queue;
	cl_program              program;
	
};
