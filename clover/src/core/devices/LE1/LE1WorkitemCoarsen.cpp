#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "LE1WorkitemCoarsen.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

typedef std::vector<Stmt*> StmtSet;
typedef std::vector<DeclRefExpr*> DeclRefSet;
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;

bool WorkitemCoarsen::CreateWorkgroup(std::string &Filename) {
#ifdef DEBUGCL
  std::cerr << "CreateWorkgroup\n";
#endif
  OpenCLCompiler<KernelInitialiser> InitCompiler(LocalX, LocalY, LocalZ);
  OrigFilename = Filename;
  InitCompiler.setFile(OrigFilename);
  InitCompiler.Parse();

  // At this point the rewriter's buffer should be full with the rewritten
  // file contents.
  const RewriteBuffer *RewriteBuf = InitCompiler.getRewriteBuf();
  if (RewriteBuf == NULL)
    return false;

  //clang_parseTranslationUnit from libclang provides the option to pass source
  // files via memory buffers ("unsaved_files" parameter).
  // You can "follow" its code path to see how this can be done programmatically

  InitKernelSource = std::string(RewriteBuf->begin(), RewriteBuf->end());
#ifdef DEBUGCL
  std::cerr << "Finished initialising workgroup\n";
  std::cerr << InitKernelSource;
#endif

  std::ofstream init_kernel;
  InitKernelFilename = std::string("init.");
  InitKernelFilename.append(OrigFilename);
  init_kernel.open(InitKernelFilename.c_str());
  init_kernel << InitKernelSource;
  init_kernel.close();
  return HandleBarriers();
}

bool WorkitemCoarsen::HandleBarriers() {
#ifdef DEBUGCL
  std::cerr << "HandleBarriers\n";
#endif
  OpenCLCompiler<ThreadSerialiser> SerialCompiler(LocalX, LocalY, LocalZ);
  SerialCompiler.setFile(InitKernelFilename);
  SerialCompiler.Parse();

  const RewriteBuffer *RewriteBuf = SerialCompiler.getRewriteBuf();
  if (RewriteBuf == NULL) {
    FinalKernel = InitKernelSource;
    return true;
  }

  //std::ofstream final_kernel;
  //FinalFilename = InitFilename.append(".final.cl");
  //final_kernel.open(FinalFilename.c_str());
  FinalKernel = std::string(RewriteBuf->begin(), RewriteBuf->end());
  //final_kernel.close();

#ifdef DEBUGCL
  std::cerr << "Finalised kernel:\n" << FinalKernel;
#endif

  return true;
}

#define CLANG_RESOURCE_DIR  "/usr/local/lib/clang/3.2/"
#define LIBCLC_INCLUDEDIR   "/opt/esdg-opencl/include"
bool WorkitemCoarsen::Compile(std::string &filename, std::string &source) {
                              //std::string &triple) {
  if (!ProduceBytecode(filename, source))
    return false;

  llvm::raw_string_ostream OutputStream(FinalKernel);
  llvm::WriteBitcodeToFile(Module, OutputStream);
  return true;
}

bool WorkitemCoarsen::ProduceBytecode(std::string &filename,
                                      std::string &source) {
                                      //std::string &triple) {

  clang::EmitBCAction Action(&llvm::getGlobalContext());
  std::string log;
  llvm::raw_string_ostream s_log(log);
  TheCompiler.getFrontendOpts().Inputs.push_back(
    clang::FrontendInputFile(filename, clang::IK_OpenCL));

  TheCompiler.getHeaderSearchOpts().UseBuiltinIncludes = true;
  TheCompiler.getHeaderSearchOpts().UseStandardSystemIncludes = false;
  TheCompiler.getHeaderSearchOpts().ResourceDir = CLANG_RESOURCE_DIR;

  // Add libclc search path
  TheCompiler.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDEDIR,
                                             clang::frontend::Angled,
                                             false, false, false);

  // Add libclc include
  TheCompiler.getPreprocessorOpts().Includes.push_back("clc/clc.h");

  // clc.h requires that this macro be defined
  TheCompiler.getPreprocessorOpts().addMacroDef(
      "cl_clang_storage_class_specifiers");

  TheCompiler.getCodeGenOpts().LinkBitcodeFile = "/opt/esdg-opencl/lib/builtins.bc";
  TheCompiler.getCodeGenOpts().OptimizationLevel = 3;
  TheCompiler.getCodeGenOpts().setInlining(
    clang::CodeGenOptions::OnlyAlwaysInlining);

  TheCompiler.getLangOpts().NoBuiltin = true;
  TheCompiler.getTargetOpts().Triple = "le1";
  TheCompiler.getInvocation().setLangDefaults(TheCompiler.getLangOpts(),
                                               clang::IK_OpenCL);
  TheCompiler.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
        s_log, &TheCompiler.getDiagnosticOpts()));

  TheCompiler.getPreprocessorOpts()
    .addRemappedFile(filename , llvm::MemoryBuffer::getMemBuffer(source));

  // Compile the code
  if (!TheCompiler.ExecuteAction(Action)) {
    return false;
  }

  Module = Action.takeModule();
  return true;
}

bool WorkitemCoarsen::ProduceAssembly(std::string &filename) {
  clang::EmitAssemblyAction act(&llvm::getGlobalContext());
  std::string log;
  llvm::raw_string_ostream s_log(log);

  TheCompiler.getFrontendOpts().ProgramAction = clang::frontend::EmitAssembly;
  TheCompiler.getFrontendOpts().Inputs.push_back(
    clang::FrontendInputFile(filename, clang::IK_LLVM_IR));
  TheCompiler.getFrontendOpts().OutputFile = filename;

  if (!TheCompiler.ExecuteAction(act)) {
    return false;
  }

  return true;
}

//template <typename T>
//OpenCLCompiler<T>::KernelInitialiser::KernelInitialiser(Rewriter &R,
WorkitemCoarsen::KernelInitialiser::KernelInitialiser(Rewriter &R,
                                                        unsigned x,
                                                        unsigned y,
                                                        unsigned z)
    : LocalX(x), LocalY(y), LocalZ(z), TheRewriter(R) {

  if (LocalZ != 0) {
    OpenWhile << "\n__kernel_local_id[2] = 0;\n";
    OpenWhile  << "while (__kernel_local_id[2] < " << LocalZ << ") {\n";
  }
  if (LocalY != 0) {
    OpenWhile << "__kernel_local_id[1] = 0;\n";
    OpenWhile << "while (__kernel_local_id[1] < " << LocalY << ") {\n";
  }
  if (LocalX != 0) {
    OpenWhile << "__kernel_local_id[0] = 0;\n";
    OpenWhile << "while (__kernel_local_id[0] < " << LocalX << ") {\n";
  }

  if (LocalX != 0) {
    CloseWhile << "\n__kernel_local_id[0]++;\n";
    CloseWhile  << "}\n";
  }
  if (LocalY != 0) {
    CloseWhile << " __kernel_local_id[1]++;\n";
    CloseWhile << "}\n";
  }
  if (LocalZ != 0) {
    CloseWhile << "__kernel_local_id[2]++;\n";
    CloseWhile << "}\n";
  }
}

//template <typename T>
//void OpenCLCompiler<T>::KernelInitialiser::CloseLoop(SourceLocation Loc) {
void WorkitemCoarsen::KernelInitialiser::CloseLoop(SourceLocation Loc) {
  TheRewriter.InsertText(Loc, CloseWhile.str(), true, true);
}

//template <typename T>
void WorkitemCoarsen::KernelInitialiser::OpenLoop(SourceLocation Loc) {
  TheRewriter.InsertText(Loc, OpenWhile.str(), true, true);
}

//template <typename T>
bool WorkitemCoarsen::KernelInitialiser::VisitFunctionDecl(FunctionDecl *f) {
  // Only function definitions (with bodies), not declarations.
  if (f->hasBody()) {
    Stmt *FuncBody = f->getBody();
    SourceLocation FuncBodyStart =
    FuncBody->getLocStart().getLocWithOffset(2);

    std::stringstream FuncBegin;
    FuncBegin << "  int __kernel_local_id[";
    if (LocalZ && LocalY && LocalX)
      FuncBegin << "3";
    else if (LocalY && LocalX)
      FuncBegin << "2";
    else
      FuncBegin << "1";
    FuncBegin << "];\n";

    TheRewriter.InsertText(FuncBodyStart, FuncBegin.str(), true, true);
    OpenLoop(FuncBodyStart);
    CloseLoop(FuncBody->getLocEnd());
  }
  return true;
}

//template <typename T>
bool WorkitemCoarsen::KernelInitialiser::VisitDeclStmt(Stmt *s) {
  DeclStmt *DS = cast<DeclStmt>(s);
  // If there are grouped declarations, split them into individual decls.
  if (DS->isSingleDecl())
    return true;
  else {
    // Remove the original declarations
    TheRewriter.RemoveText(s->getSourceRange());

    // Iterate through the group, splitting each into individual stmts.
    NamedDecl *ND = NULL;
    DeclGroupRef::iterator DE = DS->decl_end();
    --DE;

    for (DeclGroupRef::iterator DI = DS->decl_begin(); DI != DE; ++DI) {
      ND = cast<NamedDecl>((*DI));
      std::string key = ND->getName().str();
      std::cout << ND->getName().str() << " is declared\n";

      std::string type = cast<ValueDecl>(ND)->getType().getAsString();
      std::stringstream newDecl;
      newDecl << "\n" << type << " " << key << ";";
      TheRewriter.InsertText(ND->getLocStart(), newDecl.str(), true);
    }

    // Handle the last declaration which could also be initialised.
    DeclStmt::reverse_decl_iterator Last = DS->decl_rbegin();
    ND = cast<NamedDecl>(*Last);
    std::string key = ND->getName().str();
    std::string type = cast<ValueDecl>(ND)->getType().getAsString();
    std::stringstream newDecl;
    newDecl << "\n" << type << " " << key;
    TheRewriter.InsertText(ND->getLocStart(), newDecl.str(), true);

    if (s->child_begin() == s->child_end()) {
      TheRewriter.InsertText(ND->getLocStart(), ";");
      return true;
    }

    for (Stmt::child_iterator SI = s->child_begin(), SE = s->child_end();
         SI != SE; ++SI) {
      std::stringstream init;
      init << " = " << TheRewriter.ConvertToString(*SI) << ";";
      TheRewriter.InsertText(ND->getLocStart(), init.str(), true);
    }
  }
  return true;
}

//template <typename T>
bool WorkitemCoarsen::KernelInitialiser::VisitCallExpr(Expr *s) {
  CallExpr *Call = cast<CallExpr>(s);
  FunctionDecl* FD = Call->getDirectCallee();
  DeclarationName DeclName = FD->getNameInfo().getName();
  std::string FuncName = DeclName.getAsString();

  // Modify any calls to get_global_id to use the generated local ids.
  if (FuncName.compare("get_global_id") == 0) {
    IntegerLiteral *Arg;
    unsigned Index = 0;

    if (IntegerLiteral::classof(Call->getArg(0))) {
      Arg = cast<IntegerLiteral>(Call->getArg(0));
    }
    else if (ImplicitCastExpr::classof(Call->getArg(0))) {
      Expr *Cast = cast<ImplicitCastExpr>(Call->getArg(0))->getSubExpr();
      Arg = cast<IntegerLiteral>(Cast);
    }
    else
      llvm_unreachable("unhandled argument type!");

    Index = Arg->getValue().getZExtValue();

    // Remove the semi-colon after the call
    TheRewriter.RemoveText(Call->getLocEnd().getLocWithOffset(1), 1);

    switch(Index) {
    default:
      break;
    case 0:
      TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(2),
                             " + __kernel_local_id[0];\n", true, true);
      break;
    case 1:
      TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(2),
                             " + __kernel_local_id[1];\n", true, true);
      break;
    case 2:
      TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(2),
                             " + __kernel_local_id[2];\n", true, true);
      break;
    }
  }
  // FIXME Handle local_id calls
  else if (FuncName.compare("get_local_id") == 0) {

  }
  return true;
}

//template <typename T>
WorkitemCoarsen::ThreadSerialiser::ThreadSerialiser(Rewriter &R,
                                   unsigned x,
                                   unsigned y,
                                   unsigned z)
        : LocalX(x), LocalY(y), LocalZ(z), TheRewriter(R) {
  if (LocalZ != 0) {
    OpenWhile << "\n__kernel_local_id[2] = 0;\n";
    OpenWhile  << "while (__kernel_local_id[2] < " << LocalZ << ") {\n";
  }
  if (LocalY != 0) {
    OpenWhile << "__kernel_local_id[1] = 0;\n";
    OpenWhile << "while (__kernel_local_id[1] < " << LocalY << ") {\n";
  }
  if (LocalX != 0) {
    OpenWhile << "__kernel_local_id[0] = 0;\n";
    OpenWhile << "while (__kernel_local_id[0] < " << LocalX << ") {\n";
  }

  if (LocalX != 0) {
    CloseWhile << "\n__kernel_local_id[0]++;\n";
    CloseWhile  << "}\n";
  }
  if (LocalY != 0) {
    CloseWhile << " __kernel_local_id[1]++;\n";
    CloseWhile << "}\n";
  }
  if (LocalZ != 0) {
    CloseWhile << "__kernel_local_id[2]++;\n";
    CloseWhile << "}\n";
  }
}

//template <typename T>
void WorkitemCoarsen::ThreadSerialiser::CloseLoop(SourceLocation Loc) {
    TheRewriter.InsertText(Loc, CloseWhile.str(), true, true);
}

//template <typename T>
void WorkitemCoarsen::ThreadSerialiser::OpenLoop(SourceLocation Loc) {
  TheRewriter.InsertText(Loc, OpenWhile.str(), true, true);
}

//template <typename T>
void WorkitemCoarsen::ThreadSerialiser::CreateLocalVariable(DeclRefExpr *Ref,
                                                            bool ScalarRepl) {
  NamedDecl *ND = Ref->getDecl();
  std::string varName = ND->getName().str();

  std::cout << "Creating local variable for " << varName << std::endl;

  // Make sure we don't add the same value more than once
  if (NewLocalDecls.find(varName) != NewLocalDecls.end())
    return;

  std::string type = cast<ValueDecl>(ND)->getType().getAsString();
  std::stringstream NewDecl;
  NewDecl << type << " " << varName;

  // For variables that are live across loop boundaries, we use scalar
  // replication to hold the values of each work-item. This requires us
  // to retrospectively look back at the previous references and turn them
  // into array accesses as well as just declaring the variables as an array.
  if (ScalarRepl) {
    std::cout << "Declaring it as an array\n";
    if (LocalX != 0)
      NewDecl << "[" << LocalX << "]";
    if (LocalY != 0)
      NewDecl << "[" << LocalY << "]";
    if (LocalZ != 0)
      NewDecl << "[" << LocalZ << "]";
    NewScalarRepls.insert(std::make_pair(varName, ND));

    // Visit all the references of this variable
    DeclRefSet varRefs = AllRefs[varName];
    for (std::vector<DeclRefExpr*>::iterator RI = varRefs.begin(),
         RE = varRefs.end(); RI != RE; ++RI) {
      AccessScalar(*RI);
    }
  }
  else {
    NewLocalDecls.insert(std::make_pair(varName, ND));
  }
  NewDecl << ";\n";

  TheRewriter.InsertText(FuncStart, NewDecl.str(), true, true);

  // Remove the old declaration; if it wasn't initialised, remove the whole
  // statement and not just the type declaration.
  if (DeclStmts.find(varName) != DeclStmts.end()) {
    DeclStmt *DS = DeclStmts[varName];
    if (DS->child_begin() != DS->child_end()) {
      TheRewriter.RemoveText(ND->getLocStart(), type.length());
      if(ScalarRepl)
        AccessScalar(DeclStmts[varName]->getSingleDecl());
    }
    else {
      TheRewriter.RemoveText(ND->getLocEnd().getLocWithOffset(1));
      TheRewriter.RemoveText(ND->getSourceRange());
    }
    DeclStmts.erase(varName);
  }
}

//template <typename T>
void WorkitemCoarsen::ThreadSerialiser::AccessScalar(Decl *decl) {
  NamedDecl *ND = cast<NamedDecl>(decl);
  unsigned offset = ND->getName().str().length();
  offset += cast<ValueDecl>(ND)->getType().getAsString().length();
  // increment because of a space between type and name
  ++offset;
  SourceLocation Loc = ND->getLocStart().getLocWithOffset(offset);
  TheRewriter.InsertText(Loc, "[__kernel_local_id[0]]", true);
}

//template <typename T>
void WorkitemCoarsen::ThreadSerialiser::AccessScalar(DeclRefExpr *Ref) {
  std::cout << "Creating scalar access for "
      << Ref->getDecl()->getName().str() << std::endl;
  unsigned offset = Ref->getDecl()->getName().str().length();
  SourceLocation loc = Ref->getLocEnd().getLocWithOffset(offset);
  TheRewriter.InsertText(loc, "[__kernel_local_id[0]]", true);
}

//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::BarrierInLoop(ForStmt* s) {
  Stmt* ForBody = cast<ForStmt>(s)->getBody();
  SourceLocation ForLoc = s->getLocStart();

  for (Stmt::child_iterator FI = ForBody->child_begin(),
       FE = ForBody->child_end(); FI != FE; ++FI) {

    // Recursively visit inner loops
    if (ForStmt* nested = dyn_cast_or_null<ForStmt>(*FI)) {
      if (BarrierInLoop(nested)) {
        LoopsWithBarrier.insert(std::make_pair(ForLoc, s));
        LoopsToDistribute.push_back(s);
        return true;
      }
    }
    else if (CallExpr* Call = dyn_cast_or_null<CallExpr>(*FI)) {
      FunctionDecl* FD = Call->getDirectCallee();
      DeclarationName DeclName = FD->getNameInfo().getName();
      std::string FuncName = DeclName.getAsString();

      if (FuncName.compare("barrier") == 0) {
        LoopsWithBarrier.insert(std::make_pair(ForLoc, s));
        LoopsToDistribute.push_back(s);
        LoopBarrier = Call;
        LoopBarriers.insert(std::make_pair(Call->getLocStart(), Call));
        return true;
      }
    }
  }
  LoopsWithoutBarrier.insert(std::make_pair(ForLoc, s));
  return false;
}

// Check for barrier calls within loops, this is necessary to close the
// nested loops properly.
//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::VisitForStmt(Stmt *s) {
  std::cout << "VisitForStmt\n";
  ForStmt* ForLoop = cast<ForStmt>(s);
  SourceLocation ForLoc = ForLoop->getLocStart();
  // Check whether we've already visited the loop
  if (LoopsWithBarrier.find(ForLoc) != LoopsWithBarrier.end())
    return true;
  else if (LoopsWithoutBarrier.find(ForLoc) != LoopsWithoutBarrier.end())
    return true;

  if (BarrierInLoop(ForLoop)) {
    // LoopsToReplicate is a vector containing one or more (nested)
    // loops that need to be distributed.

    // Close the loop(s)
    SourceLocation BarrierLoc = LoopBarrier->getLocStart();
    for (unsigned i = 0; i < LoopsToDistribute.size(); ++i)
      TheRewriter.InsertText(BarrierLoc, "}\n", true, true);

    CloseLoop(BarrierLoc);
    OpenLoop(BarrierLoc);

    // Initialise the new loop(s)
    while (!LoopsToDistribute.empty()) {
      ForStmt *Loop = LoopsToDistribute.back();

      // If the for loop use DeclRefExpr, we will need to move the
      // declarations into a global scope since the while loop was has just
      // been closed.
      // TODO There must be a cleaner way of doing this?
      for (Stmt::child_iterator FI = Loop->getInit()->child_begin(),
           FE = Loop->getInit()->child_end(); FI != FE; ++FI) {
        if (isa<DeclRefExpr>(*FI)) {
          DeclRefExpr *Ref = cast<DeclRefExpr>(*FI);
          NamedDecl *ND = Ref->getDecl();
          std::string key = ND->getName().str();
          if (NewLocalDecls.find(key) == NewLocalDecls.end()) {
            CreateLocalVariable(Ref, false);
          }
        }
      }
      for (Stmt::child_iterator FI = Loop->getCond()->child_begin(),
           FE = Loop->getCond()->child_end(); FI != FE; ++FI) {
        if (isa<DeclRefExpr>(*FI)) {
          DeclRefExpr *Ref = cast<DeclRefExpr>(*FI);
          NamedDecl *ND = Ref->getDecl();
          std::string key = ND->getName().str();
          if (NewLocalDecls.find(key) == NewLocalDecls.end()) {
            CreateLocalVariable(Ref, false);
          }
        }
      }
      for (Stmt::child_iterator FI = Loop->getInc()->child_begin(),
           FE = Loop->getInc()->child_end(); FI != FE; ++FI) {
        if (isa<DeclRefExpr>(*FI)) {
          DeclRefExpr *Ref = cast<DeclRefExpr>(*FI);
          NamedDecl *ND = Ref->getDecl();
          std::string key = ND->getName().str();
          if (NewLocalDecls.find(key) == NewLocalDecls.end()) {
            CreateLocalVariable(Ref, false);
          }
        }
      }

      LoopsToDistribute.pop_back();
      Stmt *Init = Loop->getInit();
      //const DeclStmt *CondVar = Loop->getConditionVariableDeclStmt();
      Expr *Cond = Loop->getCond();
      Expr *Inc = Loop->getInc();

      std::stringstream LoopHeader;
      LoopHeader << "for (";
      if (Init)
        LoopHeader << TheRewriter.ConvertToString(Init) << "; ";
      // FIXME Need to handle CondVar
      //if (CondVar)
        //std::cout << TheRewriter.ConvertToString(static_cast<CondVar);
      if (Cond)
        LoopHeader << TheRewriter.ConvertToString(Cond) << "; ";
      if (Inc)
        LoopHeader << TheRewriter.ConvertToString(Inc) << ") {\n";

      TheRewriter.InsertText(BarrierLoc, LoopHeader.str(), true, true);
    }

  }
  return true;
}

// Remove barrier calls and modify calls to kernel builtin functions.
//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::VisitCallExpr(Expr *s) {
  std::cout << "VisitCallExpr\n";
  CallExpr *Call = cast<CallExpr>(s);
  FunctionDecl* FD = Call->getDirectCallee();
  DeclarationName DeclName = FD->getNameInfo().getName();
  std::string FuncName = DeclName.getAsString();

  if (FuncName.compare("barrier") == 0) {
    BarrierCalls.push_back(Call);
    TheRewriter.InsertText(Call->getLocStart(), "//", true, true);

    if (LoopBarriers.find(Call->getLocStart()) == LoopBarriers.end()) {
      // Then open another nested loop for the code after the barrier
      OpenLoop(Call->getLocEnd().getLocWithOffset(2));
      // Close the triple nest so all work items complete
      CloseLoop(Call->getLocEnd().getLocWithOffset(1));
    }
  }
  return true;
}

// TODO Need to handle continues and breaks
//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::WalkUpFromUnaryContinueStmt(
  UnaryOperator *S) {
  return true;
}

//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::VisitDeclStmt(Stmt *s) {
  DeclStmt *DS = cast<DeclStmt>(s);
  if (DS->isSingleDecl()) {
    NamedDecl *ND = cast<NamedDecl>(DS->getSingleDecl());
    std::string key = ND->getName().str();
    DeclStmts.insert(std::make_pair(key, DS));
  }
  return true;
}

//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::VisitDeclRefExpr(Expr *expr) {
  DeclRefExpr *RefExpr = cast<DeclRefExpr>(expr);
  std::string key = RefExpr->getDecl()->getName().str();

  if (AllRefs.find(key) == AllRefs.end()) {
    DeclRefSet refset;
    refset.push_back(RefExpr);
    AllRefs.insert(std::make_pair(key, refset));
  }
  else {
    AllRefs[key].push_back(RefExpr);
  }

  SourceLocation RefLoc = RefExpr->getLocStart();

  // If we've already added it, don't do it again, but we may need to access
  // an array instead
  if (NewScalarRepls.find(key) != NewScalarRepls.end()) {
    AccessScalar(RefExpr);
    return true;
  }

  // Don't add it if its one of an work-item indexes
  if (key.compare("__kernel_local_id") == 0)
    return true;

  // Also don't add it if it is a function parameter
  for (std::vector<std::string>::iterator PI = ParamVars.begin(),
       PE = ParamVars.end(); PI != PE; ++PI) {
    if ((*PI).compare(key) == 0)
      return true;
  }

  // First, get the location of the declartion of this variable. It will be
  // inside a while loop, but we have to figure out which one.
  NamedDecl *Decl = RefExpr->getDecl();

  // Get the location of the variable declaration, and see if there's any
  // barrier calls between the reference and declaration.
  SourceLocation DeclLoc = Decl->getLocStart();

  for (std::vector<CallExpr*>::iterator CI = BarrierCalls.begin(),
       CE = BarrierCalls.end(); CI != CE; ++CI) {

    SourceLocation BarrierLoc = (*CI)->getLocStart();
    if ((DeclLoc < BarrierLoc) && (BarrierLoc < RefLoc)) {
      CreateLocalVariable(RefExpr, true);
    }
  }

  return true;
}

//template <typename T>
bool WorkitemCoarsen::ThreadSerialiser::VisitFunctionDecl(FunctionDecl *f) {
  // Only function definitions (with bodies), not declarations.
  if (f->hasBody()) {

    // Collect the parameter names so we don't later try to localize them
    for (FunctionDecl::param_iterator PI = f->param_begin(),
         PE = f->param_end(); PI != PE; ++PI) {
      ParamVars.push_back((*PI)->getName().str());
    }

    FuncStart = f->getBody()->getLocStart().getLocWithOffset(1);
    Stmt *FuncBody = f->getBody();
    FuncBodyStart = FuncBody->getLocStart().getLocWithOffset(2);
  }
  return true;
}

template <typename T>
WorkitemCoarsen::OpenCLCompiler<T>::OpenCLCompiler(unsigned x,
                                                unsigned y,
                                                unsigned z) {

  // CompilerInstance will hold the instance of the Clang compiler for us,
  // managing the various objects needed to run the compiler.
  TheCompInst.createDiagnostics(0, 0);

  // Initialize target info with the default triple for our platform.
  IntrusiveRefCntPtr<TargetOptions> TO(new TargetOptions);
  TO.getPtr()->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *TI = TargetInfo::CreateTargetInfo(
  TheCompInst.getDiagnostics(), *TO);
  TheCompInst.setTarget(TI);

  // Set the compiler up to handle OpenCL
  TheCompInst.getHeaderSearchOpts().AddPath("/opt/esdg-opencl/include",
                                            clang::frontend::Angled,
                                            false, false, false);
  TheCompInst.getHeaderSearchOpts().AddPath(
    "/usr/local/lib/clang/3.2/include", clang::frontend::Angled,
    false, false, false);
  TheCompInst.getHeaderSearchOpts().ResourceDir = "/usr/local/lib/clang/3.2/";
  TheCompInst.getHeaderSearchOpts().UseBuiltinIncludes = true;
  TheCompInst.getHeaderSearchOpts().UseStandardSystemIncludes = false;

  TheCompInst.getPreprocessorOpts().Includes.push_back("clc/clc.h");
  TheCompInst.getPreprocessorOpts().addMacroDef(
    "cl_clang_storage_class_specifiers");
  TheCompInst.getInvocation().setLangDefaults(TheCompInst.getLangOpts(),
                                              clang::IK_OpenCL);

  TheCompInst.createFileManager();
  FileMgr = &(TheCompInst.getFileManager());
  TheCompInst.createSourceManager(*FileMgr);
  SourceMgr = &(TheCompInst.getSourceManager());
  TheCompInst.createPreprocessor();
  TheCompInst.createASTContext();

  // A Rewriter helps us manage the code rewriting task.
  TheRewriter.setSourceMgr(*SourceMgr, TheCompInst.getLangOpts());

  // Create an AST consumer instance which is going to get called by
  // ParseAST.
  TheConsumer = new OpenCLASTConsumer(TheRewriter, x, y, z);
}

template <typename T>
WorkitemCoarsen::OpenCLCompiler<T>::~OpenCLCompiler() {
  delete TheConsumer;
}

template <typename T>
void WorkitemCoarsen::OpenCLCompiler<T>::setFile(std::string input) {
  // Set the main file handled by the source manager to the input file.
  const FileEntry *FileIn = FileMgr->getFile(input.c_str());
  SourceMgr->createMainFileID(FileIn);
  TheCompInst.getDiagnosticClient().BeginSourceFile(
  TheCompInst.getLangOpts(),
  &TheCompInst.getPreprocessor());
}

template <typename T>
void WorkitemCoarsen::OpenCLCompiler<T>::Parse() {
  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), TheConsumer,
           TheCompInst.getASTContext());
}

