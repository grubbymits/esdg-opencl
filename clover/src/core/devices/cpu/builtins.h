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
 * \file builtins.h
 * \brief CPU built-in functions
 */
#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include <string>

namespace Coal {
    class CPUKernelWorkGroup;
}

/**
 * \brief Set the current kernel work-group of this thread
 * \param current \c Coal::CPUKernelWorkGroup to be set in \c g_work_group.
 */
void setThreadLocalWorkGroup(Coal::CPUKernelWorkGroup *current);

/**
 * \brief Return the address of a built-in function given its name
 * \param name name of the built-in whose address is requested
 */
void *getBuiltin(const std::string &name);

/**
 * \brief Work-item stacks
 * \see \ref barrier
 * \param size size of the allocated space for stacks
 * \return address of the allocated space for stacks
 */
void *getWorkItemsData(size_t &size);

/**
 * \brief Set work-item stacks
 * \see \ref barrier
 * \param ptr address of allocated space for stacks
 * \param size size of the allocated space for stacks
 */
void setWorkItemsData(void *ptr, size_t size);

/**
 * \brief Increment a n-component vector given a maximum value
 *
 * This function is used to increment a vector for which a set of maximum values
 * each of its element can reach before the next is incremented.
 *
 * For example, if \p dims is \c 3, \p vec starts at <tt>{0, 0, 0}</tt> and
 * \p maxs if <tt>{2, 3, 1}</tt>, repeatedly calling this function with the
 * same vector will produce the following results :
 *
 * \code
 * {0, 0, 1}
 * {0, 1, 0}
 * {0, 1, 1}
 * {0, 2, 0}
 * {0, 2, 1}
 * {0, 3, 0}
 * {0, 3, 1}
 * {1, 0, 0}
 * ...
 * \endcode
 *
 * Until \p vec reaches <tt>{2, 3, 1}</tt>.
 *
 * \param dims number of elements in the vectors
 * \param vec vector whose elements will be incremented
 * \param maxs vector containing a maximum value above which each corresponding
 *             element of \p vec cannot go.
 * \return false if the increment was ok, true if \p vec was already at it's
 *         maximum value and couldn't be further incremented.
 */
template<typename T>
bool incVec(unsigned long dims, T *vec, T *maxs)
{
    bool overflow = false;

    for (unsigned int i=0; i<dims; ++i)
    {
        vec[i] += 1;

        if (vec[i] > maxs[i])
        {
            vec[i] = 0;
            overflow = true;
        }
        else
        {
            overflow = false;
            break;
        }
    }

    return overflow;
}

/**
 * \brief Address of a pixel in an image
 *
 * This function is heavily used when Clover needs to address a pixel or a byte
 * in a rectangular or three-dimensional image or buffer.
 *
 * \param base address of the first pixel in the image (address of the image itself)
 * \param x X coordinate, cannot be bigger or equal to \c width
 * \param y Y coordinate, cannot be bigger or equal to \c height
 * \param z Z coordinate, cannot be bigger or equal to \c depth (1 for 2D arrays)
 * \param row_pitch size in bytes of a row of pixels in the image
 * \param slice_pitch size in bytes of a slice in a 3D array
 * \param bytes_per_pixel bytes per pixel (1 for simple buffers), used when
 *                        coordinates are in pixels and not in bytes.
 */
unsigned char *imageData(unsigned char *base, size_t x, size_t y, size_t z,
                         size_t row_pitch, size_t slice_pitch,
                         unsigned int bytes_per_pixel);

#endif