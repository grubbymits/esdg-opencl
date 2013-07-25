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
 * \file cpu/worker.cpp
 * \brief Code running in the worker threads launched by \c Coal::CPUDevice
 * \sa builtins.cpp
 */

#include "LE1worker.h"
#include "LE1device.h"
#include "LE1buffer.h"
#include "LE1kernel.h"
#include "LE1Simulator.h"
//#include "builtins.h"

#include "../../commandqueue.h"
#include "../../events.h"
#include "../../memobject.h"
#include "../../kernel.h"

#include <sys/mman.h>

#include <cstring>
#include <iostream>

using namespace Coal;

void *worker(void *data)
{
#ifdef DEBUGCL
  std::cerr << "Entering worker in thread " << pthread_self() << std::endl;
#endif
    LE1Device *device = (LE1Device *)data;
    bool stop = false;
    cl_int errcode;
    Event *event;

    // Initialize TLS
    //setWorkItemsData(0, 0);

    while (true)
    {
        event = device->getEvent(stop);

        // Ensure we have a good event and we don't have to stop
        if (stop) break;
        if (!event) continue;

        // Get info about the event and its command queue
        Event::Type t = event->type();
        CommandQueue *queue = 0;
        cl_command_queue_properties queue_props = 0;

        errcode = CL_SUCCESS;

        event->info(CL_EVENT_COMMAND_QUEUE, sizeof(CommandQueue *), &queue, 0);

        if (queue)
            queue->info(CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties),
                        &queue_props, 0);

        if (queue_props & CL_QUEUE_PROFILING_ENABLE)
            event->updateTiming(Event::Start);

        // Execute the action
        switch (t)
        {
            case Event::ReadBuffer:
            case Event::WriteBuffer:
            {
#ifdef DEBUGCL
              std::cerr << "Worker : Event::ReadBuffer or Event::WriteBuffer in\
thread " << pthread_self() << std::endl;
#endif
                ReadWriteBufferEvent *e = (ReadWriteBufferEvent *)event;
                LE1Buffer *buf = (LE1Buffer *)e->buffer()->deviceBuffer(device);
                //char *data = (char *)buf->data();
                // TODO Read the data back from the simulator. Buffer should
                // have an address assigned to it from print_data

                //data += e->offset();

                if (t == Event::ReadBuffer) {
#ifdef DEBUGCL
                  std::cerr << "ReadBuffer\n";
#endif
                  device->getSimulator()->LockAccess();
                  std::memcpy(e->ptr(), buf->data(), e->cb());
                  device->getSimulator()->UnlockAccess();
                }
                    /*
                  if (e->cb() == 1) {
                    unsigned char data;
                    //device->getSimulator()->readCharData(buf->addr(), e->cb(),
                      //                                   &data);
                    std::memcpy(e->ptr(), buf->data(), e->cb());
                  }
                  else {
                    //unsigned int data[e->cb()];
                    //device->getSimulator()->readIntData(buf->addr(), e->cb(),
                      //                                  data);
                    std::memcpy(e->ptr(), buf->data(), e->cb());
                  }
                }*/
                else {
#ifdef DEBUGCL
                  std::cerr << "WriteBuffer: write " << e->cb() << " bytes\n";
#endif
                  device->getSimulator()->LockAccess();
                  char *data = (char *)buf->data();

                  // Ensure we have a good pointer
                  if (e->ptr() == NULL) {
#ifdef DEBUGCL
                    std::cerr << "!ERROR: Bad data pointer\n";
#endif
                    device->getSimulator()->UnlockAccess();
                    errcode = CL_INVALID_HOST_PTR;
                    break;
                  }

                  std::memcpy(data, e->ptr(), e->cb());
                  device->getSimulator()->UnlockAccess();
                }
                break;
            }
            case Event::CopyBuffer:
            {
                CopyBufferEvent *e = (CopyBufferEvent *)event;
                LE1Buffer *src = (LE1Buffer *)e->source()->deviceBuffer(device);
                LE1Buffer *dst = (LE1Buffer *)e->destination()->deviceBuffer(device);
                std::memcpy(dst->data(), src->data(), e->cb());
                break;
            }
            /*
            case Event::ReadBufferRect:
            case Event::WriteBufferRect:
            case Event::CopyBufferRect:
            case Event::ReadImage:
            case Event::WriteImage:
            case Event::CopyImage:
            case Event::CopyBufferToImage:
            case Event::CopyImageToBuffer:
            {
                // src = buffer and dst = mem if note copy
                ReadWriteCopyBufferRectEvent *e = (ReadWriteCopyBufferRectEvent *)event;
                LE1Buffer *src_buf = (LE1Buffer *)e->source()->deviceBuffer(device);

                unsigned char *src = (unsigned char *)src_buf->data();
                unsigned char *dst;

                switch (t)
                {
                    case Event::CopyBufferRect:
                    case Event::CopyImage:
                    case Event::CopyImageToBuffer:
                    case Event::CopyBufferToImage:
                    {
                        CopyBufferRectEvent *cbre = (CopyBufferRectEvent *)e;
                        LE1Buffer *dst_buf =
                            (LE1Buffer *)cbre->destination()->deviceBuffer(device);

                        dst = (unsigned char *)dst_buf->data();
                        break;
                    }
                    default:
                    {
                        // dst = host memory location
                        ReadWriteBufferRectEvent *rwbre = (ReadWriteBufferRectEvent *)e;

                        dst = (unsigned char *)rwbre->ptr();
                    }
                }

                // Iterate over the lines to copy and use memcpy
                for (size_t z=0; z<e->region(2); ++z)
                {
                    for (size_t y=0; y<e->region(1); ++y)
                    {
                        unsigned char *s;
                        unsigned char *d;

                        d = imageData(dst,
                                      e->dst_origin(0),
                                      y + e->dst_origin(1),
                                      z + e->dst_origin(2),
                                      e->dst_row_pitch(),
                                      e->dst_slice_pitch(),
                                      1);

                        s = imageData(src,
                                      e->src_origin(0),
                                      y + e->src_origin(1),
                                      z + e->src_origin(2),
                                      e->src_row_pitch(),
                                      e->src_slice_pitch(),
                                      1);

                        // Copying and image to a buffer may need to add an offset
                        // to the buffer address (its rectangular origin is
                        // always (0, 0, 0)).
                        if (t == Event::CopyBufferToImage)
                        {
                            CopyBufferToImageEvent *cptie = (CopyBufferToImageEvent *)e;
                            s += cptie->offset();
                        }
                        else if (t == Event::CopyImageToBuffer)
                        {
                            CopyImageToBufferEvent *citbe = (CopyImageToBufferEvent *)e;
                            d += citbe->offset();
                        }

                        if (t == Event::WriteBufferRect || t == Event::WriteImage)
                            std::memcpy(s, d, e->region(0)); // Write dest (memory) in src
                        else
                            std::memcpy(d, s, e->region(0)); // Write src (buffer) in dest (memory), or copy the buffers
                    }
                }
                break;
            }*/
            case Event::MapBuffer:
            case Event::MapImage:
#ifdef DEBUGCL
        std::cerr << "Event::MapBuffer or Event::MapImage\n";
#endif
                // All was already done in CPUBuffer::initEventDeviceData()
                break;

            case Event::NativeKernel:
            {
                NativeKernelEvent *e = (NativeKernelEvent *)event;
                void (*func)(void *) = (void (*)(void *))e->function();
                void *args = e->args();

                func(args);

                break;
            }
            case Event::NDRangeKernel:
            case Event::TaskKernel:
            {
#ifdef DEBUGCL
              std::cerr << "Event::NDRangeKernel or Event::TaskKernel\n";
#endif
              // TODO Send code to simulator here
              KernelEvent *e = (KernelEvent*)event;
              LE1KernelEvent *ke = (LE1KernelEvent*)e->deviceData();
              if (!ke->run())
                errcode = CL_INVALID_PROGRAM_EXECUTABLE;
              ke = 0; // Unlocked, don't use anymore

              /*
                KernelEvent *e = (KernelEvent *)event;
                CPUKernelEvent *ke = (CPUKernelEvent *)e:->deviceData();

                // Take an instance
                CPUKernelWorkGroup *instance = ke->takeInstance();
                ke = 0;     // Unlocked, don't use anymore

                if (!instance->run())
                    errcode = CL_INVALID_PROGRAM_EXECUTABLE;

                delete instance;
              */
                break;
            }
            default:
                break;
        }

        // Cleanups
        if (errcode == CL_SUCCESS)
        {
          // TODO Read the data back from the simulator
            bool finished = true;

            if (event->type() == Event::NDRangeKernel ||
                event->type() == Event::TaskKernel)
            {
                LE1KernelEvent *ke = (LE1KernelEvent *)event->deviceData();
                finished = ke->finished();
            }

            if (finished)
            {
                event->setStatus(Event::Complete);

                if (queue_props & CL_QUEUE_PROFILING_ENABLE)
                    event->updateTiming(Event::End);

                // Clean the queue
                if (queue)
                    queue->cleanEvents();
            }
        }
        else
        {
            // The event failed
            event->setStatus((Event::Status)errcode);

            if (queue_props & CL_QUEUE_PROFILING_ENABLE)
                    event->updateTiming(Event::End);
            if (queue)
              queue->cleanEvents();
        }
    }

    // Free mmapped() data if needed
    size_t mapped_size;
    void *mapped_data;// = getWorkItemsData(mapped_size);

    if (mapped_data)
        munmap(mapped_data, mapped_size);

    return 0;
}
