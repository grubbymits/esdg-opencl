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
 * \file api_device.cpp
 * \brief Devices
 */
#include <iostream>

#include "CL/cl.h"
#include <core/deviceinterface.h>
#include <core/devices/LE1/LE1device.h>
/*

  Coal::LE1Device("1C_1w.xml",  "scalar", 1),                     // 0
  Coal::LE1Device("2C_1w.xml",  "scalar", 2),                     // 1
  Coal::LE1Device("4C_1w.xml",  "scalar", 4),                     // 2
  Coal::LE1Device("8C_1w.xml",  "scalar", 8),                     // 3
  Coal::LE1Device("16C_1w.xml", "scalar", 16),                    // 4

  Coal::LE1Device("1C_2w_1ls_1b.xml", "2w2a1m1ls1b", 1),          // 5
  Coal::LE1Device("1C_2w_2ls_1b.xml", "2w2a1m2ls1b", 1),          // 6
  Coal::LE1Device("1C_2w_2ls_2b.xml", "2w2a1m2ls1b", 1),          // 7
  Coal::LE1Device("2C_2w_1ls_1b.xml", "2w2a1m1ls1b", 2),          // 8
  Coal::LE1Device("2C_2w_2ls_1b.xml", "2w2a1m2ls1b", 2),          // 9
  Coal::LE1Device("2C_2w_2ls_2b.xml", "2w2a1m2ls1b", 2),          // 10
  Coal::LE1Device("4C_2w_1ls_1b.xml", "2w2a1m1ls1b", 4),          // 11
  Coal::LE1Device("4C_2w_2ls_1b.xml", "2w2a1m2ls1b", 4),          // 12
  Coal::LE1Device("4C_2w_2ls_2b.xml", "2w2a1m2ls1b", 4),          // 13
  Coal::LE1Device("8C_2w_1ls_1b.xml", "2w2a1m1ls1b", 8),          // 14
  Coal::LE1Device("8C_2w_2ls_1b.xml", "2w2a1m2ls1b", 8),          // 15
  Coal::LE1Device("8C_2w_2ls_2b.xml", "2w2a1m2ls1b", 8),          // 16
  Coal::LE1Device("16C_2w_1ls_1b.xml", "2w2a1m1ls1b", 16),        // 17
  Coal::LE1Device("16C_2w_2ls_1b.xml", "2w2a1m2ls1b", 16),        // 18
  Coal::LE1Device("16C_2w_2ls_2b.xml", "2w2a1m2ls1b", 16),        // 19

  Coal::LE1Device("1C_3w_1ls_1b.xml", "3w3a1m1ls1b", 1),          // 20
  Coal::LE1Device("1C_3w_2ls_1b.xml", "3w3a1m2ls1b", 1),          // 21
  Coal::LE1Device("1C_3w_2ls_2b.xml", "3w3a1m2ls1b", 1),          // 22
  Coal::LE1Device("2C_3w_1ls_1b.xml", "3w3a1m1ls1b", 2),          // 23
  Coal::LE1Device("2C_3w_2ls_1b.xml", "3w3a1m2ls1b", 2),          // 24
  Coal::LE1Device("2C_3w_2ls_2b.xml", "3w3a1m2ls1b", 2),          // 25
  Coal::LE1Device("4C_3w_1ls_1b.xml", "3w3a1m1ls1b", 4),          // 26
  Coal::LE1Device("4C_3w_2ls_1b.xml", "3w3a1m2ls1b", 4),          // 27
  Coal::LE1Device("4C_3w_2ls_2b.xml", "3w3a1m2ls1b", 4),          // 28
  Coal::LE1Device("8C_3w_1ls_1b.xml", "3w3a1m1ls1b", 8),          // 29
  Coal::LE1Device("8C_3w_2ls_1b.xml", "3w3a1m2ls1b", 8),          // 30
  Coal::LE1Device("8C_3w_2ls_2b.xml", "3w3a1m2ls1b", 8),          // 31
  Coal::LE1Device("16C_3w_1ls_1b.xml", "3w3a1m1ls1b", 16),        // 32
  Coal::LE1Device("16C_3w_2ls_1b.xml", "3w3a1m2ls1b", 16),        // 33
  Coal::LE1Device("16C_3w_2ls_2b.xml", "3w3a1m2ls1b", 16),        // 34

  Coal::LE1Device("1C_4w_1ls_1b.xml", "4w4a1m1ls1b", 1),          // 35
  Coal::LE1Device("1C_4w_2ls_1b.xml", "4w4a1m2ls1b", 1),          // 36
  Coal::LE1Device("1C_4w_2ls_2b.xml", "4w4a1m2ls1b", 1),          // 37
  Coal::LE1Device("2C_4w_1ls_1b.xml", "4w4a1m1ls1b", 2),          // 38
  Coal::LE1Device("2C_4w_2ls_1b.xml", "4w4a1m2ls1b", 2),          // 39
  Coal::LE1Device("2C_4w_2ls_2b.xml", "4w4a1m2ls1b", 2),          // 40
  Coal::LE1Device("4C_4w_1ls_1b.xml", "4w4a1m1ls1b", 4),          // 41
  Coal::LE1Device("4C_4w_2ls_1b.xml", "4w4a1m2ls1b", 4),          // 42
  Coal::LE1Device("4C_4w_2ls_2b.xml", "4w4a1m2ls1b", 4),          // 43
  Coal::LE1Device("8C_4w_1ls_1b.xml", "4w4a1m1ls1b", 8),          // 44
  Coal::LE1Device("8C_4w_2ls_1b.xml", "4w4a1m2ls1b", 8),          // 45
  Coal::LE1Device("8C_4w_2ls_2b.xml", "4w4a1m2ls1b", 8),          // 46
  Coal::LE1Device("16C_4w_1ls_1b.xml", "4w4a1m1ls1b", 16),        // 47
  Coal::LE1Device("16C_4w_2ls_1b.xml", "4w4a1m2ls1b", 16),        // 48
  Coal::LE1Device("16C_4w_2ls_2b.xml", "4w4a1m2ls1b", 16),        // 49
   */
// TODO Add a LE1Device to this and replace the the cpu code
static Coal::LE1Device LE1Devices[TotalLE1Devices] = {
  Coal::LE1Device("1C_1w.xml",          "scalar", 1),                     // 0
  Coal::LE1Device("1C_2w_1ls_1b.xml",   "2w2a1m1ls1b", 1),          // 5
  //Coal::LE0Device("1C_2w_2ls_1b.xml", "2w2a1m2ls1b", 1),          // 6
  //Coal::LE1Device("1C_2w_2ls_2b.xml", "2w2a1m2ls1b", 1),          // 7
  Coal::LE1Device("1C_3w_1ls_1b.xml",   "3w3a1m1ls1b", 1),          // 20
  //Coal::LE1Device("1C_3w_2ls_1b.xml", "3w3a1m2ls1b", 1),          // 21
  //Coal::LE1Device("1C_3w_2ls_2b.xml", "3w3a1m2ls1b", 1),          // 22
  Coal::LE1Device("1C_4w_1ls_1b.xml",   "4w4a1m1ls1b", 1),          // 35
  //Coal::LE1Device("1C_4w_2ls_1b.xml", "4w4a1m2ls1b", 1),          // 36
  //Coal::LE1Device("1C_4w_2ls_2b.xml", "4w4a1m2ls1b", 1),          // 37

  Coal::LE1Device("2C_1w.xml",          "scalar", 2),                     // 1
  Coal::LE1Device("2C_1w_2b.xml",       "scalar", 2),
  Coal::LE1Device("2C_2w_1ls_1b.xml",   "2w2a1m1ls1b", 2),          // 8
  Coal::LE1Device("2C_2w_1ls_2b.xml",   "2w2a1m1ls1b", 2),
  //Coal::LE1Device("2C_2w_2ls_1b.xml", "2w2a1m2ls1b", 2),          // 9
  //Coal::LE1Device("2C_2w_2ls_2b.xml", "2w2a1m2ls1b", 2),          // 10
  Coal::LE1Device("2C_3w_1ls_1b.xml",   "3w3a1m1ls1b", 2),          // 23
  Coal::LE1Device("2C_3w_1ls_2b.xml",   "3w3a1m1ls1b", 2),
  //Coal::LE1Device("2C_3w_2ls_1b.xml", "3w3a1m2ls1b", 2),          // 24
  //Coal::LE1Device("2C_3w_2ls_2b.xml", "3w3a1m2ls1b", 2),          // 25
  Coal::LE1Device("2C_4w_1ls_1b.xml",   "4w4a1m1ls1b", 2),          // 38
  Coal::LE1Device("2C_4w_1ls_2b.xml",   "4w4a1m1ls1b", 2),
  //Coal::LE1Device("2C_4w_2ls_1b.xml", "4w4a1m2ls1b", 2),          // 39
  //Coal::LE1Device("2C_4w_2ls_2b.xml", "4w4a1m2ls1b", 2),          // 40
  //Coal::LE1Device("2C_4w_2ls_4b.xml", "4w4a1m2ls1b", 2),

  Coal::LE1Device("4C_1w.xml",          "scalar", 4),                     // 2
  Coal::LE1Device("4C_1w_2b.xml",       "scalar", 4),
  Coal::LE1Device("4C_1w_4b.xml",       "scalar", 4),
  Coal::LE1Device("4C_2w_1ls_1b.xml",   "2w2a1m1ls1b", 4),          // 11
  Coal::LE1Device("4C_2w_1ls_2b.xml",   "2w2a1m1ls1b", 4),
  Coal::LE1Device("4C_2w_1ls_4b.xml",   "2w2a1m1ls1b", 4),
  Coal::LE1Device("4C_3w_1ls_1b.xml",   "3w3a1m1ls1b", 4),          // 26
  Coal::LE1Device("4C_3w_1ls_2b.xml",   "3w3a1m1ls1b", 4),
  Coal::LE1Device("4C_3w_1ls_4b.xml",   "3w3a1m1ls1b", 4),
  Coal::LE1Device("4C_4w_1ls_1b.xml",   "4w4a1m1ls1b", 4),          // 41
  Coal::LE1Device("4C_4w_1ls_2b.xml",   "4w4a1m1ls1b", 4),
  Coal::LE1Device("4C_4w_1ls_4b.xml",   "4w4a1m1ls1b", 4),

  Coal::LE1Device("8C_1w.xml",          "scalar", 8),                     // 3
  Coal::LE1Device("8C_1w_2b.xml",       "scalar", 8),
  Coal::LE1Device("8C_1w_4b.xml",       "scalar", 8),
  Coal::LE1Device("8C_1w_8b.xml",       "scalar", 8),
  Coal::LE1Device("8C_2w_1ls_1b.xml",   "2w2a1m1ls1b", 8),          // 14
  Coal::LE1Device("8C_2w_1ls_2b.xml",   "2w2a1m1ls1b", 8),          // 14
  Coal::LE1Device("8C_2w_1ls_4b.xml",   "2w2a1m1ls1b", 8),
  Coal::LE1Device("8C_2w_1ls_8b.xml",   "2w2a1m1ls1b", 8),
  Coal::LE1Device("8C_3w_1ls_1b.xml",   "3w3a1m1ls1b", 8),          // 29
  Coal::LE1Device("8C_3w_1ls_2b.xml",   "3w3a1m1ls1b", 8),          // 29
  Coal::LE1Device("8C_3w_1ls_4b.xml",   "3w3a1m1ls1b", 8),
  Coal::LE1Device("8C_3w_1ls_8b.xml",   "3w3a1m1ls1b", 8),
  Coal::LE1Device("8C_4w_1ls_1b.xml",   "4w4a1m1ls1b", 8),          // 44
  Coal::LE1Device("8C_4w_1ls_2b.xml",   "4w4a1m1ls1b", 8),          // 44
  Coal::LE1Device("8C_4w_1ls_4b.xml",   "4w4a1m1ls1b", 8),
  Coal::LE1Device("8C_4w_1ls_8b.xml",   "4w4a1m1ls1b", 8),

  Coal::LE1Device("16C_1w.xml",         "scalar", 16),                    // 4
  Coal::LE1Device("16C_1w_2b.xml",      "scalar", 16),
  Coal::LE1Device("16C_1w_4b.xml",      "scalar", 16),
  Coal::LE1Device("16C_1w_8b.xml",      "scalar", 16),
  Coal::LE1Device("16C_1w_16b.xml",     "scalar", 16),
  Coal::LE1Device("16C_2w_1ls_1b.xml",  "2w2a1m1ls1b", 16),        // 17
  Coal::LE1Device("16C_2w_1ls_2b.xml",  "2w2a1m1ls1b", 16),        // 17
  Coal::LE1Device("16C_2w_1ls_4b.xml",  "2w2a1m1ls1b", 16),        // 17
  Coal::LE1Device("16C_2w_1ls_8b.xml",  "2w2a1m1ls1b", 16),
  Coal::LE1Device("16C_2w_1ls_16b.xml", "2w2a1m1ls1b", 16),
  Coal::LE1Device("16C_3w_1ls_1b.xml",  "3w3a1m1ls1b", 16),        // 32
  Coal::LE1Device("16C_3w_1ls_2b.xml",  "3w3a1m1ls1b", 16),        // 32
  Coal::LE1Device("16C_3w_1ls_4b.xml",  "3w3a1m1ls1b", 16),        // 32
  Coal::LE1Device("16C_3w_1ls_8b.xml",  "3w3a1m1ls1b", 16),
  Coal::LE1Device("16C_3w_1ls_16b.xml", "3w3a1m1ls1b", 16),
  Coal::LE1Device("16C_4w_1ls_1b.xml",  "4w4a1m1ls1b", 16),        // 47
  Coal::LE1Device("16C_4w_1ls_2b.xml",  "4w4a1m1ls1b", 16),        // 47
  Coal::LE1Device("16C_4w_1ls_4b.xml",  "4w4a1m1ls1b", 16),        // 47
  Coal::LE1Device("16C_4w_1ls_8b.xml",  "4w4a1m1ls1b", 16),
  Coal::LE1Device("16C_4w_1ls_16b.xml", "4w4a1m1ls1b", 16)
};

cl_int
clGetDeviceIDs(cl_platform_id   platform,
               cl_device_type   device_type,
               cl_uint          num_entries,
               cl_device_id *   devices,
               cl_uint *        num_devices)
{
#ifdef DBG_API
  std::cerr << "clGetDeviceIDs" << std::endl;
  std::cerr << "Address of num_devices = " << std::hex << num_devices
    << std::endl;
#endif
    if (platform != 0) {
      std::cerr << "INVALID_PLATFORM\n";
        // We currently implement only one platform
        return CL_INVALID_PLATFORM;
    }
    if (num_entries == 0 && devices != 0) {
      std::cerr << "INVALID_VALUE\n";
        return CL_INVALID_VALUE;
    }

    if (num_devices == 0 && devices == 0) {
      std::cerr << "INVALID_VALUE\n";
        return CL_INVALID_VALUE;
    }

    // We currently implement only CPU-based acceleration
    if (device_type & (CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_ACCELERATOR))
    {
        //if(!le1device.init())
          //return CL_DEVICE_NOT_AVAILABLE;
        if (devices) {
          for (unsigned i = 0; ((i < TotalLE1Devices) && (i < num_entries)); ++i) {
#ifdef DBG_API
            std::cerr << "assigning device " << i << " to list" << std::endl;
#endif
            devices[i] = (cl_device_id)&LE1Devices[i];
          }
        }

        if (num_devices) {
          std::cerr << "Writing total devices = " << TotalLE1Devices << std::endl;
            *num_devices = TotalLE1Devices;
        }
    }
    else {
      std::cerr << "DEVICE_NOT_FOUND\n";
        return CL_DEVICE_NOT_FOUND;
    }

#ifdef DBG_API
    std::cerr << "Returning CL_SUCCESS from clGetDeviceIDs" << std::endl;
#endif
    return CL_SUCCESS;
}

cl_int
clGetDeviceInfo(cl_device_id    device,
                cl_device_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret)
{
#ifdef DBG_API
  std::cerr << "clGetDeviceInfo\n";
#endif
    if (!device->isA(Coal::Object::T_Device))
        return CL_INVALID_DEVICE;

    Coal::DeviceInterface *iface = (Coal::DeviceInterface *)device;
    return iface->info(param_name, param_value_size, param_value,
                       param_value_size_ret);
}
