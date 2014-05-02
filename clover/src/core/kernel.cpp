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
 * \file core/kernel.cpp
 * \brief Kernel
 */

#include "kernel.h"
#include "propertylist.h"
#include "program.h"
#include "memobject.h"
#include "sampler.h"
#include "deviceinterface.h"

#include <string>
#include <iostream>
#include <cstring>
#include <cstdlib>

#include <llvm/Support/Casting.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace Coal;
Kernel::Kernel(Program *program)
: Object(Object::T_Kernel, program), p_has_locals(false)
{
#ifdef DBG_KERNEL
  std::cerr << "Entering Kernel::Kernel\n";
#endif
    // TODO: Say a kernel is attached to the program (that becomes unalterable)

    null_dep.device = 0;
    null_dep.kernel = 0;
    null_dep.function = 0;
    null_dep.module = 0;
    Built = false;
#ifdef DBG_KERNEL
  std::cerr << "Leaving Kernel::Kernel\n";
#endif
}

Kernel::~Kernel()
{
#ifdef DBG_KERNEL
  std::cerr << "Destructing Kernel" << std::endl;
#endif

  while (p_device_dependent.size())
  {
      DeviceDependent &dep = p_device_dependent.back();

      delete dep.kernel;

      p_device_dependent.pop_back();
  }

  for (unsigned i = 0; i < p_args.size(); ++i)
    delete p_args[i];

#ifdef DBG_KERNEL
  std::cerr << "Successfully destructed kernel" << std::endl;
#endif
}

Kernel::Kernel(const Kernel& kernel) : Object(kernel) {
#ifdef DBG_OUTPUT
  std::cout << "Kernel copy constructor" << std::endl;
#endif
}

const Kernel::DeviceDependent &Kernel::deviceDependent(DeviceInterface *device) const
{
    for (size_t i=0; i<p_device_dependent.size(); ++i)
    {
        const DeviceDependent &rs = p_device_dependent[i];

        if (rs.device == device || (!device && p_device_dependent.size() == 1))
            return rs;
    }

    return null_dep;
}

Kernel::DeviceDependent &Kernel::deviceDependent(DeviceInterface *device)
{
    for (size_t i=0; i<p_device_dependent.size(); ++i)
    {
        DeviceDependent &rs = p_device_dependent[i];

        if (rs.device == device || (!device && p_device_dependent.size() == 1))
            return rs;
    }

    return null_dep;
}

cl_int Kernel::addFunction(DeviceInterface *device, llvm::Function *function,
                           llvm::Module *module)
{
    p_name = function->getName();
#ifdef DBG_KERNEL
  std::cerr << "Entering Kernel::addFunction : " << p_name << std::endl;
#endif

    // Add a device dependent
    DeviceDependent dep;

    dep.device = device;
    dep.function = function;
    dep.module = module;

    // Build the arg list of the kernel (or verify it if a previous function
    // was already registered)
    llvm::FunctionType *f = function->getFunctionType();
    bool append = (p_args.size() == 0);

    if (!append && p_args.size() != f->getNumParams()) {
#ifdef DBG_OUTPUT
      std::cout << "!! ERROR: p_args.size != f->getNumParams : "
        << p_args.size() << " ! = " << f->getNumParams()
        << std::endl;
#endif
        return CL_INVALID_KERNEL_DEFINITION;
    }

    for (unsigned int i=0; i<f->getNumParams(); ++i)
    {
        llvm::Type *arg_type = f->getParamType(i);
        Arg::Kind kind = Arg::Invalid;
        Arg::File file = Arg::Private;
        unsigned short vec_dim = 1;

        if (arg_type->isPointerTy())
        {
#ifdef DBG_KERNEL
          std::cerr << "Arg " << i << " is a pointer\n";
#endif
            // It's a pointer, dereference it
            llvm::PointerType *p_type = llvm::cast<llvm::PointerType>(arg_type);

            file = (Arg::File)p_type->getAddressSpace();
            arg_type = p_type->getElementType();

            // If it's a __local argument, we'll have to allocate memory at run time
            if (file == Arg::Local) {
#ifdef DBG_KERNEL
              std::cerr << "Arg " << i << " is a __local pointer\n";
#endif
                p_has_locals = true;
            }

            kind = Arg::Buffer;

            // If it's a struct, get its name
            if (arg_type->isStructTy())
            {
                llvm::StructType *struct_type =
                    llvm::cast<llvm::StructType>(arg_type);
                std::string struct_name = struct_type->getName().str();

                if (struct_name.compare(0, 14, "struct.image2d") == 0)
                {
                    kind = Arg::Image2D;
                    file = Arg::Global;
                }
                else if (struct_name.compare(0, 14, "struct.image3d") == 0)
                {
                    kind = Arg::Image3D;
                    file = Arg::Global;
                }
            }
        }
        else
        {
#ifdef DBG_KERNEL
          std::cerr << "Arg " << i << " is not a pointer\n";
#endif
            if (arg_type->isVectorTy())
            {
                // It's a vector, we need its element's type
                llvm::VectorType *v_type = llvm::cast<llvm::VectorType>(arg_type);

                vec_dim = v_type->getNumElements();
                arg_type = v_type->getElementType();
            }

            // Get type kind
            if (arg_type->isFloatTy())
            {
                kind = Arg::Float;
            }
            else if (arg_type->isDoubleTy())
            {
                kind = Arg::Double;
            }
            else if (arg_type->isIntegerTy())
            {
                llvm::IntegerType *i_type = llvm::cast<llvm::IntegerType>(arg_type);

                if (i_type->getBitWidth() == 8)
                {
                    kind = Arg::Int8;
                }
                else if (i_type->getBitWidth() == 16)
                {
                    kind = Arg::Int16;
                }
                else if (i_type->getBitWidth() == 32)
                {
                    // NOTE: May also be a sampler, check done in setArg
                    kind = Arg::Int32;
                }
                else if (i_type->getBitWidth() == 64)
                {
                    kind = Arg::Int64;
                }
            }
        }

        // Check if we recognized the type
        if (kind == Arg::Invalid) {
#ifdef DBG_OUTPUT
          std::cout << "!! ERROR: Arg kind == Invalid" << std::endl;
#endif
          return CL_INVALID_KERNEL_DEFINITION;
        }

        if (append) {
          // Create arg
          Arg* a = new Arg(vec_dim, file, kind, arg_type);
          p_args.push_back(a);
        }

        // If we also have a function registered, check for signature compliance
        //if (!append && a != p_args[i]) {
//#ifdef DBG_OUTPUT
  //        std::cout << "!! ERROR: !append && a p_args[i]" << std::endl;
//#endif
  //        return CL_INVALID_KERNEL_DEFINITION;
    //    }

        // Append arg if needed
      //  if (append)
        //  p_args.push_back(a);
    }

    dep.kernel = device->createDeviceKernel(this, dep.function);
    p_device_dependent.push_back(dep);
#ifdef DBG_KERNEL
  std::cerr << "Leaving Kernel::addFunction\n";
#endif

    return CL_SUCCESS;
}

llvm::Function *Kernel::function(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.function;
}

cl_int Kernel::setArg(cl_uint index, size_t size, const void *value)
{
#ifdef DBG_KERNEL
  std::cerr << "Entering Kernel::setArg for kernel at " << std::hex << this
    << ", index = " << index << " size = " << size << std::endl;
#endif

    if (index > p_args.size())
        return CL_INVALID_ARG_INDEX;

    Arg *arg = p_args[index];
#ifdef DBG_KERNEL
    std::cerr << "p_args.size = " << p_args.size() << std::endl;
#endif

    // Special case for __local pointers
    if (arg->file() == Arg::Local)
    {
#ifdef DBG_KERNEL
      std::cerr << "Arg::Local\n";
#endif
        if (size == 0)
            return CL_INVALID_ARG_SIZE;

        if (value != 0)
            return CL_INVALID_ARG_VALUE;

        arg->setAllocAtKernelRuntime(size);

        return CL_SUCCESS;
    }

    // Check that size corresponds to the arg type
    size_t arg_size = arg->valueSize();
#ifdef DBG_KERNEL
    std::cerr << "arg_size = " << arg_size << std::endl;
#endif

    /*
    // Special case for samplers (pointers in C++, uint32 in OpenCL).
    if (size == sizeof(cl_sampler) && arg_size == 4 &&
        (*(Object **)value)->isA(T_Sampler))
    {
#ifdef DEBUGCL
      std::cerr << "Arg is sampler\n";
#endif
        unsigned int bitfield = (*(Sampler **)value)->bitfield();

        arg.refineKind(Arg::Sampler);
        arg.alloc();
        arg.loadData(&bitfield);
#ifdef DEBUGCL
      std::cerr << "Leaving Kernel::setArg\n";
#endif

        return CL_SUCCESS;
    }*/

    if (size != arg_size)
        return CL_INVALID_ARG_SIZE;

    // Check for null values
    cl_mem null_mem = 0;

    if (!value)
    {
        switch (arg->kind())
        {
            case Arg::Buffer:
            case Arg::Image2D:
            case Arg::Image3D:
#ifdef DBG_KERNEL
              std::cerr << "Arg::Buffer, Arg::Image2D, Arg::Image3D\n";
#endif
                // Special case buffers : value can be 0 (or point to 0)
                value = &null_mem;

            default:
                return CL_INVALID_ARG_VALUE;
        }
    }

    // Copy the data
    //arg.alloc();
    arg->loadData(value);

  // Loop through dependent devices and check whether any of them (the LE1) need
  // to perform anything involving moving the data out. Then we can pass the
  // data size and type.
  for (size_t i=0; i<p_device_dependent.size(); ++i)
  {
    DeviceDependent &rs = p_device_dependent[i];
  }
#ifdef DBG_KERNEL
    std::cerr << "Leaving Kernel::setArg with CL_SUCCESS\n";
#endif
    return CL_SUCCESS;
}

unsigned int Kernel::numArgs() const
{
    return p_args.size();
}

const Kernel::Arg *Kernel::arg(unsigned int index) const
{
    return p_args.at(index);
}

bool Kernel::argsSpecified() const
{
    for (size_t i=0; i<p_args.size(); ++i)
    {
        if (!p_args[i]->defined()) {
#ifdef DBG_OUTPUT
          std::cout << "ERROR: arg " << i << " is not defined!\n";
#endif
            return false;
        }
    }

    return true;
}

bool Kernel::hasLocals() const
{
    return p_has_locals;
}

DeviceKernel *Kernel::deviceDependentKernel(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.kernel;
}

cl_int Kernel::info(cl_kernel_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const
{
#ifdef DBG_KERNEL
  std::cerr << "Entering Kernel::info\n";
#endif
    void *value = 0;
    size_t value_length = 0;

    union {
        cl_uint cl_uint_var;
        cl_program cl_program_var;
        cl_context cl_context_var;
    };

    switch (param_name)
    {
        case CL_KERNEL_FUNCTION_NAME:
            MEM_ASSIGN(p_name.size() + 1, p_name.c_str());
            break;

        case CL_KERNEL_NUM_ARGS:
            SIMPLE_ASSIGN(cl_uint, p_args.size());
            break;

        case CL_KERNEL_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_KERNEL_CONTEXT:
            SIMPLE_ASSIGN(cl_context, parent()->parent());
            break;

        case CL_KERNEL_PROGRAM:
            SIMPLE_ASSIGN(cl_program, parent());
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
#ifdef DBG_KERNEL
  std::cerr << "Leaving Kernel::info\n";
#endif

    return CL_SUCCESS;
}

cl_int Kernel::workGroupInfo(DeviceInterface *device,
                             cl_kernel_work_group_info param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret) const
{
#ifdef DBG_KERNEL
  std::cerr << "Entering Kernel::workGroupInfo\n";
#endif
    void *value = 0;
    size_t value_length = 0;

    union {
        size_t size_t_var;
        size_t three_size_t[3];
        cl_ulong cl_ulong_var;
    };

    const DeviceDependent &dep = deviceDependent(device);

    switch (param_name)
    {
        case CL_KERNEL_WORK_GROUP_SIZE:
            SIMPLE_ASSIGN(size_t, dep.kernel->workGroupSize());
            break;

        case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
            // TODO: Get this information from the kernel source
            three_size_t[0] = 0;
            three_size_t[1] = 0;
            three_size_t[2] = 0;
            value = &three_size_t;
            value_length = sizeof(three_size_t);
            break;

        case CL_KERNEL_LOCAL_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, dep.kernel->localMemSize());
            break;

        case CL_KERNEL_PRIVATE_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, dep.kernel->privateMemSize());
            break;

        case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
            SIMPLE_ASSIGN(size_t, dep.kernel->preferredWorkGroupSizeMultiple());
            break;

        default:
#ifdef DBG_OUTPUT
            std::cout << "Leaving Kernel::workGroupInfo, INVALID_VALUE\n";
#endif
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

#ifdef DBG_KERNEL
  std::cerr << "Leaving Kernel::workGroupInfo\n";
#endif
    return CL_SUCCESS;
}

/*
 * Kernel::Arg
 */
Kernel::Arg::Arg(unsigned short vec_dim, File file, Kind kind, llvm::Type* type)
: p_vec_dim(vec_dim), p_file(file), p_kind(kind), p_type(type),
  //p_data(0),
  p_defined(false), p_runtime_alloc(0)
{
#ifdef DBG_KERNEL
  std::cerr << "Created Arg at " << std::hex << (unsigned long)this
    << std::endl;
#endif

  pointer = false;
  // FIXME Add more types to this? 
  if (type->getTypeID() == llvm::Type::PointerTyID)
    pointer = true;
}

/*
Kernel::Arg::Arg(const Kernel::Arg& arg) {
  p_vec_dim = arg.vecDim();
  p_file = arg.file();
  p_type = arg.type();
  p_kind = arg.kind();
  p_defined = arg.defined();
  p_runtime_alloc = arg.allocAtKernelRuntime();
  //if (arg.data() && arg.defined()) {
    //p_data = new unsigned long[arg.vecDim()];
    //std::memcpy(p_data, arg.data(), arg.valueSize() * arg.vecDim());
  //}

  argData = arg.getData();
}
*/
Kernel::Arg::Arg(const Kernel::Arg& arg) {
#ifdef DBG_OUTPUT
  std::cout << "Arg copy constructor" << std::endl;
#endif
}

Kernel::Arg::~Arg()
{
#ifdef DBG_KERNEL
  std::cerr << "Destructing Kernel::Arg: "
    << std::hex << (unsigned long)this << std::endl;
#endif
  /*
  if (p_data) {
#ifdef DBG_KERNEL
    std::cerr << "freeing data at " << std::hex << (unsigned long)p_data
      << std::endl;
#endif
    //std::free(p_data);
    delete[] (unsigned long*)p_data;
  }*/
#ifdef DBG_KERNEL
  std::cerr << "Successfully destroyed Kernel::Arg: "
    << std::hex << (unsigned long)this << std::endl;
#endif
}

void Kernel::Arg::alloc()
{
  // TODO Don't know if I actually need to allocate memory for the kernel arg.
  /*
  if (!p_data) {
#ifdef DBG_KERNEL
    std::cerr << "Allocating data for Kernel::Arg ("
      << std::hex << (unsigned long)this << ")" << std::endl;
#endif
    //p_data = std::malloc(p_vec_dim * valueSize());
    p_data = new unsigned long[p_vec_dim];
  }*/
}

void Kernel::Arg::loadData(const void *data)
{
#ifdef DBG_KERNEL
  std::cerr << "Kernel::Arg::loadData for Arg at " << std::hex << this
    << ", copying " << p_vec_dim * valueSize() << " bytes of data\n";
#endif

  if (p_kind == Arg::Buffer) {
    argData.mem = *(static_cast<MemObject**>((const_cast<void*>(data))));
#ifdef DBG_KERNEL
    std::cerr << "Arg is a buffer, set addr to = " << std::hex <<
      (unsigned long) argData.mem << std::endl;
#endif
  }
  else if (p_kind == Arg::Int8)
    argData.i8 = *(static_cast<unsigned char*>(const_cast<void*>(data)));
  else if (p_kind == Arg::Int16)
    argData.i16 = *(static_cast<unsigned short*>(const_cast<void*>(data)));
  else if (p_kind == Arg::Int32)
    argData.i32 = *(static_cast<unsigned*>(const_cast<void*>(data)));
  else
    argData.f32 = *(static_cast<float*>(const_cast<void*>(data)));

  /* TODO Use the p_type to decide how to write the data out instead of doing a
     memcpy. */
  //  std::memcpy(p_data, data, p_vec_dim * valueSize());
    // FIXME What about if it isn't a MemObject?!
    p_defined = true;
}

DeviceBuffer *Kernel::Arg::getDeviceBuffer(DeviceInterface *device) const {
  return argData.mem->deviceBuffer(device);
}

unsigned Kernel::Arg::getMemSize() const {
  return argData.mem->size();
}

void Kernel::Arg::setAllocAtKernelRuntime(size_t size)
{
    p_runtime_alloc = size;
    p_defined = true;
}

void Kernel::Arg::refineKind (Kernel::Arg::Kind kind)
{
    p_kind = kind;
}

bool Kernel::Arg::operator!=(const Arg *b)
{
    bool same = (p_vec_dim == b->p_vec_dim) &&
                (p_file == b->p_file) &&
                (p_kind == b->p_kind);

    return !same;
}

size_t Kernel::Arg::valueSize() const
{
    switch (p_kind)
    {
        case Invalid:
            return 0;
        case Int8:
            return 1;
        case Int16:
            return 2;
        case Int32:
        case Sampler:
            return 4;
        case Int64:
            return 8;
        case Float:
            return sizeof(cl_float);
        case Double:
            return sizeof(double);
        case Buffer:
        case Image2D:
        case Image3D:
            return sizeof(cl_mem);
    }

    return 0;
}

unsigned short Kernel::Arg::vecDim() const
{
    return p_vec_dim;
}

Kernel::Arg::File Kernel::Arg::file() const
{
    return p_file;
}

Kernel::Arg::Kind Kernel::Arg::kind() const
{
    return p_kind;
}

llvm::Type* Kernel::Arg::type() const
{
  return p_type;
}

bool Kernel::Arg::defined() const
{
    return p_defined;
}

size_t Kernel::Arg::allocAtKernelRuntime() const
{
    return p_runtime_alloc;
}

const void *Kernel::Arg::value(unsigned short index) const
{
    const char *data = (const char *)p_data;
    unsigned int offset = index * valueSize();

    data += offset;

    return (const void *)data;
}

const void *Kernel::Arg::data() const
{
    return p_data;
}
