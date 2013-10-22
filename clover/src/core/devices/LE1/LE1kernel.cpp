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
#include "LE1DataPrinter.h"
#include "LE1device.h"
#include "LE1buffer.h"
#include "LE1program.h"
#include "LE1Simulator.h"
#include "coarsener/SourceRewriter.h"
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

// FIXME This probably needs to match device->info
size_t LE1Kernel::workGroupSize() const
{
  return 256;
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
#ifdef DBG_KERNEL
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

#ifdef DBG_KERNEL
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

//unsigned int LE1KernelEvent::addr = 4;
std::vector<LE1Buffer*> LE1KernelEvent::DeviceBuffers;

LE1KernelEvent::LE1KernelEvent(LE1Device *device, KernelEvent *event)
: p_device(device), p_event(event), p_current_wg(0), p_finished_wg(0),
  p_kernel_args(0)
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::LE1KernelEvent\n";
#endif

#ifdef DBG_KERNEL
  std::cerr << "mutex_init\n";
#endif
    // Mutex
    pthread_mutex_init(&p_mutex, 0);

#ifdef DBG_KERNEL
  std::cerr << "memset\n";
#endif
    // Set current work group to (0, 0, ..., 0)
    std::memset(p_current_work_group, 0, event->work_dim() * sizeof(size_t));

    // Populate p_max_work_groups
    p_num_wg = 1;

    for (cl_uint i=0; i<event->work_dim(); ++i)
    {
#ifdef DBG_KERNEL
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
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelEvent::LE1KernelEvent\n";
#endif
}

LE1KernelEvent::~LE1KernelEvent()
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::~LE1KernelEvent\n";
#endif
    pthread_mutex_destroy(&p_mutex);

    if (p_kernel_args)
        std::free(p_kernel_args);

#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelEvent::~LE1KernelEvent\n";
#endif
}

// If there are multiple devices in the system, buffer allocation is delayed
// until the first run. Check all the buffer argument to check whether they
// are allocated and do so if not.
bool LE1KernelEvent::AllocateBuffers() {
  Kernel* TheKernel = p_event->kernel();

  for (unsigned i = 0; i < TheKernel->numArgs(); ++i) {

    const Kernel::Arg& Arg = TheKernel->arg(i);

    if ((Arg.kind() == Kernel::Arg::Buffer) &&
        (Arg.file() != Kernel::Arg::Local)) {

#ifdef DBG_KERNEL
  std::cerr << "Allocating buffer for arg " << i << std::endl;
#endif

      LE1Buffer* buffer = static_cast<LE1Buffer*>(
        (*(MemObject**)Arg.data())->deviceBuffer(p_device));

      if (buffer->data() == NULL) {
        if (!((*(MemObject**)Arg.data())->allocate(p_device)))
          return false;
      }
    }
  }
  return true;
}

bool LE1KernelEvent::createFinalSource(LE1Program *prog) {
  // TODO Create a proper .s file:
#ifdef DBG_KERNEL
  std::cerr << "Entering createFinalSource: ";
#endif

  KernelName = p_event->kernel()->function(p_device)->getName();
#ifdef DBG_KERNEL
  std::cerr << KernelName << std::endl;
#endif
  OriginalSource = prog->getSource();
  OriginalSourceName = KernelName + ".cl";
  CoarsenedSourceName = "workgroup_" + KernelName  + ".cl";
  CoarsenedBCName = "workgroup_" + KernelName + ".bc";
  FinalBCName = KernelName + ".bc";
  TempAsmName = "temp_" + KernelName + ".s";
  FinalAsmName = "kernel_" + KernelName + ".s";
  CompleteFilename = "final_" + KernelName + ".s";

  //CalculateBufferAddrs();

  /*
  // Only compile a kernel once
  if (!p_event->kernel()->isBuilt()) {
    // FIXME Change this to the kernel specific name
    //if(!CompileSource("program.bc", filename))
    if(!CompileSource())
      return false;
  }
  else {
#ifdef DBG_KERNEL
    std::cerr << "Program has already been compiled\n";
#endif
  }*/
  if(!CompileSource())
    return false;

  p_event->kernel()->SetBuilt();
#if DBG_KERNEL
  std::cerr << "Successfully leaving createFinalSource" << std::endl;
#endif

  return true;

  /*
  if (!WriteDataArea())
    return false;

  std::string assemble = "perl " + LE1Device::ScriptsDir + "secondpass.pl ";
  assemble.append(CompleteFilename);
  assemble.append(" -OPC=").append(LE1Device::IncDir + "opcodes.txt");

  if (system(assemble.c_str()) != 0)
    return false;
  else {
    p_event->kernel()->SetBuilt();
    return true;
  }*/
}


bool LE1KernelEvent::CompileSource() {
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::CompileSource for " << KernelName
    << std::endl;
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
  if (!Coarsener.CreateWorkgroup(OriginalSourceName, p_event->kernel()->name()))
    return false;

  std::string WorkgroupSource = Coarsener.getFinalKernel();
#ifdef DBG_KERNEL
  std::cerr << std::endl << WorkgroupSource << std::endl;
#endif

  Compiler LE1Compiler(p_device);
  std::string Opts = "";
  if (!LE1Compiler.CompileToBitcode(WorkgroupSource, clang::IK_OpenCL, Opts))
    return false;
  llvm::Module *WorkgroupModule = LE1Compiler.module();

  //Coarsener.DeleteTempFiles();

#ifdef DBG_KERNEL
  std::cerr << "Merged Kernel\n";
#endif

  std::string LauncherString;
  CreateLauncher(LauncherString, WorkgroupsPerCore);

  Compiler MainCompiler(p_device);
  if(!MainCompiler.CompileToBitcode(LauncherString, clang::IK_C, std::string()))
    return false;
  llvm::Module *MainModule = MainCompiler.module();

  // Link the main module with the coarsened kernel code
  llvm::Module *CompleteModule =
    MainCompiler.LinkModules(MainModule, WorkgroupModule);

  // Output a single assembly file
  if(!MainCompiler.CompileToAssembly(TempAsmName,
                                     CompleteModule))
    return false;

  std::stringstream pre_asm_command;
  // TODO Include the script as a char array
  pre_asm_command << "perl " << LE1Device::ScriptsDir << "llvmTransform.pl -syscall "
    << TempAsmName
    //<< " -OPC=/home/sam/Dropbox/src/LE1/Assembler/includes/opcodes.txt_asm "
    << " > " << FinalAsmName;
  // FIXME return false?
  if (system(pre_asm_command.str().c_str()) != 0) {
    std::cerr << "LLVM Transform failed\n";
    return false;
  }

#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelEvent::CompileSource\n";
#endif

  p_event->kernel()->SetBuilt();
  return true;
}

void LE1KernelEvent::CreateLauncher(std::string &LauncherString,
                                    unsigned *WorkgroupsPerCore) {
  // Calculate the addresses in global memory where the arguments will be stored
  Kernel* kernel = p_event->kernel();

  std::stringstream launcher;
  for(unsigned i = 0; i < kernel->numArgs(); ++i) {
    const Kernel::Arg& arg = kernel->arg(i);
    if (arg.kind() == Kernel::Arg::Buffer) {
      launcher << "extern int BufferArg_" << i << ";" << std::endl;
      //++j;
    }
  }

  launcher << "\nvoid reset_local(int *buffer, int size);\n\n";

  // Create a main function to the launcher for the kernel
  launcher << "int main(void) {\n";
  unsigned NestedLoops = 0;
  if (WorkgroupsPerCore[2] != 0) {
    launcher << "  for (unsigned z = 0; z < " << WorkgroupsPerCore[2]
      << "; ++z) {\n"
    <<      "   __builtin_le1_set_group_id_2(z);\n";
    ++NestedLoops;
  }
  if (WorkgroupsPerCore[1] != 0) {
    launcher << "    for (unsigned y = 0; y < " << WorkgroupsPerCore[1]
      << "; ++y) {\n"
    << "      __builtin_le1_set_group_id_1(y);\n";
    ++NestedLoops;
  }
  if (WorkgroupsPerCore[0] != 0) {
    launcher << "      for (unsigned x = 0; x < " << WorkgroupsPerCore[0]
      << "; ++x) {\n"
    << "        __builtin_le1_set_group_id_0(x);\n";
    ++NestedLoops;
  }
  launcher<< "        " << KernelName << "(";

  for (unsigned i = 0; i < kernel->numArgs(); ++i) {
    const Kernel::Arg& arg = kernel->arg(i);
    // Local
    if ((arg.kind() == Kernel::Arg::Buffer) &&
        arg.allocAtKernelRuntime()) {
      // We're defining the pointers as ints, so divide
      // FIXME what if the size isn't divisible by four?
      unsigned size = arg.allocAtKernelRuntime() / 4;
      launcher << "(&BufferArg_" << i << " + (__builtin_le1_read_cpuid() * "
        << size << "))";
    }
    // Global and local
    else if (arg.kind() == Kernel::Arg::Buffer)
      launcher << "&BufferArg_" << i;
    // Private
    else {
      void *ArgData = const_cast<void*>(arg.data());
      launcher << *(static_cast<unsigned*>(ArgData));
    }
    if (i < (kernel->numArgs()-1))
      launcher << ", ";
    else {
      launcher << ");\n";

      for (unsigned i = 0; i < kernel->numArgs(); ++i) {
        const Kernel::Arg& arg = kernel->arg(i);
        if (arg.file() == Kernel::Arg::Local) {
          unsigned size = arg.allocAtKernelRuntime() / 4;
          launcher << "      reset_local(&BufferArg_" << i << ", "
            << size << ");\n";
        }
      }

      for (unsigned i = 0; i < NestedLoops; ++i)
        launcher << "}\n";
      launcher << "return 0;\n}";
    }
  }

  launcher << "\nvoid reset_local(int *buffer, int size) {\n"
    << "  for (unsigned id = 0; id < size; ++id)\n"
    << "    buffer[id] = 0;\n"
    << "}\n";

  LauncherString = launcher.str();
#ifdef DBG_KERNEL
  std::cerr << LauncherString << std::endl;
#endif
}



bool LE1KernelEvent::run() {
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::run" << std::endl;
#endif
 // TODO Use either simulator or hardware? And use variables
 // instead of hard coded parameters
  bool wasSuccess = false;
  //char* device_name = const_cast<char*>(p_device->model());
  LE1Simulator *simulator = p_device->getSimulator();
  simulator->LockAccess();

  std::string CopyCommand = "cp " + FinalAsmName + " " + CompleteFilename;
  system(CopyCommand.c_str());
  LE1DataPrinter dataPrinter(p_device, p_event,
                             CompleteFilename.c_str());

  if (!dataPrinter.AppendDataArea()) {
    pthread_mutex_unlock(&p_mutex);
    simulator->UnlockAccess();
    return false;
  }

  std::string assemble = "perl " + LE1Device::ScriptsDir + "secondpass.pl ";
  assemble.append(CompleteFilename);
  assemble.append(" -OPC=").append(LE1Device::IncDir + "opcodes.txt");

  if (system(assemble.c_str()) != 0) {
    simulator->UnlockAccess();
    return false;
  }

  std::string dram = "binaries/final_" + KernelName + ".data.bin";
  std::string iram = "binaries/final_" + KernelName + ".s.bin";

  wasSuccess = simulator->Run(iram.c_str(), dram.c_str());
  if (!wasSuccess) {
    pthread_mutex_unlock(&p_mutex);
    simulator->UnlockAccess();
    return false;
  }

  // If the simulator executes successfully, update the buffers.
  Kernel* TheKernel = p_event->kernel();
  for (unsigned i = 0; i < TheKernel->numArgs(); ++i) {

    const Kernel::Arg& Arg = TheKernel->arg(i);
    if ((Arg.kind() == Kernel::Arg::Buffer) &&
        (Arg.file() != Kernel::Arg::Local)) {

#ifdef DBG_KERNEL
      std::cerr << "Updating kernel arg " << i << std::endl;
#endif
      LE1Buffer* buffer =
        static_cast<LE1Buffer*>((*(MemObject**)Arg.data())->deviceBuffer(p_device));

      if (buffer->data() == NULL) {
        std::cerr << "!ERROR : Arg " << i << " buffer data is NULL"
          << std::endl;
        return false;
      }

      unsigned TotalSize = (*(MemObject**)Arg.data())->size();

      if (Arg.type()->isIntegerTy(8)) {
        p_device->getSimulator()->readCharData(buffer->addr(), TotalSize,
                                               (unsigned char*)buffer->data());
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
  simulator->UnlockAccess();

  // Release event
  pthread_mutex_unlock(&p_mutex);
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelEvent::run" << std::endl;
#endif
  return wasSuccess;
}

bool LE1KernelEvent::reserve()
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::reserve\n";
#endif
    // Lock, this will be unlocked in takeInstance()
    pthread_mutex_lock(&p_mutex);
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelEvent::reserve\n";
#endif

  // Because an event now represents a whole workgroup
  return true;
    // Last work group if current == max - 1
    return (p_current_wg == p_num_wg - 1);
}

bool LE1KernelEvent::finished()
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::finished\n";
#endif
    bool rs;

    pthread_mutex_lock(&p_mutex);

    rs = (p_finished_wg == p_num_wg);

    pthread_mutex_unlock(&p_mutex);
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelEvent::finished\n";
#endif

    return true;
}

void LE1KernelEvent::workGroupFinished()
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelEvent::workGroupFinished\n";
#endif
    pthread_mutex_lock(&p_mutex);

    p_finished_wg++;

    pthread_mutex_unlock(&p_mutex);
#ifdef DBG_KERNEL
    std::cerr << "Leaving LE1KernelEvent::workGroupFinished\n";
#endif
}

LE1KernelWorkGroup *LE1KernelEvent::takeInstance()
{
#ifdef DBG_KERNEL
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
#ifdef DBG_KERNEL
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
#ifdef DBG_KERNEL
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
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelWorkGroup::LE1KernelWorkGroup\n";
#endif
}

LE1KernelWorkGroup::~LE1KernelWorkGroup()
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelWorkGroup::~LE1KernelWorkGroup\n";
#endif
    p_cpu_event->workGroupFinished();
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelWorkGroup::~LE1KernelWorkGroup\n";
#endif
}

void *LE1KernelWorkGroup::callArgs(std::vector<void *> &locals_to_free)
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelWorkGroup::callArgs\n";
#endif
    if (p_cpu_event->kernelArgs() && !p_kernel->kernel()->hasLocals())
    {
        // We have cached the args and can reuse them
#ifdef DBG_KERNEL
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
#ifdef DBG_KERNEL
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
#ifdef DBG_KERNEL
  std::cerr << "Leaving LE1KernelWorkGroup::run\n";
#endif
    return true;
}

LE1KernelWorkGroup::Context *LE1KernelWorkGroup::getContextAddr(unsigned int index)
{
#ifdef DBG_KERNEL
  std::cerr << "Entering LE1KernelWorkGroup::getContextAddr\n";
#endif
    size_t size;
    char *data = (char *)p_contexts;

    // Each Context in data is an element of size p_stack_size + sizeof(Context)
    size = p_stack_size + sizeof(Context);
    size *= index;  // To get an offset
#ifdef DBG_KERNEL
    std::cerr << "Leaving LE1KernelWorkGroup::getContextAddr\n";
#endif

    return (Context *)(data + size); // Pointer to the context
}
