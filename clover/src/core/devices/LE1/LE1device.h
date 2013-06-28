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
 * \file cpu/device.h
 * \brief LE1 device
 */

#ifndef __LE1_DEVICE_H__
#define __LE1_DEVICE_H__

#include "../../deviceinterface.h"

#include <pthread.h>
#include <list>
#include <map>

namespace Coal
{

class MemObject;
class Event;
class Program;
class Kernel;
class LE1Simulator;
class SimulationStats;

typedef std::vector<SimulationStats*> StatsSet;
typedef std::map<std::string, StatsSet*> StatsMap;

/**
 * \brief LE1 device
 *
 * This class is the base of all the LE1-accelerated OpenCL processing. It
 * creates and manages subclasses such as \c Coal::DeviceBuffer,
 * \c Coal::DeviceProgram and \c Coal::DeviceKernel.
 *
 * This class and the aforementioned ones work together to compile and run
 * kernels using the LLVM JIT, manage buffers, provide built-in functions
 * and do all of this in a multithreaded fashion using worker threads.
 *
 * \see \ref events
 */
class LE1Device : public DeviceInterface
{
    public:
        LE1Device();
        ~LE1Device();

        /**
         * \brief Initialize the LE1 device
         *
         * This function creates the worker threads and get information about
         * the host system for the \c numLE1s() and \c cpuMhz functions.
         */
        bool init();

        cl_int info(cl_device_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        DeviceBuffer *createDeviceBuffer(MemObject *buffer, cl_int *rs);
        void incrGlobalBaseAddr(unsigned mem_incr);
        void SaveStats(std::string &Kernel);
        DeviceProgram *createDeviceProgram(Program *program);
        DeviceKernel *createDeviceKernel(Kernel *kernel,
                                         llvm::Function *function);

        cl_int initEventDeviceData(Event *event);
        void freeEventDeviceData(Event *event);

        void pushEvent(Event *event);
        Event *getEvent(bool &stop);
        LE1Simulator* getSimulator() { return Simulator; }

        unsigned int numLE1s() const;   /*!< \brief Number of logical LE1 cores on the system */
        const char* model() { return simulatorModel.c_str(); }
        const char* target() { return compilerTarget.c_str(); }
        float cpuMhz() const;           /*!< \brief Speed of the LE1 in Mhz */
        static std::string sysDir;
        static std::string libDir;
        static std::string incDir;
        static std::string machinesDir;
        static std::string scriptsDir;

    private:
        unsigned int p_cores, p_num_events;
        std::string simulatorModel;
        std::string compilerTarget;
        float p_cpu_mhz;
        pthread_t *p_workers;
        LE1Simulator* Simulator;
        StatsMap ExecutionStats;

        std::list<Event *> p_events;
        pthread_cond_t p_events_cond;
        pthread_mutex_t p_events_mutex;
        bool p_stop, p_initialized;
        // Addresses of work-item attributes
        // TODO now remove these
        //static const unsigned global_size_addr = 0;
        //static const unsigned local_size_addr = 12;
        //static const unsigned num_groups_addr = 24;
        //static const unsigned global_offset_addr = 36;
        // Variables to hold the address of where a new data item can begin.
        unsigned global_base_addr;
        unsigned max_global_addr;
        unsigned current_local_addr;
};

}

#endif
