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
 * \file cpu/kernel.cpp
 * \brief LE1 kernel
 */

#include "LE1kernel.h"
#include "LE1device.h"
#include "LE1buffer.h"
#include "LE1program.h"
#include "LE1Simulator.h"
#include "LE1WorkitemCoarsen.h"
//#include "LE1builtins.h"

#include "../../compiler.h"
#include "../../kernel.h"
#include "../../memobject.h"
#include "../../events.h"
#include "../../program.h"


#include <llvm/Function.h>
//#include <llvm/Constants.h>
//#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
//#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <sys/mman.h>


using namespace Coal;

LE1Kernel::LE1Kernel(LE1Device *device, Kernel *kernel, llvm::Function *function)
: DeviceKernel(), p_device(device), p_kernel(kernel), p_function(function),
  p_call_function(0)
{
    pthread_mutex_init(&p_call_function_mutex, 0);
}

LE1Kernel::~LE1Kernel()
{
    if (p_call_function)
        p_call_function->eraseFromParent();

    pthread_mutex_destroy(&p_call_function_mutex);
}

size_t LE1Kernel::workGroupSize() const
{
    return 0; // TODO
}

cl_ulong LE1Kernel::localMemSize() const
{
    return 0; // TODO
}

cl_ulong LE1Kernel::privateMemSize() const
{
    return 0; // TODO
}

size_t LE1Kernel::preferredWorkGroupSizeMultiple() const
{
    return 0; // TODO set preferredWorkGroupSizeMultiple after ILP analysis
}

static std::string ConvertToBinary(unsigned int value) {
  std::string binary_string;
    unsigned mask = 0x80000000;
    while(mask > 0x00) {
      if((value & mask) != 0)
        binary_string += "1";
      else
        binary_string += "0";
      mask = mask >> 1;
    }
    return binary_string;
}

template<typename T>
T k_exp(T base, unsigned int e)
{
    T rs = base;

    for (unsigned int i=1; i<e; ++i)
        rs *= base;

    return rs;
}

// Try to find the size a work group has to have to be executed the fastest on
// the LE1.
size_t LE1Kernel::guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                          size_t global_work_size) const
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1Kernel::guessWorkGroupSize\n";
#endif
    unsigned int cpus = p_device->numLE1s();

    /*
    // Don't break in too small parts
    if (k_exp(global_work_size, num_dims) > 64)
        return global_work_size;*/

    // FIXME This is just to get the ball rolling - will break code
    return global_work_size / cpus;

    // Find the divisor of global_work_size the closest to cpus but >= than it
    unsigned int divisor = cpus;

    while (true)
    {
        if ((global_work_size % divisor) == 0)
            break;

        // Don't let the loop go up to global_work_size, the overhead would be
        // too huge
        if (divisor > global_work_size || divisor > cpus * 32)
        {
            divisor = 1;  // Not parallel but has no CommandQueue overhead
            break;
        }
    }

#ifdef DEBUGCL
    std::cerr << "Leaving LE1Kernel::guessWorkGroupSize\n";
#endif
    // Return the size
    return global_work_size / divisor;
}

llvm::Function *LE1Kernel::function() const
{
    return p_function;
}

Kernel *LE1Kernel::kernel() const
{
    return p_kernel;
}

LE1Device *LE1Kernel::device() const
{
    return p_device;
}

// From Wikipedia : http://www.wikipedia.org/wiki/Power_of_two#Algorithm_to_round_up_to_power_of_two
template <class T>
T next_power_of_two(T k) {
        if (k == 0)
                return 1;
        k--;
        for (int i=1; i<sizeof(T)*8; i<<=1)
                k = k | k >> i;
        return k+1;
}

size_t LE1Kernel::typeOffset(size_t &offset, size_t type_len)
{
    size_t rs = offset;

    // Align offset to stype_len
    type_len = next_power_of_two(type_len);
    size_t mask = ~(type_len - 1);

    while (rs & mask != rs)
        rs++;

    // Where to try to place the next value
    offset = rs + type_len;

    return rs;
}

// TODO !! Look at this, help create a launcher?
llvm::Function *LE1Kernel::callFunction()
{
    pthread_mutex_lock(&p_call_function_mutex);

    // If we can reuse the same function between work groups, do it
    if (p_call_function)
    {
        llvm::Function *rs = p_call_function;
        pthread_mutex_unlock(&p_call_function_mutex);

        return rs;
    }

    llvm::Function *stub_function;
    return stub_function;

    /* Create a stub function in the form of
     *
     * void stub(void *args) {
     *     kernel(*(int *)((char *)args + 0),
     *            *(float **)((char *)args + sizeof(int)),
     *            *(sampler_t *)((char *)args + sizeof(int) + sizeof(float *)));
     * }
     *
     * In LLVM, it is exprimed in the form of :
     *
     * @stub(i8* args) {
     *     kernel(
     *         load(i32* bitcast(i8* getelementptr(i8* args, i64 0), i32*)),
     *         load(float** bitcast(i8* getelementptr(i8* args, i64 4), float**)),
     *         ...
     *     );
     * }
     */
    /*
    llvm::FunctionType *kernel_function_type = p_function->getFunctionType();
    llvm::FunctionType *stub_function_type = llvm::FunctionType::get(
        p_function->getReturnType(),
        llvm::Type::getInt8PtrTy(
            p_function->getContext()),
        false);
    llvm::Function *stub_function = llvm::Function::Create(
        stub_function_type,
        llvm::Function::InternalLinkage,
        "",
        p_function->getParent());

    // Insert a basic block
    llvm::BasicBlock *basic_block = llvm::BasicBlock::Create(
        p_function->getContext(),
        "",
        stub_function);

    // Create the function arguments
    llvm::Argument &stub_arg = stub_function->getArgumentList().front();
    llvm::SmallVector<llvm::Value *, 8> args;
    size_t args_offset = 0;

    for (unsigned int i=0; i<kernel_function_type->getNumParams(); ++i)
    {
        llvm::Type *param_type = kernel_function_type->getParamType(i);
        llvm::Type *param_type_ptr = param_type->getPointerTo(); // We'll use pointers to the value
        const Kernel::Arg &arg = p_kernel->arg(i);

        // Calculate the size of the arg
        size_t arg_size = arg.valueSize() * arg.vecDim();

        // Get where to place this argument
        size_t arg_offset = typeOffset(args_offset, arg_size);

        // %1 = getelementptr(args, $arg_offset);
        llvm::Value *getelementptr = llvm::GetElementPtrInst::CreateInBounds(
            &stub_arg,
            llvm::ConstantInt::get(stub_function->getContext(),
                                   llvm::APInt(arg_offset, 64)),
            "",
            basic_block);

        // %2 = bitcast(%1, $param_type_ptr)
        llvm::Value *bitcast = new llvm::BitCastInst(
            getelementptr,
            param_type_ptr,
            "",
            basic_block);

        // %3 = load(%2)
        llvm::Value *load = new llvm::LoadInst(
            bitcast,
            "",
            false,
            arg_size,   // We ensure that an argument is always aligned on its size, it enables things like fast movaps
            basic_block);

        // We have the value, send it to the function
        args.push_back(load);
    }

    // Create the call instruction
    llvm::CallInst *call_inst = llvm::CallInst::Create(
        p_function,
        args,
        "",
        basic_block);
    call_inst->setCallingConv(p_function->getCallingConv());
    call_inst->setTailCall();

    // Create a return instruction to end the stub
    llvm::ReturnInst::Create(
        p_function->getContext(),
        basic_block);

    // Retain the function if it can be reused
    p_call_function = stub_function;

    pthread_mutex_unlock(&p_call_function_mutex);

    return stub_function;*/
}

/*
 * LE1KernelEvent
 */

unsigned int LE1KernelEvent::addr = 4;
std::vector<LE1Buffer*> LE1KernelEvent::DeviceBuffers;

LE1KernelEvent::LE1KernelEvent(LE1Device *device, KernelEvent *event)
: p_device(device), p_event(event), p_current_wg(0), p_finished_wg(0),
  p_kernel_args(0)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::LE1KernelEvent\n";
#endif

#ifdef DEBUGCL
  std::cerr << "mutex_init\n";
#endif
    // Mutex
    pthread_mutex_init(&p_mutex, 0);

#ifdef DEBUGCL
  std::cerr << "memset\n";
#endif
    // Set current work group to (0, 0, ..., 0)
    std::memset(p_current_work_group, 0, event->work_dim() * sizeof(size_t));

    // Populate p_max_work_groups
    p_num_wg = 1;

    for (cl_uint i=0; i<event->work_dim(); ++i)
    {
#ifdef DEBUGCL
      std::cerr << "i = " << i << ", global work size = "
        << event->global_work_size(i) << " and local work size = "
        << event->local_work_size(i) << std::endl;
#endif
      // Work groups are now defined by the number of cores, local work size
      // will be calculated later
        //p_max_work_groups[i] =
          //  (event->global_work_size(i) / event->local_work_size(i)) - 1; // 0..n-1, not 1..n
      p_max_work_groups[i] = device->numLE1s();

        p_num_wg *= p_max_work_groups[i] + 1;
    }
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelEvent::LE1KernelEvent\n";
#endif
}

LE1KernelEvent::~LE1KernelEvent()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::~LE1KernelEvent\n";
#endif
    pthread_mutex_destroy(&p_mutex);

    if (p_kernel_args)
        std::free(p_kernel_args);

#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelEvent::~LE1KernelEvent\n";
#endif
}

bool LE1KernelEvent::createFinalSource(LE1Program *prog) {
  // TODO Create a proper .s file:
#ifdef DEBUGCL
  std::cerr << "Entering createFinalSource\n";
#endif

  KernelName = p_event->kernel()->function(p_device)->getName();
  OriginalSource = prog->getSource();
  OriginalSourceName = KernelName + ".cl";
  CoarsenedSourceName = "workgroup_" + KernelName  + ".cl";
  CoarsenedBCName = "workgroup_" + KernelName + ".bc";
  FinalBCName = KernelName + ".bc";
  TempAsmName = "temp_" + KernelName + ".s";
  FinalAsmName = "kernel_" + KernelName + ".s";
  CompleteFilename = "final_" + KernelName + ".s";

  //CalculateBufferAddrs();

  // Only compile a kernel once
  if (!p_event->kernel()->isBuilt()) {
    // FIXME Change this to the kernel specific name
    //if(!CompileSource("program.bc", filename))
    if(!CompileSource())
      return false;
  }
  else {
#ifdef DEBUGCL
    std::cerr << "Program has already been compiled\n";
#endif
  }

  if (!WriteDataArea())
    return false;

  std::string assemble = "perl " + LE1Device::ScriptsDir + "secondpass.pl ";
  assemble.append(CompleteFilename);
  assemble.append(" -OPC=").append(LE1Device::IncDir + "opcodes.txt");

  if (system(assemble.c_str()) != 0)
    return false;
  else {
    p_event->kernel()->SetBuilt();
    // delete the intermediate files
    std::string clean = "rm " + OriginalSourceName + " " + CoarsenedSourceName
      + " " + CoarsenedBCName + " " + TempAsmName;
    system(clean.c_str());
    return true;
  }
}

void LE1KernelEvent::CalculateBufferAddrs(unsigned Addr) {
  // FIXME should the global_addr be defined like this?

  // FIXME surely we could calculate this number during kernel creation?
  // Not all arguments will be buffers, and so not all will require having data
  // written to global memory
  Kernel *TheKernel = p_event->kernel();

  for (unsigned i = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& arg = TheKernel->arg(i);
    if (arg.kind() == Kernel::Arg::Buffer) {
      ArgAddrs.push_back(Addr);
      Addr += (*(MemObject**)arg.data())->size();
    }
  }
}

bool LE1KernelEvent::CompileSource() {
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::CompileSource\n";
#endif

  // TODO This part needs to calculate how many cores to instantiate
  // Impose an upper limit of 12 cores?
  unsigned cores = p_device->numLE1s();
  unsigned merge_dims[3] = {1, 1, 1};
  unsigned WorkgroupsPerCore[3] = { 1, 1, 1};

  // Local size may have been set by user, especially in the presence of
  // barriers, so we need to take that into consideration. If the user didn't
  // set a local size, we set it just by global_work_size / num_of_cores. We
  // can adjust the launcher to work with several workgroups in this case.
  for(unsigned i = 0; i < p_event->work_dim(); ++i) {
    merge_dims[i] = p_event->global_work_size(i) / cores;
    if (p_event->local_work_size(i) != merge_dims[i]) {
      WorkgroupsPerCore[i] = merge_dims[i] / p_event->local_work_size(i);
      merge_dims[i] = p_event->local_work_size(i);
    }
  }

  // First, write the source string to a file
  std::ofstream SourceFile;
  SourceFile.open(OriginalSourceName.c_str());
  SourceFile << OriginalSource << std::endl;
  SourceFile.close();

  // Then pass the file name to workitem coarsener
  WorkitemCoarsen Coarsener(merge_dims[0], merge_dims[1], merge_dims[2]);
  if (!Coarsener.CreateWorkgroup(OriginalSourceName))
    return false;

  std::string WorkgroupSource = Coarsener.getFinalKernel();
  if (!Coarsener.Compile(CoarsenedBCName, WorkgroupSource))
    return false;

#ifdef DEBUGCL
  std::cerr << "Merged Kernel\n";
#endif

  // Calculate the addresses in global memory where the arguments will be stored
  Kernel* kernel = p_event->kernel();


  std::stringstream launcher;
  for(unsigned i = 0, j = 0; i < kernel->numArgs(); ++i) {
    const Kernel::Arg& arg = kernel->arg(i);
    if (arg.kind() == Kernel::Arg::Buffer) {
      launcher << "extern int BufferArg_" << j << ";" << std::endl;
      ++j;
    }
  }

  // FIXME Really not sure if this works!
  // Create a main function to the launcher for the kernel
  launcher << "int main(void) {\n";
  unsigned NestedLoops = 0;
  if (WorkgroupsPerCore[2] != 0) {
    launcher << "  for (unsigned z = 0; z < " << WorkgroupsPerCore[2] << "; ++z) {\n"
    <<      "__builtin_le1_set_group_id_2(z);\n"; //group_id[((__builtin_le1_read_cpuid()*4) + 2)] = z;\n";
    ++NestedLoops;
  }
  if (WorkgroupsPerCore[1] != 0) {
    launcher << "    for (unsigned y = 0; y < " << WorkgroupsPerCore[1] << "; ++y) {\n"
    << "      __builtin_le1_set_group_id_1(y);\n"; //group_id[((__builtin_le1_read_cpuid()*4) + 1)] = y;\n";
    ++NestedLoops;
  }
  if (WorkgroupsPerCore[0] != 0) {
    launcher << "      for (unsigned x = 0; x < " << WorkgroupsPerCore[0] << "; ++x) {\n"
    << "        __builtin_le1_set_group_id_0(x);\n"; //group_id[((__builtin_le1_read_cpuid()*4) + 0)] = x;\n";
    ++NestedLoops;
  }
  launcher<< "        " << KernelName << "(";

  // TODO Instead of writing a main file, passing immediates and having to
  // compile every time, we can write the addresses to memory and pass them
  // in variable names. This only then requires the few variables to be
  // rewritten instead of compiling and linking everything each time.

  for (unsigned i = 0; i < kernel->numArgs(); ++i) {
    const Kernel::Arg& arg = kernel->arg(i);
    if (arg.kind() == Kernel::Arg::Buffer) {
      //launcher << arg_addrs[j];
      //++j;
      launcher << "&BufferArg_" << i;
    }
    else {
      void *ArgData = const_cast<void*>(arg.data());
      launcher << *(static_cast<unsigned*>(ArgData));
    }
    if (i < (kernel->numArgs()-1))
      launcher << ", ";
    else {
      launcher << ");\n";
      for (unsigned i = 0; i < NestedLoops; ++i)
        launcher << "}\n";
      launcher << "return 0;\n}";
    }
  }

  std::ofstream main;
  main.open("main.c", std::ofstream::out);
  main << launcher.str();
  main.close();

#ifdef DEBUGCL
  std::cerr << "Created main file for the launcher\n";
#endif

  // TODO Clean this up
  // Now that a main.c file exists, compile it to llvm ir and link with the
  // kernel
  system("clang -target le1 -emit-llvm -c main.c");
  std::stringstream link;
  link << "llvm-link main.o " << CoarsenedBCName << " -o " << FinalBCName;
  if(system(link.str().c_str()) != 0)
    return false;

  // Run the transformation pass to prepare it for the assembler
  // Compile the merged kernel to assembly
  std::stringstream compile_command;
  compile_command << "llc -march=le1 -mcpu="<< p_device->target() << " " 
    << FinalBCName << " -o " << TempAsmName;
  if(system(compile_command.str().c_str()) != 0)
    return false;

  //return compiler.produceAsm(input, output);

  std::stringstream pre_asm_command;
  // TODO Include the script as a char array
  pre_asm_command << "perl " << LE1Device::ScriptsDir << "llvmTransform.pl "
    << TempAsmName
    //<< " -OPC=/home/sam/Dropbox/src/LE1/Assembler/includes/opcodes.txt_asm "
    << " > " << FinalAsmName;
  // FIXME return false?
  system(pre_asm_command.str().c_str());

  //std::stringstream clean;
  //clean << "rm " << merged_bc << " " << final_bc << " " << temp_asm << " main.o";
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelEvent::CompileSource\n";
#endif
  //return system(clean.str().c_str());

  return true;
}

void LE1KernelEvent::WriteKernelAttr(std::ostringstream &Output, size_t attr) {
  Output << std::hex << std::setw(5) << std::setfill('0') << addr << " - "
      << std::hex << std::setw(8) << std::setfill('0')
      << attr << " - "
      << ConvertToBinary(attr) << std::endl;
    addr += 4;
}

bool LE1KernelEvent::WriteDataArea() {
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::WriteDataArea\n";
#endif
  std::string CopyCommand = "cp " + FinalAsmName + " " + CompleteFilename;
  system(CopyCommand.c_str());

  std::ofstream FinalSource;
  FinalSource.open(CompleteFilename.c_str(), std::ios_base::app);
  std::ostringstream Output (std::ostringstream::out);

  // Size taken by kernel attributes before the argument data
  // FIXME is this size right?
  unsigned DataSize = 52;

  // Reset the static addr
  addr = 0x4;

  Kernel* TheKernel = p_event->kernel();

  // FIXME Is this calculating the size properly?
  for(unsigned i = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& Arg = TheKernel->arg(i);
    // TODO This will need to handle images as well
    if (Arg.kind() == Kernel::Arg::Buffer) {
      // Local Buffers need to be copied for each workgroup (core).
      if (Arg.file() == Kernel::Arg::Local)
        DataSize += (*(MemObject**)Arg.data())->size() * p_device->numLE1s();
      else
        DataSize += (*(MemObject**)Arg.data())->size();
    }
  }

  Output << "##Data Labels" << std::endl;
  Output << "00000 - work_dim" << std::endl;
  Output << "00004 - global_size" << std::endl;
  Output << "00010 - local_size" << std::endl;
  Output << "0001c - num_groups" << std::endl;
  Output << "00028 - global_offset" << std::endl;
  Output << "00034 - num_cores" << std::endl;
  // Each core has a word allocated to it to store group id data, we use one
  // byte per dimension and so waste a byte. The cores can access their group_id
  // using their cpuid: group_id[((cpuid >> 8) + dim)]

  Output << "00038 - group_id" << std::endl;
  unsigned GroupIdEnd = 0x38;
  unsigned NumCores = p_device->numLE1s();
  for (unsigned i = 0; i < NumCores; ++i)
    GroupIdEnd += 4;

  CalculateBufferAddrs(GroupIdEnd);

  for (unsigned i = 0, j = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& arg = TheKernel->arg(i);
    if (arg.kind() == Kernel::Arg::Buffer) {
      Output << std::hex << std::setw(5) << std::setfill('0') << ArgAddrs[j]
        << " - BufferArg_" << j << std::endl;
      ++j;
    }
  }

  Output << std::endl;
  Output << "##Data Section - " << DataSize << " - Data_align=32" << std::endl;
  Output << "00000 - " << std::hex << std::setw(8) << std::setfill('0')
    << p_event->work_dim() << " - "
    << ConvertToBinary(p_event->work_dim())
    << std::endl;

  for(unsigned i = 0; i < 3; ++i)
    WriteKernelAttr(Output, p_event->global_work_size(i));
#ifdef DEBUGCL
  std::cerr << "Written global work size\n";
#endif

  for (unsigned i = 0; i < 3; ++i)
    WriteKernelAttr(Output, p_event->local_work_size(i));
#ifdef DEBUGCL
  std::cerr << "Written local work size\n";
#endif

  // FIXME Printing num_groups?
  for (unsigned i = 0; i < 3; ++i) {
    if ((p_event->global_work_size(i) == 0) ||
        (p_event->local_work_size(i) == 0))
      WriteKernelAttr(Output, 0);
    else {
      unsigned NumGroups =
        p_event->global_work_size(i) / p_event->local_work_size(i);
      WriteKernelAttr(Output, NumGroups);
    }
  }

#ifdef DEBUGCL
  std::cerr << "Written work groups (cores)\n";
#endif

  for(unsigned i = 0; i < 3; ++i)
    WriteKernelAttr(Output, p_event->global_work_offset(i));
#ifdef DEBUGCL
  std::cerr << "Written global work offset\n";
#endif

  // Set num_cores
  WriteKernelAttr(Output, NumCores);

  // Zero initalise all the group ids
  for (unsigned i = 0; i < NumCores; ++i)
    WriteKernelAttr(Output, 0);

  FinalSource << Output.str();
  FinalSource.close();

  // First, write the global data first
  for(unsigned i = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& Arg = TheKernel->arg(i);

    // TODO This will need to check for images too
    if (Arg.kind() != Kernel::Arg::Buffer)
      continue;

    if (Arg.file() != Kernel::Arg::Local)
      if (!HandleBufferArg(Arg))
        return false;
  }

  // Then write the local area for each core
  for(unsigned i = 0; i < TheKernel->numArgs(); ++i) {
    const Kernel::Arg& Arg = TheKernel->arg(i);

    // TODO This will need to check for images too
    if (Arg.kind() != Kernel::Arg::Buffer)
      continue;

    if (Arg.file() == Kernel::Arg::Local)
      for (unsigned i = 0; i < p_device->numLE1s(); ++i)
        if(!HandleBufferArg(Arg))
          return false;
  }
#ifdef DEBUGCL
  std::cerr << "Leaving createFinalSource\n";
#endif

  return true;
}

bool LE1KernelEvent::HandleBufferArg(const Kernel::Arg &arg) {
#ifdef DEBUGCL
  std::cerr << "HandleBufferArg\n";
#endif
    llvm::Type* type = arg.type();
    LE1Buffer* buffer =
      static_cast<LE1Buffer*>((*(MemObject**)arg.data())->deviceBuffer(p_device));

    // Check whether this buffer has been passed to the device before
    /*
    bool isBufferOnDevice = false;
    for (std::vector<LE1Buffer*>::iterator DI = DeviceBuffers.begin(),
          DE = DeviceBuffers.end(); DI != DE; ++DI) {
      if (*DI == buffer) {
        isBufferOnDevice = true;
        buffer = *DI;
        break;
      }
    }*/

    unsigned TotalSize = (*(MemObject**)arg.data())->size();
    void *Data;
    // If it is, it means that the data that is being passed to the new kernel
    // needs to be updated with the results from the previous run.
    /*
    if (isBufferOnDevice) {
      Data = malloc(TotalSize);
#ifdef DEBUGCL
      std::cerr << "Buffer is on device - need to get new data!\n";
#endif
      if (arg.type()->isIntegerTy(8))
        p_device->getSimulator()->readCharData(buffer->addr(), TotalSize,
                                               (unsigned char*)Data);
      //else if (arg.type()->isIntegerTy(16))
        //p_device->getSimulator()->readData(buffer->addr(), TotalSize, Data);
      else if (arg.type()->isIntegerTy(32))
        p_device->getSimulator()->readIntData(buffer->addr(), TotalSize,
                                              (unsigned int*)Data);
    }
    else {
      DeviceBuffers.push_back(buffer);
      Data = buffer->data();
    }*/

    Data = buffer->data();
    buffer->setAddr(addr);

#ifdef DEBUGCL
    std::cerr << "Set buffer arg address to " << addr << std::endl;
#endif
    if (type->getTypeID() == llvm::Type::IntegerTyID) {
#ifdef DEBUGCL
      std::cerr << "Data is int\n";
#endif
      if (type->isIntegerTy(8)) {
        PrintData(Data, 0, sizeof(char), TotalSize);
      }
      else if (type->isIntegerTy(16)) {
        PrintData(Data, 0, sizeof(short), TotalSize);
      }
      else if (type->isIntegerTy(32)) {
        PrintData(Data, 0, sizeof(int), TotalSize);
      }
    }
    else if (type->getTypeID() == llvm::Type::FloatTyID) {
#ifdef DEBUGCL
      std::cerr << "Data is float\n";
#endif
      PrintData(Data, 0, sizeof(int), TotalSize);
    }
    else if (type->getTypeID() == llvm::Type::VectorTyID) {
#ifdef DEBUGCL
      std::cerr << "Data is vector type\n";
#endif
      llvm::Type* elementType =
        static_cast<llvm::VectorType*>(type)->getElementType();

      if (elementType->getTypeID() == llvm::Type::IntegerTyID) {
#ifdef DEBUGCL
        std::cerr << "Elements are integers\n";
#endif
        if (elementType->isIntegerTy(8)) {
          PrintData(Data, 0, sizeof(char), TotalSize);
        }
        else if (elementType->isIntegerTy(16)) {
          PrintData(Data, 0, sizeof(short), TotalSize);
        }
        else if (elementType->isIntegerTy(32)) {
          PrintData(Data, 0, sizeof(int), TotalSize);
        }
        else {
          std::cerr << "!! unhandled vector element type!!\n";
          return false;
        }
      }
    }
    else if (type->getTypeID() == llvm::Type::StructTyID) {
#ifdef DEBUGCL
      std::cerr << "Data is Struct\n";
#endif
      llvm::StructType* StructArg = static_cast<llvm::StructType*>(type);
      WriteStructData(StructArg, Data, TotalSize);
      addr += TotalSize;
    }
    else {
      std::cerr << "!! Unhandled argument type of size = " << TotalSize
        << std::endl;
      return false;
    }
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelEvent::WriteDataArea\n";
#endif
  //if (isBufferOnDevice)
    //free(Data);

  return true;
}


void LE1KernelEvent::WriteStructData(llvm::StructType *StructArg,
                                     const void *Data,
                                     unsigned TotalSize) {
  // Chars are aligned on byte boundaries, short on half word boundaries
  // and ints are word aligned. So it could mean we need to pack data like
  // this:
  // char | char | short    |
  // int                    |
  // Print data is not currently setup to handle that type of packing, it zero
  // pads to finish the final line, expecting a large lump of data, of the same
  // type. So we need a new function, and we need to remember that the data will
  // also already be packed for x86_64, but it looks like that it matches the
  // LE1.
  // FIXME are pointers allowed in structs?

  unsigned NumElements = StructArg->getNumElements();
#ifdef DEBUGCL
  std::cerr << "Data is struct type, size = " << TotalSize << " with "
    << NumElements << " elements" << std::endl;
#endif

  // DataWritten acts as a counter, as well as an offset value to the data
  // pointer that we pass to print from.
  unsigned DataWritten = 0;
  while (DataWritten < TotalSize) {

    for (unsigned i = 0; i < NumElements; ++i) {
      llvm::Type *ElementType = StructArg->getTypeAtIndex(i);

      if (ElementType->getTypeID() == llvm::Type::IntegerTyID) {
        if (ElementType->isIntegerTy(32))
          PrintSingleElement(Data, &DataWritten, sizeof(int));
        else if (ElementType->isIntegerTy(16))
          PrintSingleElement(Data, &DataWritten, sizeof(short));
        else if (ElementType->isIntegerTy(8))
          PrintSingleElement(Data, &DataWritten, sizeof(char));
      }
    } // end for

  } // end while

} // end WriteStructData

static void ConvertToBinary(std::string *binary_string,
                            const void* value,
                            size_t type) {
  unsigned mask = 0;
  switch(type) {
  case sizeof(char): {
    mask = 0x80;
    char* byte = (char*)value;
    while(mask > 0x00) {
      if((*byte & mask) != 0)
        *binary_string += "1";
      else
        *binary_string += "0";
      mask = mask >> 1;
    }
    break;
  }
  case sizeof(short): {
    mask = 0x8000;
    short* half = (short*)value;
    while(mask > 0x00) {
      if((*half & mask) != 0)
        *binary_string += "1";
      else
        *binary_string += "0";
      mask = mask >> 1;
    }
    break;
  }
  case sizeof(int): {
    mask = 0x80000000;
    unsigned* word = (unsigned*)value;
    while(mask > 0x00) {
      if((*word & mask) != 0)
        *binary_string += "1";
      else
        *binary_string += "0";
      mask = mask >> 1;
    }
    break;
  }
  default:
    std::cerr << "Error! size is too great in convert_to_binary\n";
    break;
    //exit(1);
  }
}

inline void LE1KernelEvent::PrintLine(unsigned Address,
                                      std::ostringstream &HexString,
                                      std::string &BinaryString) {
#ifdef DEBUGCL
  //std::cerr << "PrintLine\n";
#endif
  std::ofstream asm_out;
  asm_out.open(CompleteFilename.c_str(), std::ios_base::app);
  std::ostringstream DataLine(std::ostringstream::out);

  // Write the address
  DataLine << std::hex << std::setw(5) << std::setfill('0')
    << (Address - 4) << " - ";

  // Write data in hex
  DataLine << HexString.str();

  // Write data in binary
  DataLine << " - " << BinaryString << std::endl;
  BinaryString.erase();
  HexString.str("");

  asm_out << DataLine.str();
  asm_out.close();
}

// This function is used for writing elements of a structure. It takes single
// elements and automatically inserts the correct padding and adjusts the data
// pointer offset accordingly. The data is actually output a line at a time, so
// this function may be called several times before the output file is even
// changed.
void LE1KernelEvent::PrintSingleElement(const void *Data,
                                        unsigned *Offset,
                                        size_t ElementSize) {
#ifdef DEBUGCL
  //std::cerr << "PrintSingleElement, Address = " << addr << " + " << *Offset
    //<< std::endl;
#endif
  static unsigned ByteCount = 0;
  static std::string BinaryString;
  static std::ostringstream HexString;
  unsigned ZeroPad = 0;

  // FIXME Where should we update 'addr'?

  switch(ElementSize) {
  case sizeof(char): {
    const char* Byte = static_cast<const char*>(Data) + *Offset;
    std::string ByteString;
    ConvertToBinary(&ByteString, Byte, sizeof(char));
    BinaryString.append(ByteString);
    HexString << std::setw(2) << std::setfill('0') << *Byte;
    ++ByteCount;
    ++(*Offset);
    break;
  }
  case sizeof(short): {
    if ((ByteCount == 0) || (ByteCount == 2)) {
      std::string HalfString;
      const short* Half = static_cast<const short*>(Data) + (*Offset >> 1);
      ConvertToBinary(&HalfString, Half, sizeof(short));
      BinaryString.append(HalfString);
      HexString << std::setw(4) << std::setfill('0') << *Half;
      ByteCount += 2;
      *Offset += 2;
    }
    else if (ByteCount == 1) {
      std::string PadString;
      ConvertToBinary(&PadString, &ZeroPad, sizeof(char));
      ++(*Offset);
      const short *Half = static_cast<const short*>(Data) + (*Offset >> 1);
      std::string HalfString;
      ConvertToBinary(&HalfString, Half, sizeof(short));
      BinaryString.append(PadString).append(HalfString);
      HexString << std::setw(2) << std::setfill('0') << 0;
      HexString << std::setw(4) << std::setfill('0') << *Half;
      ByteCount += 3;
      *Offset += 2;
    }
    else {
      std::string PadString;
      ConvertToBinary(&PadString, &ZeroPad, sizeof(char));
      BinaryString.append(PadString);
      HexString << std::setw(2) << std::setfill('0') << 0;
      // Write the pad finished line
      PrintLine((addr + *Offset), HexString, BinaryString);

      // Start a new line
      ++(*Offset);
      const short *Half = static_cast<const short*>(Data) + (*Offset >> 1);
      std::string HalfString;
      ConvertToBinary(&HalfString, Half, sizeof(short));
      BinaryString.append(HalfString);
      HexString << std::setw(4) << std::setfill('0') << *Half;
    }
    break;
  }
  case sizeof(int): {
    if (ByteCount == 0) {
      std::string WordString;
      const int *Word = static_cast<const int*>(Data) + (*Offset >> 2);
      ConvertToBinary(&WordString, Word, sizeof(int));
      BinaryString.append(WordString);
      HexString << std::hex << std::setw(8) << std::setfill('0') << *Word;
      *Offset += 4;
    }
    else {
      std::string PadString;
      ConvertToBinary(&PadString, &ZeroPad, sizeof(char));
      for (unsigned i = 0; i < (4 - ByteCount); ++i) {
        BinaryString.append(PadString);
        HexString << std::setw(2) << std::setfill('0') << 0;
        ++(*Offset);
      }
      // Need to write the line that just got pad finished
      PrintLine((addr + *Offset), HexString, BinaryString);

      // Then create the new line
      const int *Word = static_cast<const int*>(Data) + (*Offset >> 2);
      ConvertToBinary(&BinaryString, Word, sizeof(int));
      HexString << std::setw(8) << std::setfill('0') << *Word;
      *Offset += 4;
    }
    ByteCount = 0;
    break;
  }
  }

  ByteCount = ByteCount % 4;

  // If ByteCount is 0, it means we need to write the data line to the file,
  // and start a new line.
  if (ByteCount == 0) {
    PrintLine((addr + *Offset), HexString, BinaryString);
  }
}


void LE1KernelEvent::PrintData(const void* data,
                               size_t offset,
                               size_t size,
                               size_t total_bytes) {
#ifdef DEBUGCL
  std::cerr << "Entered print_data\n";
  std::cerr << "Start address = " << addr << std::endl;
  std::cerr << "Offset = " << offset << ", total bytes = " << total_bytes
    << std::endl;
#endif
  //static int device_mem_ptr = 0x00;
  //unsigned device_mem_ptr = start_addr;
  unsigned device_mem_ptr = addr;
  std::ofstream asm_out;
  asm_out.open(CompleteFilename.c_str(), std::ios_base::app);

  // Divide the number by 4 to get the number of full 32-bit data lines
  // index is used to calculate the jump with elements since we're outputting
  // 32-bits at a time.
  //unsigned total_bytes = size * num_items;
  unsigned index = 0;

  for (unsigned i = 0; i < (total_bytes >> 2); ++i) {
    std::ostringstream data_line (std::ostringstream::out);
    std::string binary_string("\0");

    switch(size) {
    default:
      std::cerr << "!!! ERROR: Unhandled type!\n";
      exit(-1);
      break;
    case sizeof(char): {
      const char* bytes = static_cast<const char*>(data) + offset;
      index = i << 2;
      bytes = &bytes[index];

      // Write formatted data to stream
      data_line << std::hex << std::setw(5) << std::setfill('0')
        << device_mem_ptr << " - ";
      for (unsigned i = 0; i < 4 ; ++i) {
        ConvertToBinary(&binary_string, &bytes[i], sizeof(char));
        data_line << std::setw(2) << std::setfill('0')
          << static_cast<int>(bytes[i]);
      }
      data_line << " - " << binary_string << std::endl;
      break;
    }
    case sizeof(short): {
      const short* halves = static_cast<const short*>(data) + offset;
      index = i << 1;
      halves = &halves[index];

      // Write formatted data to stream
      data_line << std::hex << std::setw(5) << std::setfill('0')
        << device_mem_ptr << " - ";
      for (unsigned i = 0; i < 2; ++i) {
        ConvertToBinary(&binary_string, &halves[i], sizeof(short));
        data_line << std::setw(4) << std::setfill('0') << halves[i];
      }
      data_line << " - "  << binary_string << std::endl;
      break;
    }
    case sizeof(int): {
      const unsigned* word = static_cast<const unsigned*>(data) + offset;
      index = i;
      //word = &word[index];
      ConvertToBinary(&binary_string, &word[index], sizeof(int));
      // Write formatted data to stream
      data_line << std::hex << std::setw(5) << std::setfill('0')
        << device_mem_ptr << " - ";
      data_line << std::setw(8) << std::setfill('0') << word[index];
      data_line << " - " << binary_string << std::endl;
      break;
    }
    }
  
    // Write stream to file
    asm_out << data_line.str();
    device_mem_ptr += 4;
  }
  // Mask total number of bytes with 0011. If the result is 0
  // then the number of bytes is divisible by 4. If not the function can be
  // called with a value that is zero padded.
  unsigned remaining_bytes = total_bytes & 0x3;

  if(remaining_bytes != 0) {

    std::ostringstream data_line (std::ostringstream::out);
    std::string binary_string("\0");
    switch(size) {
      case sizeof(char): {
        // Write a zero padded array and integer value to fill a whole line
        const char* remaining_data = static_cast<const char*>(data) + offset;
        //remaining_data = &remaining_data[index+4];
        unsigned padded_data = 0;
        char padded_array[4] = {0};
        for(unsigned i = 0; i < remaining_bytes; ++i) {
          padded_data |= remaining_data[i] << (12 << i);
          padded_array[i] = remaining_data[i];
        }
        // Write data to the stream
        data_line  << std::hex << std::setw(5) << std::setfill('0')
          << device_mem_ptr << " - ";
        for(unsigned i = 0; i < 4; ++i) {
          ConvertToBinary(&binary_string, &padded_array[i], sizeof(char));
          data_line << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(padded_array[i]);
        }
        data_line << " - " << binary_string << std::endl;
        break;
      }
      case sizeof(short): {
        // Write a zero padded array and integer value to fill a whole line
        const short* remaining_data = static_cast<const short*>(data) + offset;
        //remaining_data = &remaining_data[index+4];
        unsigned padded_data = 0;
        short padded_array[2] = {0};
        for(unsigned i = 0; i < (remaining_bytes >> 1); ++i) {
          padded_data |= remaining_data[i] << (i << 16);
          padded_array[i] = remaining_data[i];
        }
        // Write data to the stream
        data_line << std::hex << std::setw(5) << device_mem_ptr << " - ";
        for (unsigned i = 0; i < 4; ++i) {
          ConvertToBinary(&binary_string, &padded_array[i], sizeof(short));
          data_line << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(padded_array[0]);
        }
        data_line << " - " << binary_string << std::endl;
        break;
      }
    }
    // Write stream out to file
    asm_out << data_line.str();
    device_mem_ptr += 4;
  }

  asm_out.close();
  addr = device_mem_ptr;
#ifdef DEBUGCL
  std::cerr << "Leaving print_data\n";
#endif

}

bool LE1KernelEvent::run() {
 // TODO Use either simulator or hardware? And use variables
 // instead of hard coded parameters
  bool wasSuccess = false;
  //char* device_name = const_cast<char*>(p_device->model());
  //p_device->getSimulator()->LockAccess();
  wasSuccess = p_device->getSimulator()->Run();
  if (!wasSuccess) {
    pthread_mutex_unlock(&p_mutex);
    return false;
  }

  // If the simulator executes successfully, update the buffers.
  Kernel* TheKernel = p_event->kernel();
  for (unsigned i = 0; i < TheKernel->numArgs(); ++i) {

    const Kernel::Arg& Arg = TheKernel->arg(i);
    if (Arg.kind() == Kernel::Arg::Buffer) {

      LE1Buffer* buffer =
        static_cast<LE1Buffer*>((*(MemObject**)Arg.data())->deviceBuffer(p_device));
      unsigned TotalSize = (*(MemObject**)Arg.data())->size();

      if (Arg.type()->isIntegerTy(8)) {
        p_device->getSimulator()->readCharData(buffer->addr(), TotalSize,
                                               (unsigned char*)buffer->data());
        if (TotalSize == 1)
          std::cerr << "Read back 1 byte: " << (unsigned) *((unsigned char*)(buffer->data()))
            << std::endl;
      }
      // FIXME Shorts aren't handled!

      else if (Arg.type()->isIntegerTy(32)) {
        p_device->getSimulator()->readIntData(buffer->addr(), TotalSize,
                                              (unsigned*)buffer->data());
      }
      // FIXME This only handles aligned structures!
      else if (Arg.type()->isStructTy()) {
        p_device->getSimulator()->readIntData(buffer->addr(), TotalSize,
                                              (unsigned*)buffer->data());
      }
      else {
        std::cerr << " !!! ERROR - unhandled buffer type!!\n";
        wasSuccess = false;
        break;
      }
    }
  }

  p_device->SaveStats(KernelName);
  p_device->getSimulator()->UnlockAccess();

  // Release event
  pthread_mutex_unlock(&p_mutex);
  return wasSuccess;
}

bool LE1KernelEvent::reserve()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::reserve\n";
#endif
    // Lock, this will be unlocked in takeInstance()
    pthread_mutex_lock(&p_mutex);
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelEvent::reserve\n";
#endif

  // Because an event now represents a whole workgroup
  return true;
    // Last work group if current == max - 1
    return (p_current_wg == p_num_wg - 1);
}

bool LE1KernelEvent::finished()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::finished\n";
#endif
    bool rs;

    pthread_mutex_lock(&p_mutex);

    rs = (p_finished_wg == p_num_wg);

    pthread_mutex_unlock(&p_mutex);
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelEvent::finished\n";
#endif

    return true;
}

void LE1KernelEvent::workGroupFinished()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::workGroupFinished\n";
#endif
    pthread_mutex_lock(&p_mutex);

    p_finished_wg++;

    pthread_mutex_unlock(&p_mutex);
#ifdef DEBUGCL
    std::cerr << "Leaving LE1KernelEvent::workGroupFinished\n";
#endif
}

LE1KernelWorkGroup *LE1KernelEvent::takeInstance()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelEvent::takeInstance\n";
#endif
    LE1KernelWorkGroup *wg = new LE1KernelWorkGroup((LE1Kernel *)p_event->deviceKernel(),
                                                    p_event,
                                                    this,
                                                    p_current_work_group);

    // Increment current work group
    //incVec(p_event->work_dim(), p_current_work_group, p_max_work_groups);
    p_current_wg += 1;

    // Release event
    pthread_mutex_unlock(&p_mutex);
#ifdef DEBUGCL
    std::cerr << "Leaving LE1KernelEvent::takeInstance\n";
#endif

    return wg;
}

void *LE1KernelEvent::kernelArgs() const
{
    return p_kernel_args;
}

void LE1KernelEvent::cacheKernelArgs(void *args)
{
    p_kernel_args = args;
}

/*
 * LE1KernelWorkGroup
 */
LE1KernelWorkGroup::LE1KernelWorkGroup(LE1Kernel *kernel, KernelEvent *event,
                                       LE1KernelEvent *cpu_event,
                                       const size_t *work_group_index)
: p_kernel(kernel), p_cpu_event(cpu_event), p_event(event),
  p_work_dim(event->work_dim()), p_contexts(0), p_stack_size(8192 /* TODO */),
  p_had_barrier(false)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelWorkGroup::LE1KernelWorkGroup\n";
#endif

    // Set index
    std::memcpy(p_index, work_group_index, p_work_dim * sizeof(size_t));

    // Set maxs and global id
    p_num_work_items = 1;

    for (unsigned int i=0; i<p_work_dim; ++i)
    {
        p_max_local_id[i] = event->local_work_size(i) - 1; // 0..n-1, not 1..n
        p_num_work_items *= event->local_work_size(i);

        // Set global id
        p_global_id_start_offset[i] = (p_index[i] * event->local_work_size(i))
                         + event->global_work_offset(i);
    }
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelWorkGroup::LE1KernelWorkGroup\n";
#endif
}

LE1KernelWorkGroup::~LE1KernelWorkGroup()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelWorkGroup::~LE1KernelWorkGroup\n";
#endif
    p_cpu_event->workGroupFinished();
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelWorkGroup::~LE1KernelWorkGroup\n";
#endif
}

void *LE1KernelWorkGroup::callArgs(std::vector<void *> &locals_to_free)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelWorkGroup::callArgs\n";
#endif
    if (p_cpu_event->kernelArgs() && !p_kernel->kernel()->hasLocals())
    {
        // We have cached the args and can reuse them
#ifdef DEBUGCL
      std::cerr << "Leaving callArgs because we have cached the args\n";
#endif
        return p_cpu_event->kernelArgs();
    }

    // We need to create them from scratch
    void *rs;

    size_t args_size = 0;

    for (unsigned int i=0; i<p_kernel->kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = p_kernel->kernel()->arg(i);
        //llvm::Type* data_type = arg.type();
        //unsigned width = 0;
        //if(data_type->isIntegerTy()) {
        //const llvm::IntegerType* IntTy =
          // llvm::dyn_cast<llvm::IntegerType>(data_type);
          //width = IntTy->getBitWidth();
          //std::cerr << "Width of arg " << i << " is " << width << std::endl;
        //}
        LE1Kernel::typeOffset(args_size, arg.valueSize() * arg.vecDim());
    }

    rs = std::malloc(args_size);

    if (!rs)
        return NULL;

    size_t arg_offset = 0;

    for (unsigned int i=0; i<p_kernel->kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = p_kernel->kernel()->arg(i);
        size_t size = arg.valueSize() * arg.vecDim();
        size_t offset = LE1Kernel::typeOffset(arg_offset, size);

        // Where to place the argument
        unsigned char *target = (unsigned char *)rs;
        target += offset;

        // We may have to perform some changes in the values (buffers, etc)
        switch (arg.kind())
        {
            case Kernel::Arg::Buffer:
            {
                MemObject *buffer = *(MemObject **)arg.data();

                if (arg.file() == Kernel::Arg::Local)
                {
                    // Alloc a buffer and pass it to the kernel
                    void *local_buffer = std::malloc(arg.allocAtKernelRuntime());
                    locals_to_free.push_back(local_buffer);
                    *(void **)target = local_buffer;
                }
                else
                {
                    if (!buffer)
                    {
                        // We can do that, just send NULL
                        *(void **)target = NULL;
                    }
                    else
                    {
                        // Get the LE1 buffer, allocate it and get its pointer
                        LE1Buffer *cpubuf =
                            (LE1Buffer *)buffer->deviceBuffer(p_kernel->device());
                        void *buf_ptr = 0;

                        buffer->allocate(p_kernel->device());
                        buf_ptr = cpubuf->data();

                        *(void **)target = buf_ptr;
                    }
                }

                break;
            }
            case Kernel::Arg::Image2D:
            case Kernel::Arg::Image3D:
            {
                // We need to ensure the image is allocated
                Image2D *image = *(Image2D **)arg.data();
                image->allocate(p_kernel->device());

                // Fall through to the memcpy
            }
            default:
                // Simply copy the arg's data into the buffer
                std::memcpy(target, arg.data(), size);
                break;
        }
    }

    // Cache the arguments if we can do so
    if (!p_kernel->kernel()->hasLocals())
        p_cpu_event->cacheKernelArgs(rs);

#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelWorkGroup::callArgs\n";
#endif
    return rs;
}

bool LE1KernelWorkGroup::run()
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelWorkGroup::run\n";
#endif
  /*
    // Get the kernel function to call
    std::vector<void *> locals_to_free;
    llvm::Function *kernel_func = p_kernel->callFunction();

    if (!kernel_func)
        return false;

    Program *p = (Program *)p_kernel->kernel()->parent();
    LE1Program *prog = (LE1Program *)(p->deviceDependentProgram(p_kernel->device()));

    p_kernel_func_addr =
        (void(*)(void *))prog->jit()->getPointerToFunction(kernel_func);

    // Get the arguments
    p_args = callArgs(locals_to_free);

    // Tell the builtins this thread will run a kernel work group
    setThreadLocalWorkGroup(this);

    // Initialize the dummy context used by the builtins before a call to barrier()
    p_current_work_item = 0;
    p_current_context = &p_dummy_context;

    std::memset(p_dummy_context.local_id, 0, p_work_dim * sizeof(size_t));

    do
    {
        // Simply call the "call function", it and the builtins will do the rest
        p_kernel_func_addr(p_args);
    } while (!p_had_barrier &&
             !incVec(p_work_dim, p_dummy_context.local_id, p_max_local_id));

    // If no barrier() call was made, all is fine. If not, only the first
    // work-item has currently finished. We must let the others run.
    if (p_had_barrier)
    {
        Context *main_context = p_current_context; // After the first swapcontext,
                                                   // we will not be able to trust
                                                   // p_current_context anymore.

        // We'll call swapcontext for each remaining work-item. They will
        // finish, and when they'll do so, this main context will be resumed, so
        // it's easy (i starts from 1 because the main context already finished)
        for (unsigned int i=1; i<p_num_work_items; ++i)
        {
            Context *ctx = getContextAddr(i);
            swapcontext(&main_context->context, &ctx->context);
        }
    }

    // Free the allocated locals
    if (p_kernel->kernel()->hasLocals())
    {
        for (size_t i=0; i<locals_to_free.size(); ++i)
        {
            std::free(locals_to_free[i]);
        }

        std::free(p_args);
    }
    */
#ifdef DEBUGCL
  std::cerr << "Leaving LE1KernelWorkGroup::run\n";
#endif
    return true;
}

LE1KernelWorkGroup::Context *LE1KernelWorkGroup::getContextAddr(unsigned int index)
{
#ifdef DEBUGCL
  std::cerr << "Entering LE1KernelWorkGroup::getContextAddr\n";
#endif
    size_t size;
    char *data = (char *)p_contexts;

    // Each Context in data is an element of size p_stack_size + sizeof(Context)
    size = p_stack_size + sizeof(Context);
    size *= index;  // To get an offset
#ifdef DEBUGCL
    std::cerr << "Leaving LE1KernelWorkGroup::getContextAddr\n";
#endif

    return (Context *)(data + size); // Pointer to the context
}
