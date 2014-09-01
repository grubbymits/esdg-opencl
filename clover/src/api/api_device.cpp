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

static Coal::LE1Device LE1Devices[240] = {
//                cores,  width,  alus, muls, lsus, banks
  Coal::LE1Device( 1,      1,        1,   1,     1,   1),    // 1
  Coal::LE1Device( 1,      2,        1,   1,     1,   1),    // 2
  Coal::LE1Device( 1,      2,        1,   1,     2,   1),    // 3
  Coal::LE1Device( 1,      2,        1,   1,     2,   2),    // 4
  Coal::LE1Device( 1,      2,        1,   2,     1,   1),    // 5
  Coal::LE1Device( 1,      2,        1,   2,     2,   1),    // 6
  Coal::LE1Device( 1,      2,        1,   2,     2,   2),    // 7
  Coal::LE1Device( 1,      2,        2,   1,     1,   1),    // 8
  Coal::LE1Device( 1,      2,        2,   1,     2,   1),    // 9
  Coal::LE1Device( 1,      2,        2,   1,     2,   2),    // 10
  Coal::LE1Device( 1,      2,        2,   2,     1,   1),    // 11
  Coal::LE1Device( 1,      2,        2,   2,     2,   1),    // 12
  Coal::LE1Device( 1,      2,        2,   2,     2,   2),    // 13
  Coal::LE1Device( 1,      4,        2,   1,     1,   1),    // 14
  Coal::LE1Device( 1,      4,        2,   1,     2,   1),    // 15
  Coal::LE1Device( 1,      4,        2,   1,     2,   2),    // 16
  Coal::LE1Device( 1,      4,        2,   2,     1,   1),    // 17
  Coal::LE1Device( 1,      4,        2,   2,     2,   1),    // 18
  Coal::LE1Device( 1,      4,        2,   2,     2,   2),    // 19
  Coal::LE1Device( 1,      4,        3,   1,     1,   1),    // 20
  Coal::LE1Device( 1,      4,        3,   1,     2,   1),    // 21
  Coal::LE1Device( 1,      4,        3,   1,     2,   2),    // 22
  Coal::LE1Device( 1,      4,        3,   2,     1,   1),    // 23
  Coal::LE1Device( 1,      4,        3,   2,     2,   1),    // 24
  Coal::LE1Device( 1,      4,        3,   2,     2,   2),    // 25
  Coal::LE1Device( 1,      4,        4,   1,     1,   1),    // 26
  Coal::LE1Device( 1,      4,        4,   1,     2,   1),    // 27
  Coal::LE1Device( 1,      4,        4,   1,     2,   2),    // 28
  Coal::LE1Device( 1,      4,        4,   2,     1,   1),    // 29
  Coal::LE1Device( 1,      4,        4,   2,     2,   1),    // 30
  Coal::LE1Device( 1,      4,        4,   2,     2,   2),    // 31
  Coal::LE1Device( 2,      1,        1,   1,     1,   1),    // 32
  Coal::LE1Device( 2,      1,        1,   1,     1,   2),    // 33
  Coal::LE1Device( 2,      2,        1,   1,     1,   1),    // 34
  Coal::LE1Device( 2,      2,        1,   1,     1,   2),    // 35
  Coal::LE1Device( 2,      2,        1,   1,     2,   1),    // 36
  Coal::LE1Device( 2,      2,        1,   1,     2,   2),    // 37
  Coal::LE1Device( 2,      2,        1,   1,     2,   4),    // 38
  Coal::LE1Device( 2,      2,        1,   2,     1,   1),    // 39
  Coal::LE1Device( 2,      2,        1,   2,     1,   2),    // 40
  Coal::LE1Device( 2,      2,        1,   2,     2,   1),    // 41
  Coal::LE1Device( 2,      2,        1,   2,     2,   2),    // 42
  Coal::LE1Device( 2,      2,        1,   2,     2,   4),    // 43
  Coal::LE1Device( 2,      2,        2,   1,     1,   1),    // 44
  Coal::LE1Device( 2,      2,        2,   1,     1,   2),    // 45
  Coal::LE1Device( 2,      2,        2,   1,     2,   1),    // 46
  Coal::LE1Device( 2,      2,        2,   1,     2,   2),    // 47
  Coal::LE1Device( 2,      2,        2,   1,     2,   4),    // 48
  Coal::LE1Device( 2,      2,        2,   2,     1,   1),    // 49
  Coal::LE1Device( 2,      2,        2,   2,     1,   2),    // 50
  Coal::LE1Device( 2,      2,        2,   2,     2,   1),    // 51
  Coal::LE1Device( 2,      2,        2,   2,     2,   2),    // 52
  Coal::LE1Device( 2,      2,        2,   2,     2,   4),    // 53
  Coal::LE1Device( 2,      4,        2,   1,     1,   1),    // 54
  Coal::LE1Device( 2,      4,        2,   1,     1,   2),    // 55
  Coal::LE1Device( 2,      4,        2,   1,     2,   1),    // 56
  Coal::LE1Device( 2,      4,        2,   1,     2,   2),    // 57
  Coal::LE1Device( 2,      4,        2,   1,     2,   4),    // 58
  Coal::LE1Device( 2,      4,        2,   2,     1,   1),    // 59
  Coal::LE1Device( 2,      4,        2,   2,     1,   2),    // 60
  Coal::LE1Device( 2,      4,        2,   2,     2,   1),    // 61
  Coal::LE1Device( 2,      4,        2,   2,     2,   2),    // 62
  Coal::LE1Device( 2,      4,        2,   2,     2,   4),    // 63
  Coal::LE1Device( 2,      4,        3,   1,     1,   1),    // 64
  Coal::LE1Device( 2,      4,        3,   1,     1,   2),    // 65
  Coal::LE1Device( 2,      4,        3,   1,     2,   1),    // 66
  Coal::LE1Device( 2,      4,        3,   1,     2,   2),    // 67
  Coal::LE1Device( 2,      4,        3,   1,     2,   4),    // 68
  Coal::LE1Device( 2,      4,        3,   2,     1,   1),    // 69
  Coal::LE1Device( 2,      4,        3,   2,     1,   2),    // 70
  Coal::LE1Device( 2,      4,        3,   2,     2,   1),    // 71
  Coal::LE1Device( 2,      4,        3,   2,     2,   2),    // 72
  Coal::LE1Device( 2,      4,        3,   2,     2,   4),    // 73
  Coal::LE1Device( 2,      4,        4,   1,     1,   1),    // 74
  Coal::LE1Device( 2,      4,        4,   1,     1,   2),    // 75
  Coal::LE1Device( 2,      4,        4,   1,     2,   1),    // 76
  Coal::LE1Device( 2,      4,        4,   1,     2,   2),    // 77
  Coal::LE1Device( 2,      4,        4,   1,     2,   4),    // 78
  Coal::LE1Device( 2,      4,        4,   2,     1,   1),    // 79
  Coal::LE1Device( 2,      4,        4,   2,     1,   2),    // 80
  Coal::LE1Device( 2,      4,        4,   2,     2,   1),    // 81
  Coal::LE1Device( 2,      4,        4,   2,     2,   2),    // 82
  Coal::LE1Device( 2,      4,        4,   2,     2,   4),    // 83
  Coal::LE1Device( 4,      1,        1,   1,     1,   1),    // 84
  Coal::LE1Device( 4,      1,        1,   1,     1,   2),    // 85
  Coal::LE1Device( 4,      1,        1,   1,     1,   4),    // 86
  Coal::LE1Device( 4,      2,        1,   1,     1,   1),    // 87
  Coal::LE1Device( 4,      2,        1,   1,     1,   2),    // 88
  Coal::LE1Device( 4,      2,        1,   1,     1,   4),    // 89
  Coal::LE1Device( 4,      2,        1,   1,     2,   1),    // 90
  Coal::LE1Device( 4,      2,        1,   1,     2,   2),    // 91
  Coal::LE1Device( 4,      2,        1,   1,     2,   4),    // 92
  Coal::LE1Device( 4,      2,        1,   1,     2,   8),    // 93
  Coal::LE1Device( 4,      2,        1,   2,     1,   1),    // 94
  Coal::LE1Device( 4,      2,        1,   2,     1,   2),    // 95
  Coal::LE1Device( 4,      2,        1,   2,     1,   4),    // 96
  Coal::LE1Device( 4,      2,        1,   2,     2,   1),    // 97
  Coal::LE1Device( 4,      2,        1,   2,     2,   2),    // 98
  Coal::LE1Device( 4,      2,        1,   2,     2,   4),    // 99
  Coal::LE1Device( 4,      2,        1,   2,     2,   8),    // 100
  Coal::LE1Device( 4,      2,        2,   1,     1,   1),    // 101
  Coal::LE1Device( 4,      2,        2,   1,     1,   2),    // 102
  Coal::LE1Device( 4,      2,        2,   1,     1,   4),    // 103
  Coal::LE1Device( 4,      2,        2,   1,     2,   1),    // 104
  Coal::LE1Device( 4,      2,        2,   1,     2,   2),    // 105
  Coal::LE1Device( 4,      2,        2,   1,     2,   4),    // 106
  Coal::LE1Device( 4,      2,        2,   1,     2,   8),    // 107
  Coal::LE1Device( 4,      2,        2,   2,     1,   1),    // 108
  Coal::LE1Device( 4,      2,        2,   2,     1,   2),    // 109
  Coal::LE1Device( 4,      2,        2,   2,     1,   4),    // 110
  Coal::LE1Device( 4,      2,        2,   2,     2,   1),    // 111
  Coal::LE1Device( 4,      2,        2,   2,     2,   2),    // 112
  Coal::LE1Device( 4,      2,        2,   2,     2,   4),    // 113
  Coal::LE1Device( 4,      2,        2,   2,     2,   8),    // 114
  Coal::LE1Device( 4,      4,        2,   1,     1,   1),    // 115
  Coal::LE1Device( 4,      4,        2,   1,     1,   2),    // 116
  Coal::LE1Device( 4,      4,        2,   1,     1,   4),    // 117
  Coal::LE1Device( 4,      4,        2,   1,     2,   1),    // 118
  Coal::LE1Device( 4,      4,        2,   1,     2,   2),    // 119
  Coal::LE1Device( 4,      4,        2,   1,     2,   4),    // 120
  Coal::LE1Device( 4,      4,        2,   1,     2,   8),    // 121
  Coal::LE1Device( 4,      4,        2,   2,     1,   1),    // 122
  Coal::LE1Device( 4,      4,        2,   2,     1,   2),    // 123
  Coal::LE1Device( 4,      4,        2,   2,     1,   4),    // 124
  Coal::LE1Device( 4,      4,        2,   2,     2,   1),    // 125
  Coal::LE1Device( 4,      4,        2,   2,     2,   2),    // 126
  Coal::LE1Device( 4,      4,        2,   2,     2,   4),    // 127
  Coal::LE1Device( 4,      4,        2,   2,     2,   8),    // 128
  Coal::LE1Device( 4,      4,        3,   1,     1,   1),    // 129
  Coal::LE1Device( 4,      4,        3,   1,     1,   2),    // 130
  Coal::LE1Device( 4,      4,        3,   1,     1,   4),    // 131
  Coal::LE1Device( 4,      4,        3,   1,     2,   1),    // 132
  Coal::LE1Device( 4,      4,        3,   1,     2,   2),    // 133
  Coal::LE1Device( 4,      4,        3,   1,     2,   4),    // 134
  Coal::LE1Device( 4,      4,        3,   1,     2,   8),    // 135
  Coal::LE1Device( 4,      4,        3,   2,     1,   1),    // 136
  Coal::LE1Device( 4,      4,        3,   2,     1,   2),    // 137
  Coal::LE1Device( 4,      4,        3,   2,     1,   4),    // 138
  Coal::LE1Device( 4,      4,        3,   2,     2,   1),    // 139
  Coal::LE1Device( 4,      4,        3,   2,     2,   2),    // 140
  Coal::LE1Device( 4,      4,        3,   2,     2,   4),    // 141
  Coal::LE1Device( 4,      4,        3,   2,     2,   8),    // 142
  Coal::LE1Device( 4,      4,        4,   1,     1,   1),    // 143
  Coal::LE1Device( 4,      4,        4,   1,     1,   2),    // 144
  Coal::LE1Device( 4,      4,        4,   1,     1,   4),    // 145
  Coal::LE1Device( 4,      4,        4,   1,     2,   1),    // 146
  Coal::LE1Device( 4,      4,        4,   1,     2,   2),    // 147
  Coal::LE1Device( 4,      4,        4,   1,     2,   4),    // 148
  Coal::LE1Device( 4,      4,        4,   1,     2,   8),    // 149
  Coal::LE1Device( 4,      4,        4,   2,     1,   1),    // 150
  Coal::LE1Device( 4,      4,        4,   2,     1,   2),    // 151
  Coal::LE1Device( 4,      4,        4,   2,     1,   4),    // 152
  Coal::LE1Device( 4,      4,        4,   2,     2,   1),    // 153
  Coal::LE1Device( 4,      4,        4,   2,     2,   2),    // 154
  Coal::LE1Device( 4,      4,        4,   2,     2,   4),    // 155
  Coal::LE1Device( 4,      4,        4,   2,     2,   8),    // 156
  Coal::LE1Device( 8,      1,        1,   1,     1,   1),    // 157
  Coal::LE1Device( 8,      1,        1,   1,     1,   2),    // 158
  Coal::LE1Device( 8,      1,        1,   1,     1,   4),    // 159
  Coal::LE1Device( 8,      1,        1,   1,     1,   8),    // 160
  Coal::LE1Device( 8,      2,        1,   1,     1,   1),    // 161
  Coal::LE1Device( 8,      2,        1,   1,     1,   2),    // 162
  Coal::LE1Device( 8,      2,        1,   1,     1,   4),    // 163
  Coal::LE1Device( 8,      2,        1,   1,     1,   8),    // 164
  Coal::LE1Device( 8,      2,        1,   1,     2,   1),    // 165
  Coal::LE1Device( 8,      2,        1,   1,     2,   2),    // 166
  Coal::LE1Device( 8,      2,        1,   1,     2,   4),    // 167
  Coal::LE1Device( 8,      2,        1,   1,     2,   8),    // 168
  Coal::LE1Device( 8,      2,        1,   2,     1,   1),    // 169
  Coal::LE1Device( 8,      2,        1,   2,     1,   2),    // 170
  Coal::LE1Device( 8,      2,        1,   2,     1,   4),    // 171
  Coal::LE1Device( 8,      2,        1,   2,     1,   8),    // 172
  Coal::LE1Device( 8,      2,        1,   2,     2,   1),    // 173
  Coal::LE1Device( 8,      2,        1,   2,     2,   2),    // 174
  Coal::LE1Device( 8,      2,        1,   2,     2,   4),    // 175
  Coal::LE1Device( 8,      2,        1,   2,     2,   8),    // 176
  Coal::LE1Device( 8,      2,        2,   1,     1,   1),    // 177
  Coal::LE1Device( 8,      2,        2,   1,     1,   2),    // 178
  Coal::LE1Device( 8,      2,        2,   1,     1,   4),    // 179
  Coal::LE1Device( 8,      2,        2,   1,     1,   8),    // 180
  Coal::LE1Device( 8,      2,        2,   1,     2,   1),    // 181
  Coal::LE1Device( 8,      2,        2,   1,     2,   2),    // 182
  Coal::LE1Device( 8,      2,        2,   1,     2,   4),    // 183
  Coal::LE1Device( 8,      2,        2,   1,     2,   8),    // 184
  Coal::LE1Device( 8,      2,        2,   2,     1,   1),    // 185
  Coal::LE1Device( 8,      2,        2,   2,     1,   2),    // 186
  Coal::LE1Device( 8,      2,        2,   2,     1,   4),    // 187
  Coal::LE1Device( 8,      2,        2,   2,     1,   8),    // 188
  Coal::LE1Device( 8,      2,        2,   2,     2,   1),    // 189
  Coal::LE1Device( 8,      2,        2,   2,     2,   2),    // 190
  Coal::LE1Device( 8,      2,        2,   2,     2,   4),    // 191
  Coal::LE1Device( 8,      2,        2,   2,     2,   8),    // 192
  Coal::LE1Device( 8,      4,        2,   1,     1,   1),    // 193
  Coal::LE1Device( 8,      4,        2,   1,     1,   2),    // 194
  Coal::LE1Device( 8,      4,        2,   1,     1,   4),    // 195
  Coal::LE1Device( 8,      4,        2,   1,     1,   8),    // 196
  Coal::LE1Device( 8,      4,        2,   1,     2,   1),    // 197
  Coal::LE1Device( 8,      4,        2,   1,     2,   2),    // 198
  Coal::LE1Device( 8,      4,        2,   1,     2,   4),    // 199
  Coal::LE1Device( 8,      4,        2,   1,     2,   8),    // 200
  Coal::LE1Device( 8,      4,        2,   2,     1,   1),    // 201
  Coal::LE1Device( 8,      4,        2,   2,     1,   2),    // 202
  Coal::LE1Device( 8,      4,        2,   2,     1,   4),    // 203
  Coal::LE1Device( 8,      4,        2,   2,     1,   8),    // 204
  Coal::LE1Device( 8,      4,        2,   2,     2,   1),    // 205
  Coal::LE1Device( 8,      4,        2,   2,     2,   2),    // 206
  Coal::LE1Device( 8,      4,        2,   2,     2,   4),    // 207
  Coal::LE1Device( 8,      4,        2,   2,     2,   8),    // 208
  Coal::LE1Device( 8,      4,        3,   1,     1,   1),    // 209
  Coal::LE1Device( 8,      4,        3,   1,     1,   2),    // 210
  Coal::LE1Device( 8,      4,        3,   1,     1,   4),    // 211
  Coal::LE1Device( 8,      4,        3,   1,     1,   8),    // 212
  Coal::LE1Device( 8,      4,        3,   1,     2,   1),    // 213
  Coal::LE1Device( 8,      4,        3,   1,     2,   2),    // 214
  Coal::LE1Device( 8,      4,        3,   1,     2,   4),    // 215
  Coal::LE1Device( 8,      4,        3,   1,     2,   8),    // 216
  Coal::LE1Device( 8,      4,        3,   2,     1,   1),    // 217
  Coal::LE1Device( 8,      4,        3,   2,     1,   2),    // 218
  Coal::LE1Device( 8,      4,        3,   2,     1,   4),    // 219
  Coal::LE1Device( 8,      4,        3,   2,     1,   8),    // 220
  Coal::LE1Device( 8,      4,        3,   2,     2,   1),    // 221
  Coal::LE1Device( 8,      4,        3,   2,     2,   2),    // 222
  Coal::LE1Device( 8,      4,        3,   2,     2,   4),    // 223
  Coal::LE1Device( 8,      4,        3,   2,     2,   8),    // 224
  Coal::LE1Device( 8,      4,        4,   1,     1,   1),    // 225
  Coal::LE1Device( 8,      4,        4,   1,     1,   2),    // 226
  Coal::LE1Device( 8,      4,        4,   1,     1,   4),    // 227
  Coal::LE1Device( 8,      4,        4,   1,     1,   8),    // 228
  Coal::LE1Device( 8,      4,        4,   1,     2,   1),    // 229
  Coal::LE1Device( 8,      4,        4,   1,     2,   2),    // 230
  Coal::LE1Device( 8,      4,        4,   1,     2,   4),    // 231
  Coal::LE1Device( 8,      4,        4,   1,     2,   8),    // 232
  Coal::LE1Device( 8,      4,        4,   2,     1,   1),    // 233
  Coal::LE1Device( 8,      4,        4,   2,     1,   2),    // 234
  Coal::LE1Device( 8,      4,        4,   2,     1,   4),    // 235
  Coal::LE1Device( 8,      4,        4,   2,     1,   8),    // 236
  Coal::LE1Device( 8,      4,        4,   2,     2,   1),    // 237
  Coal::LE1Device( 8,      4,        4,   2,     2,   2),    // 238
  Coal::LE1Device( 8,      4,        4,   2,     2,   4),    // 239
  Coal::LE1Device( 8,      4,        4,   2,     2,   8)     // 240
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
