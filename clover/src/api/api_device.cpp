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

static Coal::LE1Device LE1Devices[194] = {
//                cores,  width,  alus, muls, lsus, banks
  Coal::LE1Device( 1,      1,        1,   1,     1,   1),    // 0
  Coal::LE1Device( 1,      2,        1,   1,     1,   1),    // 1
  Coal::LE1Device( 1,      2,        1,   1,     2,   1),    // 2
  Coal::LE1Device( 1,      2,        1,   1,     2,   2),    // 3
  Coal::LE1Device( 1,      2,        1,   2,     1,   1),    // 4
  Coal::LE1Device( 1,      2,        1,   2,     2,   1),    // 5
  Coal::LE1Device( 1,      2,        1,   2,     2,   2),    // 6
  Coal::LE1Device( 1,      2,        2,   1,     1,   1),    // 7
  Coal::LE1Device( 1,      2,        2,   1,     2,   1),    // 8
  Coal::LE1Device( 1,      2,        2,   1,     2,   2),    // 9
  Coal::LE1Device( 1,      2,        2,   2,     1,   1),    // 10
  Coal::LE1Device( 1,      2,        2,   2,     2,   1),    // 11
  Coal::LE1Device( 1,      2,        2,   2,     2,   2),    // 12
  Coal::LE1Device( 1,      4,        3,   1,     1,   1),    // 13
  Coal::LE1Device( 1,      4,        3,   1,     2,   1),    // 14
  Coal::LE1Device( 1,      4,        3,   1,     2,   2),    // 15
  Coal::LE1Device( 1,      4,        3,   2,     1,   1),    // 16
  Coal::LE1Device( 1,      4,        3,   2,     2,   1),    // 17
  Coal::LE1Device( 1,      4,        3,   2,     2,   2),    // 18
  Coal::LE1Device( 1,      4,        4,   1,     1,   1),    // 19
  Coal::LE1Device( 1,      4,        4,   1,     2,   1),    // 20
  Coal::LE1Device( 1,      4,        4,   1,     2,   2),    // 21
  Coal::LE1Device( 1,      4,        4,   2,     1,   1),    // 22
  Coal::LE1Device( 1,      4,        4,   2,     2,   1),    // 23
  Coal::LE1Device( 1,      4,        4,   2,     2,   2),    // 24
  Coal::LE1Device( 2,      1,        1,   1,     1,   1),    // 25
  Coal::LE1Device( 2,      1,        1,   1,     1,   2),    // 26
  Coal::LE1Device( 2,      2,        1,   1,     1,   1),    // 27
  Coal::LE1Device( 2,      2,        1,   1,     1,   2),    // 28
  Coal::LE1Device( 2,      2,        1,   1,     2,   1),    // 29
  Coal::LE1Device( 2,      2,        1,   1,     2,   2),    // 30
  Coal::LE1Device( 2,      2,        1,   1,     2,   4),    // 31
  Coal::LE1Device( 2,      2,        1,   2,     1,   1),    // 32
  Coal::LE1Device( 2,      2,        1,   2,     1,   2),    // 33
  Coal::LE1Device( 2,      2,        1,   2,     2,   1),    // 34
  Coal::LE1Device( 2,      2,        1,   2,     2,   2),    // 35
  Coal::LE1Device( 2,      2,        1,   2,     2,   4),    // 36
  Coal::LE1Device( 2,      2,        2,   1,     1,   1),    // 37
  Coal::LE1Device( 2,      2,        2,   1,     1,   2),    // 38
  Coal::LE1Device( 2,      2,        2,   1,     2,   1),    // 39
  Coal::LE1Device( 2,      2,        2,   1,     2,   2),    // 40
  Coal::LE1Device( 2,      2,        2,   1,     2,   4),    // 41
  Coal::LE1Device( 2,      2,        2,   2,     1,   1),    // 42
  Coal::LE1Device( 2,      2,        2,   2,     1,   2),    // 43
  Coal::LE1Device( 2,      2,        2,   2,     2,   1),    // 44
  Coal::LE1Device( 2,      2,        2,   2,     2,   2),    // 45
  Coal::LE1Device( 2,      2,        2,   2,     2,   4),    // 46
  Coal::LE1Device( 2,      4,        3,   1,     1,   1),    // 47
  Coal::LE1Device( 2,      4,        3,   1,     1,   2),    // 48
  Coal::LE1Device( 2,      4,        3,   1,     2,   1),    // 49
  Coal::LE1Device( 2,      4,        3,   1,     2,   2),    // 50
  Coal::LE1Device( 2,      4,        3,   1,     2,   4),    // 51
  Coal::LE1Device( 2,      4,        3,   2,     1,   1),    // 52
  Coal::LE1Device( 2,      4,        3,   2,     1,   2),    // 53
  Coal::LE1Device( 2,      4,        3,   2,     2,   1),    // 54
  Coal::LE1Device( 2,      4,        3,   2,     2,   2),    // 55
  Coal::LE1Device( 2,      4,        3,   2,     2,   4),    // 56
  Coal::LE1Device( 2,      4,        4,   1,     1,   1),    // 57
  Coal::LE1Device( 2,      4,        4,   1,     1,   2),    // 58
  Coal::LE1Device( 2,      4,        4,   1,     2,   1),    // 59
  Coal::LE1Device( 2,      4,        4,   1,     2,   2),    // 60
  Coal::LE1Device( 2,      4,        4,   1,     2,   4),    // 61
  Coal::LE1Device( 2,      4,        4,   2,     1,   1),    // 62
  Coal::LE1Device( 2,      4,        4,   2,     1,   2),    // 63
  Coal::LE1Device( 2,      4,        4,   2,     2,   1),    // 64
  Coal::LE1Device( 2,      4,        4,   2,     2,   2),    // 65
  Coal::LE1Device( 2,      4,        4,   2,     2,   4),    // 66
  Coal::LE1Device( 4,      1,        1,   1,     1,   1),    // 67
  Coal::LE1Device( 4,      1,        1,   1,     1,   2),    // 68
  Coal::LE1Device( 4,      1,        1,   1,     1,   4),    // 69
  Coal::LE1Device( 4,      2,        1,   1,     1,   1),    // 70
  Coal::LE1Device( 4,      2,        1,   1,     1,   2),    // 71
  Coal::LE1Device( 4,      2,        1,   1,     1,   4),    // 72
  Coal::LE1Device( 4,      2,        1,   1,     2,   1),    // 73
  Coal::LE1Device( 4,      2,        1,   1,     2,   2),    // 74
  Coal::LE1Device( 4,      2,        1,   1,     2,   4),    // 75
  Coal::LE1Device( 4,      2,        1,   1,     2,   8),    // 76
  Coal::LE1Device( 4,      2,        1,   2,     1,   1),    // 77
  Coal::LE1Device( 4,      2,        1,   2,     1,   2),    // 78
  Coal::LE1Device( 4,      2,        1,   2,     1,   4),    // 79
  Coal::LE1Device( 4,      2,        1,   2,     2,   1),    // 80
  Coal::LE1Device( 4,      2,        1,   2,     2,   2),    // 81
  Coal::LE1Device( 4,      2,        1,   2,     2,   4),    // 82
  Coal::LE1Device( 4,      2,        1,   2,     2,   8),    // 83
  Coal::LE1Device( 4,      2,        2,   1,     1,   1),    // 84
  Coal::LE1Device( 4,      2,        2,   1,     1,   2),    // 85
  Coal::LE1Device( 4,      2,        2,   1,     1,   4),    // 86
  Coal::LE1Device( 4,      2,        2,   1,     2,   1),    // 87
  Coal::LE1Device( 4,      2,        2,   1,     2,   2),    // 88
  Coal::LE1Device( 4,      2,        2,   1,     2,   4),    // 89
  Coal::LE1Device( 4,      2,        2,   1,     2,   8),    // 90
  Coal::LE1Device( 4,      2,        2,   2,     1,   1),    // 91
  Coal::LE1Device( 4,      2,        2,   2,     1,   2),    // 92
  Coal::LE1Device( 4,      2,        2,   2,     1,   4),    // 93
  Coal::LE1Device( 4,      2,        2,   2,     2,   1),    // 94
  Coal::LE1Device( 4,      2,        2,   2,     2,   2),    // 95
  Coal::LE1Device( 4,      2,        2,   2,     2,   4),    // 96
  Coal::LE1Device( 4,      2,        2,   2,     2,   8),    // 97
  Coal::LE1Device( 4,      4,        3,   1,     1,   1),    // 98
  Coal::LE1Device( 4,      4,        3,   1,     1,   2),    // 99
  Coal::LE1Device( 4,      4,        3,   1,     1,   4),    // 100
  Coal::LE1Device( 4,      4,        3,   1,     2,   1),    // 101
  Coal::LE1Device( 4,      4,        3,   1,     2,   2),    // 102
  Coal::LE1Device( 4,      4,        3,   1,     2,   4),    // 103
  Coal::LE1Device( 4,      4,        3,   1,     2,   8),    // 104
  Coal::LE1Device( 4,      4,        3,   2,     1,   1),    // 105
  Coal::LE1Device( 4,      4,        3,   2,     1,   2),    // 106
  Coal::LE1Device( 4,      4,        3,   2,     1,   4),    // 107
  Coal::LE1Device( 4,      4,        3,   2,     2,   1),    // 108
  Coal::LE1Device( 4,      4,        3,   2,     2,   2),    // 109
  Coal::LE1Device( 4,      4,        3,   2,     2,   4),    // 110
  Coal::LE1Device( 4,      4,        3,   2,     2,   8),    // 111
  Coal::LE1Device( 4,      4,        4,   1,     1,   1),    // 112
  Coal::LE1Device( 4,      4,        4,   1,     1,   2),    // 113
  Coal::LE1Device( 4,      4,        4,   1,     1,   4),    // 114
  Coal::LE1Device( 4,      4,        4,   1,     2,   1),    // 115
  Coal::LE1Device( 4,      4,        4,   1,     2,   2),    // 116
  Coal::LE1Device( 4,      4,        4,   1,     2,   4),    // 117
  Coal::LE1Device( 4,      4,        4,   1,     2,   8),    // 118
  Coal::LE1Device( 4,      4,        4,   2,     1,   1),    // 119
  Coal::LE1Device( 4,      4,        4,   2,     1,   2),    // 120
  Coal::LE1Device( 4,      4,        4,   2,     1,   4),    // 121
  Coal::LE1Device( 4,      4,        4,   2,     2,   1),    // 122
  Coal::LE1Device( 4,      4,        4,   2,     2,   2),    // 123
  Coal::LE1Device( 4,      4,        4,   2,     2,   4),    // 124
  Coal::LE1Device( 4,      4,        4,   2,     2,   8),    // 125
  Coal::LE1Device( 8,      1,        1,   1,     1,   1),    // 126
  Coal::LE1Device( 8,      1,        1,   1,     1,   2),    // 127
  Coal::LE1Device( 8,      1,        1,   1,     1,   4),    // 128
  Coal::LE1Device( 8,      1,        1,   1,     1,   8),    // 129
  Coal::LE1Device( 8,      2,        1,   1,     1,   1),    // 130
  Coal::LE1Device( 8,      2,        1,   1,     1,   2),    // 131
  Coal::LE1Device( 8,      2,        1,   1,     1,   4),    // 132
  Coal::LE1Device( 8,      2,        1,   1,     1,   8),    // 133
  Coal::LE1Device( 8,      2,        1,   1,     2,   1),    // 134
  Coal::LE1Device( 8,      2,        1,   1,     2,   2),    // 135
  Coal::LE1Device( 8,      2,        1,   1,     2,   4),    // 136
  Coal::LE1Device( 8,      2,        1,   1,     2,   8),    // 137
  Coal::LE1Device( 8,      2,        1,   2,     1,   1),    // 138
  Coal::LE1Device( 8,      2,        1,   2,     1,   2),    // 139
  Coal::LE1Device( 8,      2,        1,   2,     1,   4),    // 140
  Coal::LE1Device( 8,      2,        1,   2,     1,   8),    // 141
  Coal::LE1Device( 8,      2,        1,   2,     2,   1),    // 142
  Coal::LE1Device( 8,      2,        1,   2,     2,   2),    // 143
  Coal::LE1Device( 8,      2,        1,   2,     2,   4),    // 144
  Coal::LE1Device( 8,      2,        1,   2,     2,   8),    // 145
  Coal::LE1Device( 8,      2,        2,   1,     1,   1),    // 146
  Coal::LE1Device( 8,      2,        2,   1,     1,   2),    // 147
  Coal::LE1Device( 8,      2,        2,   1,     1,   4),    // 148
  Coal::LE1Device( 8,      2,        2,   1,     1,   8),    // 149
  Coal::LE1Device( 8,      2,        2,   1,     2,   1),    // 150
  Coal::LE1Device( 8,      2,        2,   1,     2,   2),    // 151
  Coal::LE1Device( 8,      2,        2,   1,     2,   4),    // 152
  Coal::LE1Device( 8,      2,        2,   1,     2,   8),    // 153
  Coal::LE1Device( 8,      2,        2,   2,     1,   1),    // 154
  Coal::LE1Device( 8,      2,        2,   2,     1,   2),    // 155
  Coal::LE1Device( 8,      2,        2,   2,     1,   4),    // 156
  Coal::LE1Device( 8,      2,        2,   2,     1,   8),    // 157
  Coal::LE1Device( 8,      2,        2,   2,     2,   1),    // 158
  Coal::LE1Device( 8,      2,        2,   2,     2,   2),    // 159
  Coal::LE1Device( 8,      2,        2,   2,     2,   4),    // 160
  Coal::LE1Device( 8,      2,        2,   2,     2,   8),    // 161
  Coal::LE1Device( 8,      4,        3,   1,     1,   1),    // 162
  Coal::LE1Device( 8,      4,        3,   1,     1,   2),    // 163
  Coal::LE1Device( 8,      4,        3,   1,     1,   4),    // 164
  Coal::LE1Device( 8,      4,        3,   1,     1,   8),    // 165
  Coal::LE1Device( 8,      4,        3,   1,     2,   1),    // 166
  Coal::LE1Device( 8,      4,        3,   1,     2,   2),    // 167
  Coal::LE1Device( 8,      4,        3,   1,     2,   4),    // 168
  Coal::LE1Device( 8,      4,        3,   1,     2,   8),    // 169
  Coal::LE1Device( 8,      4,        3,   2,     1,   1),    // 170
  Coal::LE1Device( 8,      4,        3,   2,     1,   2),    // 171
  Coal::LE1Device( 8,      4,        3,   2,     1,   4),    // 172
  Coal::LE1Device( 8,      4,        3,   2,     1,   8),    // 173
  Coal::LE1Device( 8,      4,        3,   2,     2,   1),    // 174
  Coal::LE1Device( 8,      4,        3,   2,     2,   2),    // 175
  Coal::LE1Device( 8,      4,        3,   2,     2,   4),    // 176
  Coal::LE1Device( 8,      4,        3,   2,     2,   8),    // 177
  Coal::LE1Device( 8,      4,        4,   1,     1,   1),    // 178
  Coal::LE1Device( 8,      4,        4,   1,     1,   2),    // 179
  Coal::LE1Device( 8,      4,        4,   1,     1,   4),    // 180
  Coal::LE1Device( 8,      4,        4,   1,     1,   8),    // 181
  Coal::LE1Device( 8,      4,        4,   1,     2,   1),    // 182
  Coal::LE1Device( 8,      4,        4,   1,     2,   2),    // 183
  Coal::LE1Device( 8,      4,        4,   1,     2,   4),    // 184
  Coal::LE1Device( 8,      4,        4,   1,     2,   8),    // 185
  Coal::LE1Device( 8,      4,        4,   2,     1,   1),    // 186
  Coal::LE1Device( 8,      4,        4,   2,     1,   2),    // 187
  Coal::LE1Device( 8,      4,        4,   2,     1,   4),    // 188
  Coal::LE1Device( 8,      4,        4,   2,     1,   8),    // 189
  Coal::LE1Device( 8,      4,        4,   2,     2,   1),    // 190
  Coal::LE1Device( 8,      4,        4,   2,     2,   2),    // 191
  Coal::LE1Device( 8,      4,        4,   2,     2,   4),    // 192
  Coal::LE1Device( 8,      4,        4,   2,     2,   8)     // 193
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
