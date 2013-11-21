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
 * \file compiler.cpp
 * \brief Compiler wrapper around Clang
 */

#include "compiler.h"
#include "deviceinterface.h"
#include "embedded_data.h"
#include "devices/LE1/creduce/TransformationManager.h"
#include "devices/LE1/creduce/SimpleInliner.h"

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendOptions.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/LangStandard.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/CodeGen/BackendUtil.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Rewrite/Frontend/FrontendActions.h>
#include <llvm/Constants.h>
#include <llvm/DataLayout.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instruction.h>
#include <llvm/IntrinsicInst.h>
#include <llvm/Linker.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Scalar.h>

using namespace Coal;
using namespace clang;
using namespace llvm;

Compiler::Compiler(DeviceInterface *device)
: p_device(device), p_module(0), p_optimize(true), p_log_stream(p_log),
  p_log_printer(0)
{
  Triple = device->getTriple();
  CPU = device->getCPU();
#ifdef DBG_COMPILER
  std::cerr << "Constructing Compiler::Compiler for " << Triple << " "
    << CPU << std::endl;
#endif

  //p_compiler.getCodeGenOpts().BackendOptions.push_back("unroll-count=2");

}

Compiler::~Compiler()
{
#ifdef DBG_COMPILER
  std::cerr << "Destructing Compiler::Compiler\n";
#endif
  //TransformationManager::Finalize();
}


// Use a RewriteAction to expand all macros within the original source file
bool Compiler::ExpandMacros(const char *filename) {
#ifdef DBG_COMPILER
  std::cerr << "Entering Compiler::ExpandMacros" << std::endl;
#endif
  std::string log;
  llvm::raw_string_ostream macro_log(log);
  CompilerInstance CI;
  RewriteMacrosAction act;

  CI.getTargetOpts().Triple = Triple;
  CI.getFrontendOpts().Inputs.push_back(FrontendInputFile(filename,
                                                          IK_OpenCL));
  // Overwrite original
  CI.getFrontendOpts().OutputFile = filename;

  CI.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDE_DIR,
                                   frontend::Angled,
                                   false, false, false);
  CI.getHeaderSearchOpts().AddPath(CLANG_RESOURCE_DIR, frontend::Angled,
                                   false, false, false);
  CI.getHeaderSearchOpts().ResourceDir = CLANG_RESOURCE_DIR;
  CI.getPreprocessorOpts().Includes.push_back("clc/clc.h");
  CI.getPreprocessorOpts().addMacroDef(
    "cl_clang_storage_class_specifiers");
  CI.getInvocation().setLangDefaults(CI.getLangOpts(), IK_OpenCL);

  CI.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
      macro_log, &CI.getDiagnosticOpts()));

  if (!CI.ExecuteAction(act)) {
    std::cerr << "RewriteMacrosAction failed:" << std::endl
      << log;
    return false;
  }

  // FIXME Took this out because it broke sort. Think it's because it inserts:
  // # 1 esdg_src.cl at the top
  /*
  RewriteIncludesAction rewriteIncludesAct;
  if (!CI.ExecuteAction(rewriteIncludesAct)) {
    std::cerr << "RewriteIncludes failed: " << std::endl
      << log;
    return false;
  }*/

#ifdef DBG_COMPILER
  std::cerr << "Successfully leaving Compiler::ExpandMacros" << std::endl;
#endif
  return true;
}

int Compiler::InlineSource(const char *filename) {
#ifdef DBG_COMPILER
  std::cerr << "Entering Compiler::InlineSource for " << filename << std::endl;
#endif
  std::string err;
  TransformationManager *TM = TransformationManager::GetInstance();

  // First check whether there are any functions to inline
  SimpleInliner *simpleInliner = new SimpleInliner("simple-inliner", "inline");
  TM->setTransformation(simpleInliner);
  TM->setSrcFileName(filename);
  TM->setOutputFileName(filename);
  TM->setQueryInstanceFlag(true);
  TM->setTransformationCounter(1);

  if (!TM->initializeCompilerInstance(err)) {
    std::cerr << "!!ERROR: Initialising compiler failed!" << std::endl
      << err << std::endl;
    return -1;
  }

  if (!TM->doTransformation(err)) {
    std::cerr << "!!ERROR: " << err << std::endl;
    return -1;
  }

  int numInstances = TM->getNumTransformationInstances();
#ifdef DBG_COMPILER
  std::cerr << "Need to inline " << numInstances << " functions"
    << std::endl;
#endif

  if (numInstances != 0) {

    int completed = 0;
    while (completed != numInstances) {
      TM->reset();
      simpleInliner = new SimpleInliner("simple-inliner", "inline");
      TM->setTransformation(simpleInliner);
      TM->setSrcFileName(filename);
      TM->setOutputFileName(filename);
      TM->setQueryInstanceFlag(false);
      TM->setTransformationCounter(1);

      if (!TM->initializeCompilerInstance(err)) {
        std::cerr << "!!ERROR: Initialising compiler failed!" << std::endl
          << err << std::endl;
        return -1;
      }

      if (!TM->doTransformation(err)) {
        std::cerr << "Inline failed!:" << std::endl << err << std::endl;
        return -1;
      }
      ++completed;
    }
  }

  std::ifstream inlined_file(filename);
  std::string str((std::istreambuf_iterator<char>(inlined_file)),
                  std::istreambuf_iterator<char>());
  FinalSource.assign(str);

  TransformationManager::Finalize();

  return 0;
}

bool Compiler::CompileToBitcode(std::string &Source,
                                clang::InputKind SourceKind,
                                const std::string &Opts) {
#ifdef DBG_COMPILER
  std::cerr << "Entering CompileToBitcode:" << std::endl << Opts << std::endl;
#endif
  //clang::CompilerInvocation Invocation;
  clang::EmitLLVMOnlyAction act(&llvm::getGlobalContext());
  std::string log;
  llvm::raw_string_ostream s_log(log);

  // Parse the compiler options
  std::vector<std::string> opts_array;
  std::istringstream ss(Opts);

  while (!ss.eof()) {
    std::string opt;
    getline(ss, opt, ' ');
    opts_array.push_back(opt);
  }

  // opts_array.push_back(name);

  std::vector<const char*> opts_carray;
  for (unsigned i = 0; i < opts_array.size(); i++) {
    opts_carray.push_back(opts_array.at(i).c_str());
  }

  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID;
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts;
  clang::TextDiagnosticBuffer *DiagBuffer;

  DiagID = new clang::DiagnosticIDs();
  DiagOpts = new clang::DiagnosticOptions();
  DiagBuffer = new clang::TextDiagnosticBuffer();

  clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagBuffer);
  bool Success;
  Success =
    clang::CompilerInvocation::CreateFromArgs(p_compiler.getInvocation(),
                                        opts_carray.data(),
                                        opts_carray.data() + opts_carray.size(),
                                        Diags);

  if (!Success) {
    std::cerr << "!ERROR: Could not create CompilerInvocation!\n";
    return false;
  }

  std::string TempFilename;
  if (SourceKind == clang::IK_OpenCL) {
    TempFilename = "temp.cl";
    // Add libclc search path
    p_compiler.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDE_DIR,
                                             clang::frontend::Angled,
                                             false, false, false);
    // Add libclc include
    p_compiler.getPreprocessorOpts().Includes.push_back("clc/clc.h");

    // clc.h requires that this macro be defined
    p_compiler.getPreprocessorOpts().addMacroDef(
      "cl_clang_storage_class_specifiers");

    p_compiler.getCodeGenOpts().LinkBitcodeFile =
      "/opt/esdg-opencl/lib/builtins.bc";
  }
  else {
    TempFilename = "temp.c";
  }

  p_compiler.getFrontendOpts().Inputs.clear();
  p_compiler.getPreprocessorOpts().RemappedFiles.clear();
  p_compiler.getPreprocessorOpts().RemappedFileBuffers.clear();

  p_compiler.getFrontendOpts().Inputs.push_back(
    clang::FrontendInputFile(TempFilename, SourceKind));
  p_compiler.getFrontendOpts().ProgramAction = clang::frontend::EmitLLVMOnly;

  //p_compiler.getFrontendOpts().OutputFile = OutputFile;

  p_compiler.getHeaderSearchOpts().UseBuiltinIncludes = true;
  p_compiler.getHeaderSearchOpts().UseStandardSystemIncludes = false;
  p_compiler.getHeaderSearchOpts().ResourceDir = CLANG_RESOURCE_DIR;

  p_compiler.getHeaderSearchOpts().AddPath(CLANG_RESOURCE_DIR,
                                           clang::frontend::Angled,
                                           false, false, false);

  p_compiler.getCodeGenOpts().OptimizationLevel = 3;
  p_compiler.getCodeGenOpts().SimplifyLibCalls = false;
  p_compiler.getCodeGenOpts().setInlining(
    clang::CodeGenOptions::OnlyAlwaysInlining);

  p_compiler.getLangOpts().NoBuiltin = false;
  p_compiler.getTargetOpts().Triple = Triple;
  p_compiler.getInvocation().setLangDefaults(p_compiler.getLangOpts(),
                                             SourceKind);

  //llvm::MemoryBuffer Buffer = llvm::MemoryBuffer::getMemBuffer(Source);
  p_compiler.getPreprocessorOpts()
    .addRemappedFile(TempFilename, llvm::MemoryBuffer::getMemBuffer(Source));

  p_compiler.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
    s_log, &p_compiler.getDiagnosticOpts()));

  // Compile the code
  if (!p_compiler.ExecuteAction(act)) {
#ifdef DBG_COMPILER
    std::cerr << "Compilation Failed\n";
    std::cerr << log;
#endif
    return false;
  }
  p_module = act.takeModule();

    //PrevKernels.push_back(name);
#ifdef DBG_COMPILER
  p_module->dump();
  std::cerr << "Leaving Compiler::CompileToBitcode\n";
#endif
    return true;

}

llvm::Module *Compiler::LinkModules(llvm::Module *m1, llvm::Module *m2) {
#ifdef DBG_COMPILER
  std::cerr << "Entering LinkModules\n";
#endif
  llvm::StringRef progName("MyLinker");
  llvm::Linker ld(progName, m1);
  if ( ld.LinkInModule(m2) ) {
    return NULL;
  }

#ifdef DBG_COMPILER
  std::cerr << "Leaving LinkModules\n";
#endif
  return ld.releaseModule();
}

llvm::Module *Compiler::LinkRuntime(llvm::Module *M) {
#ifdef DBG_COMPILER
  std::cerr << "Entering LinkRuntime" << std::endl;
#endif

  llvm::StringRef progName("MyLinker");
  llvm::Linker ld(progName, M);

  bool isNative = false;
  if (ld.LinkInFile(llvm::sys::Path("/opt/esdg-opencl/lib/builtins.bc"),
                     isNative)) {
    return NULL;
  }

  return ld.releaseModule();

}

void Compiler::ScanForSoftfloat() {
#ifdef DBG_COMPILER
  std::cerr << "Compiler::ScanForSoftFloat" << std::endl << std::endl;
  p_module->dump();
  std::cerr << std::endl;
#endif
  for (llvm::Module::iterator FI = p_module->begin(),
    FE = p_module->end(); FI != FE; ++FI) {

    llvm::Function *F = FI;

    std::cerr << "Iterating through function:" << F->getName().str()
      << std::endl;

    // Iterate through all the basic block of the function
    for (llvm::Function::BasicBlockListType::iterator BI = F->begin(),
         BE = F->end(); BI != BE; ++BI) {

      llvm::BasicBlock *B = BI;

      // Visit all the instructions witin the basic block
      for (llvm::BasicBlock::InstListType::iterator II = B->begin(),
           IE = B->end(); II != IE; ++II) {

        llvm::Instruction *I = II;
        StringRef FuncName;
        llvm::FunctionType *FuncType; //IntegerType::getInt32Ty(C);
        std::vector<llvm::Type*> ParamTys;
        llvm::Type *Int32Ty =
          llvm::Type::getInt32Ty(p_module->getContext());
        llvm::Attributes Attrs;
          //= llvm::Attributes::get(CGM.getLLVMContext(),
            //                      llvm::Attributes::NoUnwind);

        if (II->getType()->getTypeID() == llvm::Type::FloatTyID) {
          std::cerr << "Found FP instruction" << std::endl;

          switch(I->getOpcode()) {
          default:
#ifdef DBG_COMPILER
            std::cerr << "UNHANDLED FP INSTRUCTION: " << I->getOpcodeName()
              << std::endl;
#endif
            break;
          case llvm::Instruction::FAdd:
            FuncName = "float32_add";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            p_module->getOrInsertFunction(FuncName, FuncType);
            break;
          case llvm::Instruction::FSub:
            FuncName = "float32_sub";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            p_module->getOrInsertFunction(FuncName, FuncType);
            break;
          case llvm::Instruction::FMul:
            FuncName = "float32_mul";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            p_module->getOrInsertFunction(FuncName, FuncType);
            break;
          case llvm::Instruction::FDiv:
            FuncName = "float32_div";
            ParamTys.push_back(Int32Ty);
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            p_module->getOrInsertFunction(FuncName, FuncType);
            break;
          case llvm::Instruction::UIToFP:
          case llvm::Instruction::SIToFP:
            FuncName = "int32_to_float32";
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            p_module->getOrInsertFunction(FuncName, FuncType);
            break;
          case llvm::Instruction::FPTrunc:
          case llvm::Instruction::FPExt:
          case llvm::Instruction::FCmp:
          case llvm::Instruction::FRem:
#ifdef DBG_COMPILER
            std::cerr << "UNHANDLED FP INSTRUCTION: " << I->getOpcodeName()
              << std::endl;
#endif
            break;
          case llvm::Instruction::Call:
            if (isa<llvm::IntrinsicInst>(I)) {
              llvm::IntrinsicInst *II = cast<llvm::IntrinsicInst>(I);

              switch(II->getIntrinsicID()) {
              default:
                break;
              case Intrinsic::sqrt:
                FuncName = "float32_sqrt";
                ParamTys.push_back(Int32Ty);
                FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
                p_module->getOrInsertFunction(FuncName, FuncType);
                break;
              case Intrinsic::fmuladd:
                FuncName = "float32_mul";
                ParamTys.push_back(Int32Ty);
                ParamTys.push_back(Int32Ty);
                FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
                p_module->getOrInsertFunction(FuncName, FuncType);

                FuncName = "float32_add";
                p_module->getOrInsertFunction(FuncName, FuncType);
                break;
              }
            }
            break;
          }
        }
        else {
          switch(I->getOpcode()) {
          default:
            break;
          case llvm::Instruction::FPToSI:
          case llvm::Instruction::FPToUI:
            FuncName = "float32_to_int32";
            ParamTys.push_back(Int32Ty);
            FuncType = llvm::FunctionType::get(Int32Ty, ParamTys, false);
            p_module->getOrInsertFunction(FuncName, FuncType);
            break;
          }
        }
      }
    }
  }
}

bool Compiler::ExtractKernelData(llvm::Module *M, EmbeddedData &theData) {
#ifdef DBG_COMPILER
  std::cerr << "Entering Compiler::ExtractKernelData" << std::endl;
#endif

  for (llvm::Module::global_iterator GI = M->global_begin(),
       GE = M->global_end(); GI != GE; ++GI) {

    llvm::GlobalVariable *GV = GI;
    StringRef name = GV->getName();

#ifdef DBG_COMPILER
    std::cerr << "GV = " << name.str() << std::endl;
#endif

    if (isa<Function>(GV))
      continue;

    if (GV->hasInitializer()) {
      Constant *C = GV->getInitializer();
      unsigned NumOps = C->getNumOperands();
#ifdef DBG_COMPILER
      std::cerr << "Global variable " << name.str() << " has " << NumOps
        << " values" << std::endl;
#endif
      if (isa<ConstantInt>(C)) {
        ConstantInt *CI = cast<ConstantInt>(C);
#ifdef DBG_COMPILER
        std::cerr << "global is a ConstantInt" << std::endl;
#endif
        llvm::Type *type = CI->getType();
        if (type->isIntegerTy(32)) {

          EmbeddedData::GlobalVariable<unsigned> *newGlobal =
            new EmbeddedData::GlobalVariable<unsigned>(name.str());

          newGlobal->addElement(CI->getZExtValue());
          theData.addWordVariable(newGlobal);
        }
        else {
          std::cerr << "!!ERROR: Unhandled ConstantInt initialiser"
            << std::endl;
          return false;
        }
      }
      else if (isa<ConstantArray>(C)) {
#ifdef DBG_COMPILER
         std::cerr << "!!ERROR: global is a ConstantArray and not handled"
           << std::endl;
         return false;
#endif
      }
      else if (isa<ConstantDataSequential>(C)) {
        ConstantDataSequential *CDS = cast<ConstantDataSequential>(C);
        unsigned NumElements = CDS->getNumElements();
        llvm::Type *type = CDS->getElementType();

#ifdef DBG_COMPILER
        std::cerr << "global is a ConstantDataSequential with "
          << NumElements << " elements:" << std::endl;
        for (unsigned i = 0; i < NumElements; ++i)
          std::cerr << CDS->getElementAsInteger(i) << std::endl;
#endif

        if (type->isIntegerTy(32)) {

          EmbeddedData::GlobalVariable<unsigned> *newGlobal =
            new EmbeddedData::GlobalVariable<unsigned>(name.str());

          for (unsigned i = 0; i < NumElements; ++i)
            newGlobal->addElement(CDS->getElementAsInteger(i));

          theData.addWordVariable(newGlobal);
        }
        else if (type->isIntegerTy(16)) {
          EmbeddedData::GlobalVariable<unsigned short> *newGlobal =
            new EmbeddedData::GlobalVariable<unsigned short>(name.str());

          for (unsigned i = 0; i < NumElements; ++i)
            newGlobal->addElement(CDS->getElementAsInteger(i));

          theData.addHalfVariable(newGlobal);
        }
        else {
          std::cerr << "!! ERROR: Unhandled initialised global variable"
            << std::endl;
          return false;
        }
      }
      else {
        std::cerr << "!! ERROR: Unhandled initialised global variable"
          << std::endl;
        return false;
      }
    }
    else {
      std::cerr << "GlobalVariable " << name.str()
        << " does not have initialiser!" << std::endl;
    }
  }
  return true;
}

// BackendUtil.cpp
// CodeGenOptions
bool Compiler::CompileToAssembly(std::string &Filename, llvm::Module *M) {
#ifdef DBG_COMPILER
  std::cerr << "Entering CompileToAssembly\n";
#endif
  clang::EmitAssemblyAction act(&llvm::getGlobalContext()); //Module->getContext()?


  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();

  //llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
  //llvm::initializeCore(Registry);
  //llvm::initializeScalarOpts(Registry);
  //llvm::initializeTarget(Registry);

  std::string Error;
  const llvm::Target *TheTarget = llvm::TargetRegistry::lookupTarget(Triple,
                                                                     Error);
  if (!TheTarget) {
    std::cerr << "Target not available: " << Error;
    return false;
  }

  llvm::TargetOptions Options;
  std::string FeatureSet;
  std::auto_ptr<llvm::TargetMachine>
    target(TheTarget->createTargetMachine(Triple,
                                          CPU, FeatureSet, Options,
                                          llvm::Reloc::Static,
                                          llvm::CodeModel::Default,
                                          llvm::CodeGenOpt::Aggressive));
  llvm::TargetMachine &Target = *target.get();

  llvm::PassManager PM;
  llvm::TargetLibraryInfo *TLI =
    new llvm::TargetLibraryInfo(llvm::Triple(Triple));
  TLI->disableAllFunctions();
  PM.add(TLI);
  if (target.get()) {
    PM.add(new llvm::TargetTransformInfo(target->getScalarTargetTransformInfo(),
                                         target->getVectorTargetTransformInfo())
           );
  }

  if (const llvm::DataLayout *TD = Target.getDataLayout())
    PM.add(new llvm::DataLayout(*TD));
  else
    PM.add(new llvm::DataLayout(M));

  //PM.add(createIndVarSimplifyPass());
  //PM.add(createLoopUnrollPass(10, 2, 1));

  llvm::tool_output_file *FDOut = new llvm::tool_output_file(Filename.c_str(),
                                                             Error, 0);
  if (!Error.empty()) {
    std::cerr << Error;
    delete FDOut;
    delete TLI;
    return false;
  }

  llvm::OwningPtr<llvm::tool_output_file> Out(FDOut);
  if (!Out) {
    std::cerr << "Couldn't create output file\n";
    return false;
  }
  llvm::formatted_raw_ostream FOS(Out->os());
  //AnalysisID StartAfterID = 0;
  //AnalysisID StopAfterID = 0;


  if (Target.addPassesToEmitFile(PM, FOS,
                                 llvm::TargetMachine::CGFT_AssemblyFile,
                                 false, 0, 0)) {
    std::cerr << "Target does not support generation of this filetype!\n";
    return false;
  }

  llvm::cl::PrintOptionValues();
  PM.run(*M);

  Out->keep();

#ifdef DBG_COMPILER
  std::cerr << "Leaving CompileToAssembly\n";
#endif
  return true;
}

const std::string &Compiler::log() const
{
    return p_log;
}

const std::string &Compiler::options() const
{
    return p_options;
}

bool Compiler::optimize() const
{
    return p_optimize;
}

llvm::Module *Compiler::module() const
{
    return p_module;
}

void Compiler::appendLog(const std::string &log)
{
    p_log += log;
}
