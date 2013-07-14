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
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/LangStandard.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Linker.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

using namespace Coal;

Compiler::Compiler(DeviceInterface *device)
: p_device(device), p_module(0), p_optimize(true), p_log_stream(p_log),
  p_log_printer(0)
{
#ifdef DEBUGCL
  std::cerr << "Constructing Compiler::Compiler\n";
#endif
  TargetTriple = device->getTriple();
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
  clang::EmitLLVMAction act(&llvm::getGlobalContext());
  std::string log;
  llvm::raw_string_ostream s_log(log);

  std::string TempFilename;
  if (SourceKind == clang::IK_OpenCL)
    TempFilename = "temp.cl";
  else
    TempFilename = "temp.c";
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

  if (SourceKind == clang::IK_OpenCL) {
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

  p_compiler.getCodeGenOpts().OptimizationLevel = 3;
  p_compiler.getCodeGenOpts().setInlining(
    clang::CodeGenOptions::OnlyAlwaysInlining);

  p_compiler.getLangOpts().NoBuiltin = true;
  p_compiler.getTargetOpts().Triple = TargetTriple;
  p_compiler.getInvocation().setLangDefaults(p_compiler.getLangOpts(),
                                             SourceKind);
  p_compiler.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
    s_log, &p_compiler.getDiagnosticOpts()));

  //llvm::MemoryBuffer Buffer = llvm::MemoryBuffer::getMemBuffer(Source);
  p_compiler.getPreprocessorOpts()
    .addRemappedFile(TempFilename, Source);

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

bool Compiler::CompileToAssembly(std::string &Filename, llvm::Module *Code) {
#ifdef DEBUGCL
  std::cerr << "Entering CompileToAssembly\n";
#endif
  clang::EmitAssemblyAction act(&llvm::getGlobalContext());
  std::string log;
  llvm::raw_string_ostream s_log(log);

  std::string SourceString;
  llvm::raw_string_ostream Source(SourceString);
  llvm::WriteBitcodeToFile(Code, Source);

  p_compiler.getFrontendOpts().ProgramAction = clang::frontend::EmitAssembly;
  p_compiler.getFrontendOpts().Inputs.push_back(
    clang::FrontendInputFile(llvm::MemoryBuffer::getMemBuffer(Source.str()),
                             clang::IK_LLVM_IR));
  p_compiler.getFrontendOpts().OutputFile = Filename;
  p_compiler.getTargetOpts().CPU = CPU;

  if (!p_compiler.ExecuteAction(act)) {
#ifdef DEBUGCL
    std::cerr << "Assembly compilation failed\n";
#endif
    return false;
  }

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
bool Compiler::compile(std::string &name,
                       std::string &source) {
#ifdef DEBUGCL
  std::cerr << "Entering Compiler::compile\n";
#endif
    /* Set options */
    //p_options = options;
  std::string OutputFile = name + ".tmp.bc";

    clang::EmitBCAction act(&llvm::getGlobalContext());
    std::string log;
    llvm::raw_string_ostream s_log(log);

    p_compiler.getFrontendOpts().Inputs.push_back(
      clang::FrontendInputFile(name, clang::IK_OpenCL));
    p_compiler.getFrontendOpts().ProgramAction = clang::frontend::EmitBC;
    p_compiler.getFrontendOpts().OutputFile = OutputFile;
    p_compiler.getHeaderSearchOpts().UseBuiltinIncludes = true;
    p_compiler.getHeaderSearchOpts().UseStandardSystemIncludes = false;
    p_compiler.getHeaderSearchOpts().ResourceDir = CLANG_RESOURCE_DIR;

    // Add libclc search path
    p_compiler.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDE_DIR,
                                             clang::frontend::Angled,
                                             false, false, false);
    p_compiler.getHeaderSearchOpts().AddPath(CLANG_RESOURCE_DIR,
                                             clang::frontend::Angled,
                                             false, false, false);

    // Add libclc include
    p_compiler.getPreprocessorOpts().Includes.push_back("clc/clc.h");

    // clc.h requires that this macro be defined
    p_compiler.getPreprocessorOpts().addMacroDef(
      "cl_clang_storage_class_specifiers");

    p_compiler.getCodeGenOpts().LinkBitcodeFile = "/opt/esdg-opencl/lib/builtins.bc";
    //p_compiler.getCodeGenOpts().OptimizationLevel = 2;
    p_compiler.getCodeGenOpts().setInlining(
      clang::CodeGenOptions::OnlyAlwaysInlining);

    p_compiler.getLangOpts().NoBuiltin = true;
    p_compiler.getTargetOpts().Triple = TargetTriple;
    p_compiler.getInvocation().setLangDefaults(p_compiler.getLangOpts(),
                                               clang::IK_OpenCL);
    p_compiler.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
        s_log, &p_compiler.getDiagnosticOpts()));

    p_compiler.getPreprocessorOpts()
      .addRemappedFile(name, llvm::MemoryBuffer::getMemBuffer(source));

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
  std::cerr << "Leaving Compiler::compile\n";
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
