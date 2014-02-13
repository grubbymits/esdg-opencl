//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012, 2013 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "../coarsener/SourceRewriter.h"
#include "TransformationManager.h"

#include <cstddef>
#include <iostream>
#include <sstream>

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/ParseAST.h"

/*
 * Avoid a bunch of warnings about redefinitions of PACKAGE_* symbols.
 *
 * The definitions of these symbols are produced by Autoconf et al.
 * For C-Reduce, we define these in <config.h>.
 * LLVM defines these symbols in "llvm/Config/config.h".
 * But we don't care anything about these symbols in this source file.
 */
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
//#include "llvm/Config/config.h"

#include "Transformation.h"

using namespace clang;

TransformationManager* TransformationManager::Instance;

std::map<std::string, Transformation *> *
TransformationManager::TransformationsMapPtr;

TransformationManager *TransformationManager::GetInstance(void)
{
#ifdef DEBUGCL
  std::cerr << "TransformationManager::GetInstance" << std::endl;
#endif
  if (!TransformationManager::TransformationsMapPtr) {
    TransformationManager::TransformationsMapPtr = 
      new std::map<std::string, Transformation *>();
  }
  if (TransformationManager::Instance)
    return TransformationManager::Instance;

  TransformationManager::Instance = new TransformationManager();
  assert(TransformationManager::Instance);

#ifdef DEBUGCL
  std::cerr << "Created new TransformationManager" << std::endl;
#endif

  TransformationManager::Instance->TransformationsMap = 
    *TransformationManager::TransformationsMapPtr;
  return TransformationManager::Instance;
}

bool TransformationManager::isCXXLangOpt(void)
{
  TransAssert(TransformationManager::Instance && "Invalid Instance!");
  TransAssert(TransformationManager::Instance->ClangInstance && 
              "Invalid ClangInstance!");
  return (TransformationManager::Instance->ClangInstance->getLangOpts()
          .CPlusPlus);
}

bool TransformationManager::isCLangOpt(void)
{
  TransAssert(TransformationManager::Instance && "Invalid Instance!");
  TransAssert(TransformationManager::Instance->ClangInstance && 
              "Invalid ClangInstance!");
  return (TransformationManager::Instance->ClangInstance->getLangOpts()
          .C99);
}

void TransformationManager::reset(void) {
#ifdef DEBUGCL
  std::cerr << "TransformationManager::reset" << std::endl;
#endif
  if (ClangInstance) {
#ifdef DEBUGCL
    std::cerr << "Deleting ClangInstance" << std::endl;
#endif
    delete ClangInstance;
    ClangInstance = NULL;
  }
#ifdef DEBUGCL
  std::cerr << "Clearing associated strings" << std::endl;
#endif
  SrcFileName.erase();
  OutputFileName.erase();
  Source.erase();
}

bool TransformationManager::initializeCompilerInstance(std::string &ErrorMsg)
{
#ifdef DEBUGCL
  std::cerr << "Entering TransformationManager::initializeCompilerInstance"
    << std::endl;
#endif

  if (ClangInstance) {
    ErrorMsg = "CompilerInstance has been initialized!";
    return false;
  }

  ClangInstance = new CompilerInstance();
  assert(ClangInstance);
  
  ClangInstance->createDiagnostics();

  CompilerInvocation &Invocation = ClangInstance->getInvocation();

  ClangInstance->getHeaderSearchOpts().AddPath(LIBCLC_INCLUDE_DIR,
                                               clang::frontend::Angled,
                                               false, false);
  ClangInstance->getHeaderSearchOpts().AddPath(
    "/opt/esdg-opencl/lib/clang/3.4/include", clang::frontend::Angled,
    false, false);
  ClangInstance->getHeaderSearchOpts().ResourceDir =
    "/opt/esdg-opencl/lib/clang/3.4/";
  ClangInstance->getHeaderSearchOpts().UseBuiltinIncludes = true;
  ClangInstance->getHeaderSearchOpts().UseStandardSystemIncludes = false;

  ClangInstance->getPreprocessorOpts().Includes.push_back("clc/clc.h");
  ClangInstance->getPreprocessorOpts().addMacroDef(
    "cl_clang_storage_class_specifiers");

  Invocation.setLangDefaults(ClangInstance->getLangOpts(), IK_OpenCL);

  TargetOptions &TargetOpts = ClangInstance->getTargetOpts();
  // FIXME This shouldn't be fixed
  TargetOpts.Triple = "le1"; //LLVM_DEFAULT_TARGET_TRIPLE;
  TargetInfo *Target =
    TargetInfo::CreateTargetInfo(ClangInstance->getDiagnostics(),
                                 &TargetOpts);
  ClangInstance->setTarget(Target);
  ClangInstance->createFileManager();
  ClangInstance->createSourceManager(ClangInstance->getFileManager());
  ClangInstance->createPreprocessor();

  DiagnosticConsumer &DgClient = ClangInstance->getDiagnosticClient();
  DgClient.BeginSourceFile(ClangInstance->getLangOpts(),
                           &ClangInstance->getPreprocessor());
  ClangInstance->createASTContext();

  assert(CurrentTransformationImpl && "Bad transformation instance!");

  // FIXME - When ClangInstance is deleted, I believe its destroying
  // the transformation too!
  ClangInstance->setASTConsumer(CurrentTransformationImpl);

  Preprocessor &PP = ClangInstance->getPreprocessor();
  PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());

  if (!SrcFileName.empty()) {
    if (!ClangInstance->InitializeSourceManager(FrontendInputFile(SrcFileName,
                                                                  IK_OpenCL))) {
      ErrorMsg = "Cannot open source file!";
      return false;
    }
  }
  else if (!Source.empty()){
    ClangInstance->getFrontendOpts().Inputs.push_back(
      clang::FrontendInputFile("temp.cl", IK_OpenCL));
    ClangInstance->getPreprocessorOpts()
      .addRemappedFile("temp.cl", llvm::MemoryBuffer::getMemBuffer(Source));

  }
  else {
    ErrorMsg = "Source not defined!";
    return false;
  }

  return true;
}

void TransformationManager::Finalize(void)
{
#ifdef DEBUGCL
  std::cerr << "TransformationManager::Finalize" << std::endl;
#endif

  assert(TransformationManager::Instance);
  
  std::map<std::string, Transformation *>::iterator I, E;
  for (I = Instance->TransformationsMap.begin(), 
       E = Instance->TransformationsMap.end();
       I != E; ++I) {
    // CurrentTransformationImpl will be freed by ClangInstance
    if ((*I).second != Instance->CurrentTransformationImpl)
      delete (*I).second;
  }
  if (Instance->TransformationsMapPtr) {
    Instance->TransformationsMapPtr = NULL;
    delete Instance->TransformationsMapPtr;
  }

  if (Instance->ClangInstance)
    delete Instance->ClangInstance;

  delete Instance;
  Instance = NULL;
}

llvm::raw_ostream *TransformationManager::getOutStream(void)
{
  if (OutputFileName.empty())
    return &(llvm::outs());

  std::string Err;
  llvm::raw_fd_ostream *Out = 
    new llvm::raw_fd_ostream(OutputFileName.c_str(), Err);
  assert(Err.empty() && "Cannot open output file!");
  return Out;
}

void TransformationManager::closeOutStream(llvm::raw_ostream *OutStream)
{
  if (!OutputFileName.empty())
    delete OutStream;
}

bool TransformationManager::doTransformation(std::string &ErrorMsg)
{
#ifdef DEBUGCL
  std::cerr << "Entering Transformation::doTransformation for "
    << SrcFileName << std::endl;
  std::cerr << "ClangInstance addr: " << ClangInstance << std::endl;
#endif
  ErrorMsg = "";

#ifdef DEBUGCL
  std::cerr << "createSema" << std::endl;
#endif
  ClangInstance->createSema(TU_Complete, 0);
  //ClangInstance->getDiagnostics().setSuppressAllDiagnostics(true);

  CurrentTransformationImpl->setQueryInstanceFlag(QueryInstanceOnly);
  CurrentTransformationImpl->setTransformationCounter(TransformationCounter);

#ifdef DEBUGCL
  std::cerr << "ParseAST" << std::endl;
#endif
  ParseAST(ClangInstance->getSema(), true);

#ifdef DEBUGCL
  std::cerr << "getDiagnosticsClient" << std::endl;
#endif
  ClangInstance->getDiagnosticClient().EndSourceFile();

  if (QueryInstanceOnly) {
#ifdef DEBUGCL
    std::cerr << "QueryInstanceOnly" << std::endl;
#endif
    return true;
  }

  llvm::raw_ostream *OutStream = getOutStream();
  bool RV;
  if (CurrentTransformationImpl->transSuccess()) {
#ifdef DEBUGCL
    std::cerr << "transSuccess" << std::endl;
#endif
    if (!OutputFileName.empty())
      CurrentTransformationImpl->outputTransformedSource(*OutStream);
    else
      CurrentTransformationImpl->outputTransformedSource(TransformedSource);

    RV = true;
  }
  else if (CurrentTransformationImpl->transInternalError()) {
#ifdef DEBUGCL
    std::cerr << "transInternalError" << std::endl;
#endif
    CurrentTransformationImpl->outputOriginalSource(*OutStream);
    RV = true;
  }
  else {
#ifdef DEBUGCL
    std::cerr << "TransError" << std::endl;
#endif
    CurrentTransformationImpl->getTransErrorMsg(ErrorMsg);
    RV = false;
  }
  closeOutStream(OutStream);
#ifdef DEBUGCL
  std::cerr << "Leaving doTransformation" << std::endl;
#endif
  return RV;
}

bool TransformationManager::verify(std::string &ErrorMsg)
{
  if (!CurrentTransformationImpl) {
    ErrorMsg = "Empty transformation instance!";
    return false;
  }

  if ((TransformationCounter <= 0) && 
      !CurrentTransformationImpl->skipCounter()) {
    ErrorMsg = "Invalid transformation counter!";
    return false;
  }

  return true;
}

void TransformationManager::registerTransformation(
       const char *TransName, 
       Transformation *TransImpl)
{
  if (!TransformationManager::TransformationsMapPtr) {
    TransformationManager::TransformationsMapPtr = 
      new std::map<std::string, Transformation *>();
  }

  assert((TransImpl != NULL) && "NULL Transformation!");
  assert((TransformationManager::TransformationsMapPtr->find(TransName) == 
          TransformationManager::TransformationsMapPtr->end()) &&
         "Duplicated transformation!");
  (*TransformationManager::TransformationsMapPtr)[TransName] = TransImpl;
}

void TransformationManager::printTransformations(void)
{
  llvm::outs() << "Registered Transformations:\n";

  std::map<std::string, Transformation *>::iterator I, E;
  for (I = TransformationsMap.begin(), 
       E = TransformationsMap.end();
       I != E; ++I) {
    llvm::outs() << "  [" << (*I).first << "]: "; 
    llvm::outs() << (*I).second->getDescription() << "\n";
  }
}

void TransformationManager::printTransformationNames(void)
{
  std::map<std::string, Transformation *>::iterator I, E;
  for (I = TransformationsMap.begin(), 
       E = TransformationsMap.end();
       I != E; ++I) {
    llvm::outs() << (*I).first << "\n";
  }
}

int TransformationManager::getNumTransformationInstances(void) {
    return CurrentTransformationImpl->getNumTransformationInstances();
}

void TransformationManager::outputNumTransformationInstances(void)
{
  int NumInstances = 
    CurrentTransformationImpl->getNumTransformationInstances();
  llvm::outs() << "Available transformation instances: " 
               << NumInstances << "\n";
}

TransformationManager::TransformationManager(void)
  : CurrentTransformationImpl(NULL),
    TransformationCounter(-1),
    SrcFileName(""),
    OutputFileName(""),
    ClangInstance(NULL),
    QueryInstanceOnly(false)
{
  // Nothing to do
}

TransformationManager::~TransformationManager(void)
{
  // Nothing to do
}

