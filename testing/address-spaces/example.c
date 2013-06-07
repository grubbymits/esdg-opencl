const char* programSource = {"Example"};

int main() {
  // Create local arrays

  // Discover and intialise platforms - don't need
  clGetPlatformIDs(cl_uint num_entries,
                   cl_platform_id* platforms,
                   cl_uint* num_platforms);

  /* Discover and initialise the devices - need just to return the LE1. */
  clGetDeviceIDs(cl_platform_id platform,
                 cl_device_type device_type,
                 cl_uint num_entries,
                 cl_device_id* devices,
                 cl_uint* num_devices);

  /* Create context - probably needed just to hold relevant data together. */
  clCreateContext(const cl_context_properites* properties,
                  cl_uint num_devices,
                  const cl_device_id* devices,
                  void CL_CALLBACK...,
                  void* user_data,
                  cl_int* errcode_ret);

  /* Create a command queue - needed
     The queue will interface to the simulator using the API, it will hold the
     list of kernels to run and will know when the simulator has finished
     running. */
  clCreateCommandQueue(cl_context context,
                       cl_device_id device,
                       cl_command_queue_properties properties,
                       cl_int *errcode_ret);

  /* Create device buffers - needed, and we will only support buffers
     The queue will hold the buffers until we know what the types of the
     arguments, but we will calculate the device pointer address for it */
  clCreateBuffer(cl_context context,
                 cl_mem_flags flags,
                 size_t size,
                 void* host_ptr,
                 cl_int *errcode_ret);

  /* Write host data to device buffers - needed */
  clEnqueueWriteBuffer(cl_command_queue command_queue,
                       cl_mem buffer,
                       cl_bool blocking_write,
                       size_t offset,
                       size_t cb,
                       const void* ptr,
                       cl_uint num_events_in_wait_list,
                       const cl_event* event_wait_list,
                       cl_event* event);

  /* Create the program - needed
     Read the char array in and write out a .cl file. */
  clCreateProgramWithSource(cl_context context,
                            cl_uint count,
                            const char **strings,
                            const size_t *lengths,
                            cl_int *errcode_ret);

  /* Compile the program - needed
     Use the filename from the previous function to compile the kernel into
     bytecode. */
  clBuildProgram(cl_program program,
                 cl_uint num_devices,
                 const cl_device_id* device_list,
                 const char* options,
                 void CALLBACK...,
                 void *user_data);

  /* Create the kernel - needed
     Use the filename to send the program to the preprocessor. The kernel name
     is used to search for the function prototype. */
  clCreateKernel(cl_program program,
                 const char* kernel_name,
                 cl_int *errcode_ret);

  /* Set the kernel arguments - needed
     At this point the argument types are known as well as the actual data and
     their device memory locations, so we can start writing the data out to the
     dram text file. */
  clSetKernelArg(cl_kernel kernel,
                 cl_uint arg_index,
                 size_t arg_size,
                 const void* arg_value);

  /* Enqueue the kernel for execution - needed
     Now we have a kernel bytecode file, the dram text has been written, and the
     global work size is also known. Merge the kernels and create launcher using
     the addresses of the buffers for arguments. */
  clEnqueueNDRangeKernel(cl_command_queue command_queue,
                         cl_kernel kernel,
                         cl_uint work_dim,
                         const size_t* global_work_offset,
                         const size_t* global_work_size,
                         const size_t* local_work_size,
                         cl_uint num_events_in_wait_list,
                         const cl_event* event_wait_list,
                         cl_event* event);

  /* Read the output - needed
     The command queue will check when the simulator finishing running and can
     read the data back in, converting for endianness again. */
  clEnqueueReadBuffer(cl_command_queue command_queue,
                      cl_mem buffer,
                      cl_bool blocking_read,
                      size_t offset,
                      size_t cb,
                      void* ptr,
                      cl_uint num_events_in_wait_list,
                      const cl_event* event_wait_list,
                      cl_event *event);

  return 0;

}
