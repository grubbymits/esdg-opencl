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

#include "deviceinterface.h"
#include "compiler.h"

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/LangStandard.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/CodeGen/BackendUtil.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <llvm/DataLayout.h>
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
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Target/TargetMachine.h>

using namespace Coal;

Compiler::Compiler(DeviceInterface *device)
: p_device(device), p_module(0), p_optimize(true), p_log_stream(p_log),
  p_log_printer(0)
{
#ifdef DEBUGCL
  std::cerr << "Constructing Compiler::Compiler\n";
#endif
  Triple = device->getTriple();
  CPU = device->getCPU();

}

Compiler::~Compiler()
{
#ifdef DEBUGCL
  std::cerr << "Destructing Compiler::Compiler\n";
#endif
}

bool Compiler::CompileToBitcode(std::string &Source,
                                clang::InputKind SourceKind) {
#ifdef DEBUGCL
  std::cerr << "Entering CompileToBitcode\n";
#endif
  clang::EmitLLVMOnlyAction act(&llvm::getGlobalContext());
  std::string log;
  llvm::raw_string_ostream s_log(log);

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
  p_compiler.getCodeGenOpts().setInlining(
    clang::CodeGenOptions::OnlyAlwaysInlining);

  p_compiler.getLangOpts().NoBuiltin = true;
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
#ifdef DEBUGCL
    std::cerr << "Compilation Failed\n";
    std::cerr << log;
#endif
    return false;
  }
  p_module = act.takeModule();

    //PrevKernels.push_back(name);
#ifdef DEBUGCL
  std::cerr << "Leaving Compiler::CompileToBitcode\n";
#endif
    return true;

}

llvm::Module *Compiler::LinkModules(llvm::Module *m1, llvm::Module *m2) {
#ifdef DEBUGCL
  std::cerr << "Entering LinkModules\n";
#endif
  llvm::StringRef progName("MyLinker");
  llvm::Linker ld(progName, m1);
  if ( ld.LinkInModule(m2) ) {
    return NULL;
  }
#ifdef DEBUGCL
  std::cerr << "Leaving LinkModules\n";
#endif
  return ld.releaseModule();
}

// BackendUtil.cpp
// CodeGenOptions
bool Compiler::CompileToAssembly(std::string &Filename, llvm::Module *M) {
#ifdef DEBUGCL
  std::cerr << "Entering CompileToAssembly\n";
#endif
  clang::EmitAssemblyAction act(&llvm::getGlobalContext()); //Module->getContext()?

  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();

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
                                          llvm::CodeGenOpt::Default));
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

  PM.run(*M);

  Out->keep();

  // Look at what llc does instead

  //p_compiler.getFrontendOpts().Inputs.clear();
  //p_compiler.getFrontendOpts().Inputs.push_back(
    //clang::FrontendInputFile("temp.bc", clang::IK_LLVM_IR));
  //p_compiler.getFrontendOpts().ProgramAction = clang::frontend::EmitAssembly;
  //p_compiler.getFrontendOpts().Inputs.push_back(
    //clang::FrontendInputFile(llvm::MemoryBuffer::getMemBufferCopy(Source.str()),
      //                       clang::IK_LLVM_IR));
  //p_compiler.getFrontendOpts().OutputFile = Filename;
  //p_compiler.getInvocation().setLangDefaults(p_compiler.getLangOpts(),
    //                                         clang::IK_LLVM_IR);
  //p_compiler.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
    //s_log, &p_compiler.getDiagnosticOpts()));

  //if (!p_compiler.ExecuteAction(act)) {
//#ifdef DEBUGCL
  //  std::cerr << "Assembly compilation failed\n";
    //std::cerr << log;
//#endif
  //  return false;
  //}

#ifdef DEBUGCL
  std::cerr << "Leaving CompileToAssembly\n";
#endif
  return true;
}

// TODO - I think this is possible...
// Compile nows need to take the source file and write to an AST. So the
// intermediate format will now be .ast instead of .bc and this should mean
// that source code changes will be minimal.

//bool Compiler::compile(const std::string &options,
                                //llvm::MemoryBuffer *src)
//{

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
