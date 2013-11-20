#include "LE1buffer.h"
#include "LE1DataPrinter.h"
#include "LE1device.h"
#include "LE1NDRange.h"
#include "LE1program.h"
#include "LE1Simulator.h"
#include "coarsener/SourceRewriter.h"

#include "../../compiler.h"
#include "../../deviceinterface.h"
#include "../../events.h"
#include "../../memobject.h"
#include "../../program.h"

#include <llvm/Function.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace Coal;

LE1NDRange::LE1NDRange(KernelEvent *event, LE1Device *device) {
  theDevice = device;
  theKernel = event->kernel();
  theEvent = event;
  workDims = event->work_dim();
  Program *p = (Program*)theKernel->parent();
  LE1Program *prog = (LE1Program *)p->deviceDependentProgram(theDevice);
  OriginalSource = prog->getSource();

  for (unsigned i = 0; i < workDims; ++i) {
    globalWorkSize[i] = event->global_work_size(i);
    localWorkSize[i] = event->local_work_size(i);
  }

  workgroupsPerCore[0] = 1;
  workgroupsPerCore[1] = 1;
  workgroupsPerCore[2] = 1;

  totalCores = device->numLE1s();
  disabledCores = 0;

  KernelName = theKernel->function(theDevice)->getName();
  OriginalSourceName = KernelName + ".cl";
  CoarsenedSourceName = "workgroup_" + KernelName  + ".cl";
  CoarsenedBCName = "workgroup_" + KernelName + ".bc";
  FinalBCName = KernelName + ".bc";
  TempAsmName = "temp_" + KernelName + ".s";
  FinalAsmName = "kernel_" + KernelName + ".s";
  CompleteFilename = "final_" + KernelName + ".s";

  dram = "binaries/final_" + KernelName + ".data.bin";
  iram = "binaries/final_" + KernelName + ".s.bin";
}

bool LE1NDRange::CompileSource() {
#ifdef DBG_NDRANGE
  std::cerr << "Entering CompileSource for " << KernelName << std::endl;
#endif

  // TODO This part needs to calculate how many cores to instantiate
  // Impose an upper limit of 12 cores?
  unsigned mergeDims[3] = {1, 1, 1};

  // Local size may have been set by user, especially in the presence of
  // barriers, so we need to take that into consideration. If the user didn't
  // set a local size, we set it just by global_work_size / num_of_cores. We
  // can adjust the launcher to work with several workgroups in this case.
  // Here we also choose to disable some cores if the data set sizes do not fit
  // properly onto the current configuration.

  for (unsigned i = 0; i < workDims; ++i) {

    if (globalWorkSize[i] > 1) {

      if (localWorkSize[i] > 1) {
        workgroupsPerCore[i] = globalWorkSize[i] / localWorkSize[i];
        mergeDims[i] = localWorkSize[i];
      }
      else {
        mergeDims[i] = globalWorkSize[i] / localWorkSize[i];
        workgroupsPerCore[i] = globalWorkSize[i] / mergeDims[i];
      }

      if (i == 0) {
        if (workgroupsPerCore[i] % totalCores != 0) {

          do {
            ++disabledCores;
          } while (workgroupsPerCore[i] % (totalCores - disabledCores) != 0);

          workgroupsPerCore[i] /= (totalCores - disabledCores);
        }
        else
          workgroupsPerCore[i] /= totalCores;
      }
    }
  }

  // First, write the source string to a file
  std::ofstream SourceFile;
  SourceFile.open(OriginalSourceName.c_str());
  SourceFile << OriginalSource << std::endl;
  SourceFile.close();

  // Then pass the file name to workitem coarsener
  WorkitemCoarsen Coarsener(mergeDims);
  if (!Coarsener.CreateWorkgroup(OriginalSourceName, KernelName))
    return false;

  std::string WorkgroupSource = Coarsener.getFinalKernel();
#ifdef DBG_NDRANGE
  std::cerr << std::endl << WorkgroupSource << std::endl;
#endif

  Compiler LE1Compiler(theDevice);
  std::string Opts = "-funroll-loops ";
  //Opts.append("-mllvm -unroll-threshold=10 ");
  //Opts.append("-mllvm -unroll-count=2 ");
  //Opts.append("-mllvm -unroll-allow-partial ");
  Opts.append("-mllvm -unroll-runtime ");

  if (!LE1Compiler.CompileToBitcode(WorkgroupSource, clang::IK_OpenCL, Opts))
    return false;
  llvm::Module *WorkgroupModule = LE1Compiler.module();

  //LE1Compiler.RunOptimisations(WorkgroupModule);


#ifdef DBG_NDRANGE
  std::cerr << "Merged Kernel\n";
#endif

  //std::string LauncherString;
  CreateLauncher(); //LauncherString, workgroupsPerCore, disabledCores);

  Compiler MainCompiler(theDevice);
  if(!MainCompiler.CompileToBitcode(LauncherString, clang::IK_C, std::string()))
    return false;
  llvm::Module *MainModule = MainCompiler.module();

  // Link the main module with the coarsened kernel code
  llvm::Module *CompleteModule =
    MainCompiler.LinkModules(MainModule, WorkgroupModule);

  MainCompiler.ScanForSoftfloat();

  CompleteModule = MainCompiler.LinkRuntime(CompleteModule);

  //Coarsener.DeleteTempFiles();
  if (!LE1Compiler.ExtractKernelData(CompleteModule, embeddedData))
    return false;

#ifdef DBG_NDRANGE
  std::cerr << "Extracted any embedded data" << std::endl;
#endif

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

#ifdef DBG_NDRANGE
  std::cerr << "Leaving LE1NDRange::CompileSource\n";
#endif

  return true;
}

bool LE1NDRange::RunSim() {
#ifdef DBG_NDRANGE
  std::cerr << "Entering LE1NDRange::RunSim" << std::endl;
#endif
  bool wasSuccess = false;

  LE1Simulator *simulator = theDevice->getSimulator();
  simulator->LockAccess();

  if (!Finalise()) {
    simulator->UnlockAccess();
    return false;
  }

  wasSuccess = simulator->RunSim(iram.c_str(), dram.c_str(), disabledCores);
  if (!wasSuccess) {
    simulator->UnlockAccess();
    return false;
  }

  // If the simulator executes successfully, update the buffers.
  wasSuccess = UpdateBuffers();
  if (!wasSuccess) {
    simulator->UnlockAccess();
    return false;
  }

  theDevice->SaveStats(KernelName);
  simulator->UnlockAccess();
#ifdef DBG_NDRANGE
  std::cerr << "Returning succesfully from RunSim" << std::endl;
#endif
  return true;
}

void LE1NDRange::CreateLauncher(void) {
#ifdef DBG_NDRANGE
  std::cerr << "Entering CreateLauncher with WorkgroupsPerCore = "
    << workgroupsPerCore[0] << ", " << workgroupsPerCore[1] << " and "
    << workgroupsPerCore[2] << std::endl
    << "Number of disabled cores = " << disabledCores << std::endl;
#endif

  // Calculate the addresses in global memory where the arguments will be stored
  std::stringstream launcher;
  for(unsigned i = 0; i < theKernel->numArgs(); ++i) {
    const Kernel::Arg& arg = theKernel->arg(i);
    if (arg.kind() == Kernel::Arg::Buffer) {
      launcher << "extern int BufferArg_" << i << ";" << std::endl;
      //++j;
    }
  }

  launcher << "\nvoid reset_local(int *buffer, int size);\n\n";

  // Create a main function to the launcher for the kernel
  launcher << "int main(void) {\n";

  if (disabledCores)
    launcher << "  if (__builtin_le1_read_cpuid() > "
      << (totalCores - disabledCores - 1) << ") return 0;\n" << std::endl;

  unsigned NestedLoops = 0;
  if (workgroupsPerCore[2] != 0) {
    launcher << "  for (unsigned z = 0; z < " << workgroupsPerCore[2]
      << "; ++z) {\n"
    <<      "   __builtin_le1_set_group_id_2(z);\n";
    ++NestedLoops;
  }
  if (workgroupsPerCore[1] != 0) {
    launcher << "    for (unsigned y = 0; y < " << workgroupsPerCore[1]
      << "; ++y) {\n"
    << "      __builtin_le1_set_group_id_1(y);\n";
    ++NestedLoops;
  }
  if (workgroupsPerCore[0] != 0) {
    launcher << "      for (unsigned x = 0; x < " << workgroupsPerCore[0]
      << "; ++x) {\n"
    << "        __builtin_le1_set_group_id_0(x + " << workgroupsPerCore[0]
    << " * __builtin_le1_read_cpuid());\n";
    ++NestedLoops;
  }
  launcher<< "        " << KernelName << "(";

  for (unsigned i = 0; i < theKernel->numArgs(); ++i) {
    const Kernel::Arg& arg = theKernel->arg(i);
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
    if (i < (theKernel->numArgs()-1))
      launcher << ", ";
    else {
      launcher << ");\n";

      for (unsigned i = 0; i < theKernel->numArgs(); ++i) {
        const Kernel::Arg& arg = theKernel->arg(i);
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
#ifdef DBG_NDRANGE
  std::cerr << LauncherString << std::endl;
#endif
}

bool LE1NDRange::Finalise() {
  std::string CopyCommand = "cp " + FinalAsmName + " " + CompleteFilename;
  system(CopyCommand.c_str());
  LE1DataPrinter dataPrinter(theDevice, embeddedData, theEvent,
                             CompleteFilename.c_str());

  if (!dataPrinter.AppendDataArea())
    return false;

  std::string assemble = "perl " + LE1Device::ScriptsDir + "secondpass.pl ";
  assemble.append(CompleteFilename);
  assemble.append(" -OPC=").append(LE1Device::IncDir + "opcodes.txt");

  if (system(assemble.c_str()) != 0)
    return false;

  return true;
}

bool LE1NDRange::UpdateBuffers() {
#ifdef DBG_NDRANGE
  std::cerr << "Entering LE1NDRange::UpdateBuffers" << std::endl;
#endif

  bool wasSuccess = true;
  LE1Simulator *simulator = theDevice->getSimulator();

  for (unsigned i = 0; i < theKernel->numArgs(); ++i) {

    const Kernel::Arg& Arg = theKernel->arg(i);
    if ((Arg.kind() == Kernel::Arg::Buffer) &&
        (Arg.file() != Kernel::Arg::Local)) {

#ifdef DBG_NDRange
      std::cerr << "Updating kernel arg " << i << std::endl;
#endif
      LE1Buffer* buffer = static_cast<LE1Buffer*>(
        (*(MemObject**)Arg.data())->deviceBuffer(theDevice));

      if (buffer->data() == NULL) {
        std::cerr << "!ERROR : Arg " << i << " buffer data is NULL"
          << std::endl;
        return false;
      }

      unsigned TotalSize = (*(MemObject**)Arg.data())->size();
      llvm::Type *argType = Arg.type();

      if (argType->isIntegerTy(8)) {
        simulator->readByteData(buffer->addr(), TotalSize,
                                (unsigned char*)buffer->data());
      }
      else if (argType->isIntegerTy(16)) {
        simulator->readHalfData(buffer->addr(), TotalSize,
                                (unsigned short*)buffer->data());
      }
      else if (argType->isIntegerTy(32)) {
        simulator->readWordData(buffer->addr(), TotalSize,
                                (unsigned*)buffer->data());
      }
      else if (argType->getTypeID() == llvm::Type::FloatTyID)
        simulator->readWordData(buffer->addr(), TotalSize,
                                (unsigned*)buffer->data());

      // FIXME This will be really slow since it reads an element at a time.
      else if (argType->isStructTy()) {
        llvm::StructType* StructArg = static_cast<llvm::StructType*>(argType);
#ifdef DBG_RANGE
        std::cerr << "Arg is Struct with " << StructArg->getNumElements()
          << " elements" << std::endl;
#endif

        // Use one call to the simulator to read data if possible
        llvm::Type *prevElement = StructArg->getElementType(0);
        bool uniformTypes = true;
        for (unsigned i = 1; i < StructArg->getNumElements(); ++i) {
          if (prevElement != StructArg->getElementType(i)) {
            uniformTypes = false;
            break;
          }
        }
        if (uniformTypes) {
#ifdef DBG_RANGE
          std::cerr << "Struct contains just one type" << std::endl;
#endif
          if (prevElement->isIntegerTy(8))
            simulator->readByteData(buffer->addr(), TotalSize,
                                (unsigned char*)buffer->data());
          else if (prevElement->isIntegerTy(16))
            simulator->readHalfData(buffer->addr(), TotalSize,
                                (unsigned short*)buffer->data());
          else if (prevElement->isIntegerTy(32))
            simulator->readWordData(buffer->addr(), TotalSize,
                                (unsigned*)buffer->data());
          else if (prevElement->isFloatTy())
            simulator->readWordData(buffer->addr(), TotalSize,
                                    (unsigned*)buffer->data());
          else {
            std::cerr << "!!ERROR: Unhandled struct element type"
              << std::endl;
            return false;
          }
        }
        else {

          unsigned finalAddr = buffer->addr() + TotalSize;
          unsigned readAddr = buffer->addr();

          while (readAddr < finalAddr) {

            // FIXME Is this setup for correct packing? Does +readAddr work??
            for (llvm::StructType::element_iterator EI
                 = StructArg->element_begin(), EE = StructArg->element_end();
                 EI != EE; ++EI) {

              llvm::Type *elementType = *EI;

              if (elementType->isIntegerTy(8)) {
                simulator->readByteData(readAddr, 1,
                                    (unsigned char*)buffer->data() + readAddr);
                ++readAddr;
              }
              if (elementType->isIntegerTy(16)) {
                if (readAddr % 2 != 0)
                  ++readAddr;

                simulator->readHalfData(readAddr, 2,
                                    (unsigned short*)buffer->data() + readAddr);
                readAddr += 2;
              }
              if (elementType->isIntegerTy(32)) {
                if (readAddr % 4 != 0)
                  readAddr += readAddr % 4;

                simulator->readWordData(readAddr, 4,
                                      (unsigned*)buffer->data() + readAddr);
                readAddr += 4;
              }
              if (elementType->isFloatTy()) {
                if (readAddr % 4 != 0)
                  readAddr += readAddr % 4;

                simulator->readWordData(readAddr, 4,
                                      (unsigned*)buffer->data() + readAddr);
                readAddr += 4;
              }
            }
          }
        }
      }

      else if (argType->getTypeID() == llvm::Type::VectorTyID) {
#ifdef DBG_NDRANGE
        std::cerr << "Data is vector type\n";
#endif
        llvm::Type* elementType =
          static_cast<llvm::VectorType*>(Arg.type())->getElementType();

        if (elementType->getTypeID() == llvm::Type::IntegerTyID) {
#ifdef DBG_NDRANGE
          std::cerr << "Elements are integers\n";
#endif
          if (elementType->isIntegerTy(8)) {
            simulator->readByteData(buffer->addr(), TotalSize,
                                    (unsigned char*)buffer->data());
          }
          else if (elementType->isIntegerTy(16)) {
            simulator->readHalfData(buffer->addr(), TotalSize,
                                    (unsigned short*)buffer->data());
          }
          else if (elementType->isIntegerTy(32)) {
            simulator->readWordData(buffer->addr(), TotalSize,
                                   (unsigned*)buffer->data());
          }
        }
        else {
          std::cerr << " !!! ERROR - unhandled vector buffer type!!\n";
          wasSuccess = false;
          break;
        }
      }
      else {
        std::cerr << " !!! ERROR - unhandled buffer type!!\n";
        wasSuccess = false;
        break;
      }
    }
  }

  return wasSuccess;
}
