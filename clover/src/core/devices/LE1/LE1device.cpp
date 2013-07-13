/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file devices/LE1/LE1device.cpp
 * \brief LE1 Device
 */

#include "LE1device.h"
#include "LE1buffer.h"
#include "LE1kernel.h"
#include "LE1program.h"
#include "LE1worker.h"
#include "LE1Simulator.h"
//#include "LE1builtins.h"

#include <core/config.h>
#include "../../propertylist.h"
#include "../../commandqueue.h"
#include "../../events.h"
#include "../../memobject.h"
#include "../../kernel.h"
#include "../../program.h"

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace Coal;

// File locations
std::string LE1Device::SysDir = "/opt/esdg-opencl/";
std::string LE1Device::LibDir = LE1Device::SysDir + "lib/";
std::string LE1Device::IncDir = LE1Device::SysDir + "include/";
std::string LE1Device::MachinesDir = LE1Device::SysDir + "machines/";
std::string LE1Device::ScriptsDir = LE1Device::SysDir + "scripts/";

unsigned LE1Device::MaxGlobalAddr = 0xFFFF * 1024;
bool LE1Device::isEnding = false;

LE1Device::LE1Device(const std::string SimModel, const std::string Target,
                     unsigned Cores)
: DeviceInterface(), p_num_events(0), p_workers(0), p_stop(false),
  p_initialized(false)
{
#ifdef DEBUGCL
  std::cerr << "LE1Device::LE1Device at " << std::hex << this << std::endl;
#endif
  Triple = "le1";
  simulatorModel = SimModel;
  compilerTarget = Target;
  NumCores = Cores;
}

// TODO Don't know where I should initialise the number of cores. For now, here.
bool LE1Device::init()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::init\n";
#endif
  if (p_initialized)
    return true;

  pthread_cond_init(&p_events_cond, 0);
  pthread_mutex_init(&p_events_mutex, 0);

  Simulator = new LE1Simulator();

  if(!Simulator->Initialise(simulatorModel)) {
    std::cerr << "ERROR: Failed to initialise simulator!\n";
    return false;
  }

  p_workers = (pthread_t*) std::malloc(sizeof(pthread_t));
  pthread_create(&p_workers[0], 0, &worker, this);

  p_initialized = true;
#ifdef DEBUGCL
  std::cerr << "Leaving LE1Device::init\n";
#endif
  return true;
}

LE1Device::~LE1Device()
{
  if (!p_initialized)
    return;

  pthread_mutex_lock(&p_events_mutex);
  p_stop = true;

  pthread_cond_broadcast(&p_events_cond);
  pthread_mutex_unlock(&p_events_mutex);

  // for (unsigned int i = 0; i < numLE1s(); ++i)
  //  pthread_join(p_workers[i], 0);

  std::free((void*)p_workers);
  pthread_mutex_destroy(&p_events_mutex);
  pthread_cond_destroy(&p_events_cond);

  //std::cout << "\n ---- Kernel Completion Report ----" << std::endl;
  //std::cout << "Machine Configuration: " << simulatorModel << std::endl;
  //std::cout << "Number of cores: " << NumCores << std::endl;


  for (StatsMap::iterator SMI = ExecutionStats.begin(),
       SME = ExecutionStats.end(); SMI != SME; ++SMI) {

    std::cout << "Kernel Stats for " << SMI->first << std::endl;

    unsigned AverageCycles = 0;
    unsigned AverageStalls = 0;
    unsigned AverageIdle = 0;
    unsigned AverageDecodeStalls = 0;
    unsigned AverageBranchesTaken = 0;
    unsigned AverageBranchesNotTaken = 0;
    unsigned KernelIterations = 0;
    for (StatsSet::iterator SI = SMI->second.begin(),
         SE = SMI->second.end(); SI != SE; ++SI) {
      SimulationStats Stats = *SI;
      AverageCycles += Stats.TotalCycles;
      AverageStalls += Stats.Stalls;
      AverageIdle += Stats.IdleCycles;
      AverageDecodeStalls += Stats.DecodeStalls;
      AverageBranchesTaken += Stats.BranchesTaken;
      AverageBranchesNotTaken += Stats.BranchesNotTaken;
      ++KernelIterations;
    }
    AverageCycles /= KernelIterations;
    AverageIdle /= KernelIterations;
    AverageDecodeStalls /= KernelIterations;
    AverageStalls /= KernelIterations;
    AverageBranchesTaken /= KernelIterations;
    AverageBranchesNotTaken /= KernelIterations;

    // Write results to a CSV file
    // NumCores, Total Cycles, Total Stallls, Decode Stalls, Branches
    // If this is the first device to be destructed, add column headers to the
    // files.
    std::ostringstream Line;
    if (!LE1Device::isEnding) {
      Line << "Contexts, Total Cycles, Total Stalls, Decode Stalls," 
        << " Branches Taken, Branches not Taken\n";
    }
    Line << NumCores << ", " << AverageCycles << ", " << AverageStalls << ", "
      << AverageDecodeStalls << ", " << AverageBranchesTaken << ", "
      << AverageBranchesNotTaken << std::endl;

    std::ofstream Results;
    std::string filename = SMI->first;
    filename.append(".csv");
    Results.open(filename.c_str(), std::ios_base::app);
    Results << Line.str();
    Results.close();

    //std::cout << "Average Cycles: " << AverageCycles << std::endl;
    //std::cout << "  of which stalls: "
      //<< AverageStalls << "\n   including " << AverageDecodeStalls
      //<< " decode stalls." << std::endl;
    //std::cout << "Idle Cycles = " << AverageIdle << std::endl << std::endl;
  }

  delete Simulator;

  StatsMap::iterator MapItr = ExecutionStats.begin();
  while (MapItr != ExecutionStats.end())
    ExecutionStats.erase(MapItr++);

  LE1Device::isEnding = true;
}

DeviceBuffer *LE1Device::createDeviceBuffer(MemObject *buffer, cl_int *rs)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::createDeviceBuffer\n";
#endif
  if((global_base_addr + buffer->size()) > LE1Device::MaxGlobalAddr) {
    std::cerr << "Error: Device doesn't have enough free memory to allocate \
      buffer. global_base = " << global_base_addr << ", buffer size = " <<
      buffer->size() << " and max_global = " << LE1Device::MaxGlobalAddr
      << std::endl;
    exit(1);
  }
  else {
    LE1Buffer* NewBuffer = new LE1Buffer(this, buffer, rs);
    incrGlobalBaseAddr(buffer->size());
    return (DeviceBuffer*)NewBuffer;
  }
#ifdef DEBUGCL
  std::cerr << "Leaving LE1Device::createDeviceBuffer\n";
#endif
}

void LE1Device::incrGlobalBaseAddr(unsigned mem_incr) {
  // Round the new pointer to a whole word
  global_base_addr += mem_incr;
  if(global_base_addr & 0x1)
    ++global_base_addr;
  if(global_base_addr & 0x2)
    global_base_addr = global_base_addr + 2;
}

void LE1Device::SaveStats(std::string &Kernel) {
  StatsSet *NewStats = Simulator->GetStats();

  if (ExecutionStats.find(Kernel) == ExecutionStats.end()) {
    ExecutionStats.insert(std::make_pair(Kernel, *NewStats));
  }
  else {
    StatsSet OldSet = ExecutionStats[Kernel];
    OldSet.insert(OldSet.end(), NewStats->begin(), NewStats->end());
  }

  Simulator->ClearStats();
}

DeviceProgram *LE1Device::createDeviceProgram(Program *program)
{
#ifdef DEBUGCL
  std::cerr << "LE1Device::createDeviceProgram\n";
#endif
    return (DeviceProgram *)new LE1Program(this, program);
}

DeviceKernel *LE1Device::createDeviceKernel(Kernel *kernel,
                                            llvm::Function *function)
{
#ifdef DEBUGCL
  std::cerr << "LE1Device::createDeviceKernel in thread " << pthread_self()
    << std::endl;
#endif
    return (DeviceKernel *)new LE1Kernel(this, kernel, function);
}

cl_int LE1Device::initEventDeviceData(Event *event)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::initEventDevice in thread " << pthread_self()
    << std::endl;
#endif
    switch (event->type())
    {
        case Event::MapBuffer:
        {
#ifdef DEBUGCL
          std::cerr << "Event::MapBuffer\n";
#endif
            MapBufferEvent *e = (MapBufferEvent *)event;
            LE1Buffer *buf = (LE1Buffer *)e->buffer()->deviceBuffer(this);
            unsigned char *data = (unsigned char *)buf->data();

            data += e->offset();

            e->setPtr((void *)data);
            break;
        }
        case Event::MapImage:
        {
#ifdef DEBUGCL
          std::cerr << "Event::MapImage\n";
#endif
          /*
            MapImageEvent *e = (MapImageEvent *)event;
            Image2D *image = (Image2D *)e->buffer();
            LE1Buffer *buf = (LE1Buffer *)image->deviceBuffer(this);
            unsigned char *data = (unsigned char *)buf->data();

            data = imageData(data,
                             e->origin(0),
                             e->origin(1),
                             e->origin(2),
                             image->row_pitch(),
                             image->slice_pitch(),
                             image->pixel_size());

            e->setPtr((void *)data);
            e->setRowPitch(image->row_pitch());
            e->setSlicePitch(image->slice_pitch());*/
            break;
        }
        case Event::UnmapMemObject:
            // Nothing do to
            break;

        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
#ifdef DEBUGCL
          std::cerr << "Event::NDRangeKernel or Event::TaskKernel\n";
#endif
          /* TODO Compile the kernel. Coal::KernelEvent commands contain all the
             information needed to run passes and compile the code. */
            // Instantiate the JIT for the LE1 program
            KernelEvent *kernelEvent = (KernelEvent *)event;

            //if (!prog->initJIT())
              //  return CL_INVALID_PROGRAM_EXECUTABLE;

            // Set device-specific data
            LE1KernelEvent *le1_e = new LE1KernelEvent(this, kernelEvent);
            Program *p = (Program *)kernelEvent->kernel()->parent();
            LE1Program *prog = (LE1Program *)p->deviceDependentProgram(this);
            if(!le1_e->createFinalSource(prog))
              return CL_BUILD_PROGRAM_FAILURE;
            kernelEvent->setDeviceData((void *)le1_e);

            break;
        }
        default:
            break;
    }

#ifdef DEBUGCL
  std::cerr << "Leaving LE1Device::initEventDevice\n";
#endif
    return CL_SUCCESS;
}

void LE1Device::freeEventDeviceData(Event *event)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::freeEventDeviceData\n";
#endif
    switch (event->type())
    {
        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
#ifdef DEBUGCL
          std::cerr << "Event::NDRangeKernel or Event::TaskKernel\n";
#endif
            LE1KernelEvent *le1_e = (LE1KernelEvent *)event->deviceData();

            if (le1_e)
                delete le1_e;

        }
        default:
            break;
    }
#ifdef DEBUGCL
    std::cerr << "Leaving LE1Device::freeEventDeviceData\n";
#endif
}

void LE1Device::pushEvent(Event *event)
{
  if (!p_initialized) {
    std::cerr << "ERROR: Trying to pushEvent, but device isn't initalised!!\n";
  }
#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::pushEvent in thread " << pthread_self()
    << std::endl;
#endif
    // Add an event in the list
    pthread_mutex_lock(&p_events_mutex);

    p_events.push_back(event);
    p_num_events++;                 // Way faster than STL list::size() !
#ifdef DEBUGCL
    std::cerr << "p_num_events = " << p_num_events << std::endl;
#endif

    pthread_cond_broadcast(&p_events_cond);
    pthread_mutex_unlock(&p_events_mutex);
#ifdef DEBUGCL
  std::cerr << "Leaving LE1Device::pushEvent\n";
#endif
}

Event *LE1Device::getEvent(bool &stop)
{
  if (!p_initialized) {
    std::cerr << "ERROR: Trying to getEvent, but device isn't initialised!!\n";
    return NULL;
  }

#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::getEvent in thread " << pthread_self()
    << std::endl;
#endif
    // Return the first event in the list, if any. Remove it if it is a
    // single-shot event.

    if (pthread_mutex_lock(&p_events_mutex) != 0) {
      std::cerr << "p_events_mutex lock failed!\n";
      return NULL;
    }

    while (p_num_events == 0 && !p_stop) {
#ifdef DEBUGCL
      std::cerr << "Waiting for p_num_events == 0\n";
#endif
        pthread_cond_wait(&p_events_cond, &p_events_mutex);
    }

    if (p_stop)
    {
        pthread_mutex_unlock(&p_events_mutex);
        stop = true;
        return 0;
    }

    Event *event = p_events.front();

    // If the run of this event will finish it, remove it from the list
    bool last_slot = true;

    if (event->type() == Event::NDRangeKernel ||
        event->type() == Event::TaskKernel)
    {
        LE1KernelEvent *ke = (LE1KernelEvent *)event->deviceData();
        last_slot = ke->reserve();
    }

    if (last_slot)
    {
#ifdef DEBUGCL
      std::cerr << "is last slot in p_events for LE1device\n";
#endif
        p_num_events--;
        p_events.pop_front();
    }

    pthread_mutex_unlock(&p_events_mutex);

#ifdef DEBUGCL
    std::cerr << "Leaving LE1Device::getEvent, returning event at "
      << std::hex << event << std::endl;
#endif
    return event;
}

unsigned int LE1Device::numLE1s() const
{
    return NumCores;
}

float LE1Device::cpuMhz() const
{
    return p_cpu_mhz;
}

// From inner parentheses to outher ones :
//
// sizeof * 8 => 8
// -1         => 7
// 1 << $     => 10000000
// -1         => 01111111
// *2         => 11111110
// +1         => 11111111
//
// A simple way to do this is (1 << (sizeof(type) * 8)) - 1, but it overflows
// the type (for int8, 1 << $ = 100000000 = 256 > 255)
#define TYPE_MAX(type) ((((type)1 << ((sizeof(type) * 8) - 1)) - 1) * 2 + 1)

cl_int LE1Device::info(cl_device_info param_name,
                       size_t param_value_size,
                       void *param_value,
                       size_t *param_value_size_ret) const
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1Device::info\n";
#endif
    void *value = 0;
    size_t value_length = 0;

    union {
        cl_device_type cl_device_type_var;
        cl_uint cl_uint_var;
        size_t size_t_var;
        cl_ulong cl_ulong_var;
        cl_bool cl_bool_var;
        cl_device_fp_config cl_device_fp_config_var;
        cl_device_mem_cache_type cl_device_mem_cache_type_var;
        cl_device_local_mem_type cl_device_local_mem_type_var;
        cl_device_exec_capabilities cl_device_exec_capabilities_var;
        cl_command_queue_properties cl_command_queue_properties_var;
        cl_platform_id cl_platform_id_var;
        size_t work_dims[MAX_WORK_DIMS];
    };

    switch (param_name)
    {
        case CL_DEVICE_TYPE:
            SIMPLE_ASSIGN(cl_device_type, CL_DEVICE_TYPE_ACCELERATOR);
            break;

        case CL_DEVICE_VENDOR_ID:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_MAX_COMPUTE_UNITS:
            SIMPLE_ASSIGN(cl_uint, numLE1s());
            break;

        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
            SIMPLE_ASSIGN(cl_uint, MAX_WORK_DIMS);
            break;

        case CL_DEVICE_MAX_WORK_GROUP_SIZE:
            SIMPLE_ASSIGN(size_t, TYPE_MAX(size_t));
            break;

        case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            for (int i=0; i<MAX_WORK_DIMS; ++i)
            {
                work_dims[i] = TYPE_MAX(size_t);
            }
            value_length = MAX_WORK_DIMS * sizeof(size_t);
            value = &work_dims;
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
            SIMPLE_ASSIGN(cl_uint, 16);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_MAX_CLOCK_FREQUENCY:
            SIMPLE_ASSIGN(cl_uint, cpuMhz() * 1000000);
            break;

        case CL_DEVICE_ADDRESS_BITS:
            SIMPLE_ASSIGN(cl_uint, 32);
            break;

        case CL_DEVICE_MAX_READ_IMAGE_ARGS:
            SIMPLE_ASSIGN(cl_uint, 65536);
            break;

        case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
            SIMPLE_ASSIGN(cl_uint, 65536);
            break;

        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
            SIMPLE_ASSIGN(cl_ulong, 128*1024*1024);
            break;

        case CL_DEVICE_IMAGE2D_MAX_WIDTH:
            SIMPLE_ASSIGN(size_t, 65536);
            break;

        case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
            SIMPLE_ASSIGN(size_t, 65536);
            break;

        case CL_DEVICE_IMAGE3D_MAX_WIDTH:
            SIMPLE_ASSIGN(size_t, 65536);
            break;

        case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
            SIMPLE_ASSIGN(size_t, 65536);
            break;

        case CL_DEVICE_IMAGE3D_MAX_DEPTH:
            SIMPLE_ASSIGN(size_t, 65536);
            break;

        case CL_DEVICE_IMAGE_SUPPORT:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_MAX_PARAMETER_SIZE:
            SIMPLE_ASSIGN(size_t, 65536);
            break;

        case CL_DEVICE_MAX_SAMPLERS:
            SIMPLE_ASSIGN(cl_uint, 16);
            break;

        case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
            SIMPLE_ASSIGN(cl_uint, 16);
            break;

        case CL_DEVICE_SINGLE_FP_CONFIG:
            // TODO: Check what an x86 SSE engine can support.
            SIMPLE_ASSIGN(cl_device_fp_config,
                          CL_FP_DENORM |
                          CL_FP_INF_NAN |
                          CL_FP_ROUND_TO_NEAREST);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            SIMPLE_ASSIGN(cl_device_mem_cache_type,
                          CL_READ_WRITE_CACHE);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
            // TODO: Get this information from the processor
            SIMPLE_ASSIGN(cl_uint, 16);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
            // TODO: Get this information from the processor
            SIMPLE_ASSIGN(cl_ulong, 512*1024*1024);
            break;

        case CL_DEVICE_GLOBAL_MEM_SIZE:
            // TODO: 1 Gio seems to be enough for software acceleration
            SIMPLE_ASSIGN(cl_ulong, 1*1024*1024*1024);
            break;

        case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
            SIMPLE_ASSIGN(cl_ulong, 1*1024*1024*1024);
            break;

        case CL_DEVICE_MAX_CONSTANT_ARGS:
            SIMPLE_ASSIGN(cl_uint, 65536);
            break;

        case CL_DEVICE_LOCAL_MEM_TYPE:
            SIMPLE_ASSIGN(cl_device_local_mem_type, CL_GLOBAL);
            break;

        case CL_DEVICE_LOCAL_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, 1*1024*1024*1024);
            break;

        case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
            SIMPLE_ASSIGN(cl_bool, CL_FALSE);
            break;

        case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
            // TODO
            SIMPLE_ASSIGN(size_t, 1000);        // 1000 nanoseconds = 1 ms
            break;

        case CL_DEVICE_ENDIAN_LITTLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_COMPILER_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_EXECUTION_CAPABILITIES:
            SIMPLE_ASSIGN(cl_device_exec_capabilities, CL_EXEC_KERNEL |
                          CL_EXEC_NATIVE_KERNEL);
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            SIMPLE_ASSIGN(cl_command_queue_properties,
                          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                          CL_QUEUE_PROFILING_ENABLE);
            break;

        case CL_DEVICE_NAME:
            STRING_ASSIGN("LE1");
            break;

        case CL_DEVICE_VENDOR:
            STRING_ASSIGN("Mesa");
            break;

        case CL_DRIVER_VERSION:
            STRING_ASSIGN("" COAL_VERSION);
            break;

        case CL_DEVICE_PROFILE:
            STRING_ASSIGN("FULL_PROFILE");
            break;

        case CL_DEVICE_VERSION:
            STRING_ASSIGN("OpenCL 1.1 Mesa " COAL_VERSION);
            break;

        case CL_DEVICE_EXTENSIONS:
            STRING_ASSIGN("cl_khr_global_int32_base_atomics"
                          " cl_khr_global_int32_extended_atomics"
                          " cl_khr_local_int32_base_atomics"
                          " cl_khr_local_int32_extended_atomics"
                          " cl_khr_byte_addressable_store"

                          " cl_khr_fp64"
                          " cl_khr_int64_base_atomics"
                          " cl_khr_int64_extended_atomics")

            break;

        case CL_DEVICE_PLATFORM:
            SIMPLE_ASSIGN(cl_platform_id, 0);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_HOST_UNIFIED_MEMORY:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
            SIMPLE_ASSIGN(cl_uint, 16);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_OPENCL_C_VERSION:
            STRING_ASSIGN("OpenCL C 1.1 LLVM " LLVM_VERSION);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

#ifdef DEBUGCL
    std::cerr << "Leaving LE1Device::info\n";
#endif
    return CL_SUCCESS;
}
