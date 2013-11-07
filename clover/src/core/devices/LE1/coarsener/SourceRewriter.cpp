#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "SourceRewriter.h"
#include "StmtFixers.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace Coal;

typedef std::vector<Stmt*> StmtSet;
typedef std::vector<DeclRefExpr*> DeclRefSet;
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;

WorkitemCoarsen::WorkitemCoarsen(unsigned x, unsigned y, unsigned z) :
  LocalX(x), LocalY(y), LocalZ(z) {
  pthread_mutex_init(&p_inline_mutex, 0);
}

void WorkitemCoarsen::DeleteTempFiles() {
  std::stringstream DeleteCommand;

  DeleteCommand << "rm ";

  for (std::vector<std::string>::iterator SI = TempFiles.begin(),
       SE = TempFiles.end(); SI != SE; ++SI)
    DeleteCommand << (*SI) << " ";

  system(DeleteCommand.str().c_str());
}

bool WorkitemCoarsen::CreateWorkgroup(std::string &Filename, std::string
                                      &kernel) {
#ifdef DBG_WRKGRP
  std::cerr << "CreateWorkgroup from " << Filename << std::endl;
#endif
  OrigFilename.assign(Filename);
  KernelName = kernel;

  TempFiles.push_back(OrigFilename);

  // Firstly, we need to expand the macros within the source code because
  // it is not possible to rewrite their locations.
  /*
  std::string SourceContainer;
  llvm::raw_string_ostream ExpandedSource(SourceContainer);
  if(!ExpandMacros()) {
    std::cerr << "!ERROR : Failed to expand macros" << std::endl;
    return false;
  }*/

  // Initialise the kernel with the while loop(s) and check for barriers
  OpenCLCompiler<KernelInitialiser> InitCompiler(LocalX, LocalY, LocalZ,
                                                 KernelName);
  InitCompiler.setFile(OrigFilename);
  InitCompiler.Parse();

  // At this point the rewriter's buffer should be full with the rewritten
  // file contents.
  const RewriteBuffer *RewriteBuf = InitCompiler.getRewriteBuf();
  if (RewriteBuf == NULL)
    return false;

  // TODO Don't want to have to write the source string to a file before
  // passing it to this function.
  //clang_parseTranslationUnit from libclang provides the option to pass source
  // files via memory buffers ("unsaved_files" parameter).
  // You can "follow" its code path to see how this can be done programmatically

  InitKernelSource = std::string(RewriteBuf->begin(), RewriteBuf->end());
#ifdef DBG_WRKGRP
  std::cerr << "Finished initialising workgroup\n";
#endif

  std::ofstream init_kernel;
  InitKernelFilename = std::string("init.");
  InitKernelFilename.append(OrigFilename);
  init_kernel.open(InitKernelFilename.c_str());
  init_kernel << InitKernelSource;
  init_kernel.close();

  TempFiles.push_back(InitKernelFilename);

  return HandleBarriers();
}

bool WorkitemCoarsen::HandleBarriers() {
  OpenCLCompiler<ThreadSerialiser> SerialCompiler(LocalX, LocalY, LocalZ,
                                                  KernelName);
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

#ifdef DBG_WRKGRP
  std::cerr << "Finalised kernel:\n";
#endif
  
  std::ofstream final_kernel;
  std::string FinalKernelFilename = "final.";
  FinalKernelFilename.append(OrigFilename);
  final_kernel.open(FinalKernelFilename.c_str());
  final_kernel << FinalKernel;
  final_kernel.close();

  TempFiles.push_back(FinalKernelFilename);

  return true;
}

static bool SortLocations(std::pair<SourceLocation, std::string> first,
                          std::pair<SourceLocation, std::string> second) {

  if (first.first < second.first)
    return true;
  else
    return false;
}

template <typename T>
WorkitemCoarsen::ASTVisitorBase<T>::ASTVisitorBase(Rewriter &R,
                                                   unsigned x,
                                                   unsigned y,
                                                   unsigned z)
    : LocalX(x), LocalY(y), LocalZ(z), TheRewriter(R) {

  OpenWhile << "\n";
  if (LocalZ > 1) {
    //OpenWhile  << "while (__kernel_local_id[2] < " << LocalZ << ") {\n";
    OpenWhile << "for (__esdg_idz = 0; __esdg_idz < " << LocalZ
      << "; ++__esdg_idz) {\n";
  }
  if (LocalY > 1) {
    //OpenWhile << "while (__kernel_local_id[1] < " << LocalY << ") {\n";
    OpenWhile << "for (__esdg_idy = 0; __esdg_idy < " << LocalY
      << "; ++__esdg_idy) {\n";
  }
  if (LocalX > 1) {
    //OpenWhile << "while (__kernel_local_id[0] < " << LocalX << ") {\n";
    OpenWhile << "for (__esdg_idx = 0; __esdg_idx < " << LocalX
      << "; ++__esdg_idx) {\n";
  }

  // We reset them at the end just in case a proceeding loop uses a expanded
  // variable. The variable should have only been expanded because we select
  // the variables very, very lazily. The loop would contain a barrier of sorts
  // and a barrier cannot be executed because of a thread local variable...
  if (LocalX > 1) {
    //CloseWhile << "\n__kernel_local_id[0]++;\n";
    CloseWhile  << "\n} __esdg_idx = 0;\n";
    //CloseWhile << "__kernel_local_id[0] = 0;\n";
  }
  if (LocalY > 1) {
    //CloseWhile << " __kernel_local_id[1]++;\n";
    CloseWhile << "\n} __esdg_idy = 0;\n";
    //CloseWhile << "__kernel_local_id[1] = 0;\n";
  }
  if (LocalZ > 1) {
    //CloseWhile << "__kernel_local_id[2]++;\n";
    CloseWhile << "\n} __esdg_idz = 0;\n";
    //CloseWhile << "\n__kernel_local_id[2] = 0;\n";
  }

}

template <typename T>
void WorkitemCoarsen::ASTVisitorBase<T>::RewriteSource() {
  SourceToInsert.sort(SortLocations);

  for (std::list<std::pair<SourceLocation, std::string> >::iterator PI =
       SourceToInsert.begin(), PE = SourceToInsert.end(); PI != PE; ++PI) {
    SourceLocation Loc = (*PI).first;
    std::string Text = (*PI).second;
    TheRewriter.InsertText(Loc, Text);
  }
}

template <typename T>
void WorkitemCoarsen::ASTVisitorBase<T>::CloseLoop(SourceLocation Loc) {
  //TheRewriter.InsertText(Loc, CloseWhile.str(), true, true);
  InsertText(Loc, CloseWhile.str());
}

template <typename T>
void WorkitemCoarsen::ASTVisitorBase<T>::OpenLoop(SourceLocation Loc) {
  //TheRewriter.InsertText(Loc, OpenWhile.str(), true, true);
  InsertText(Loc, OpenWhile.str());
}

static inline bool isBarrier(Stmt *s) {
  if (isa<CallExpr>(s)) {
    FunctionDecl *FD = (cast<CallExpr>(s))->getDirectCallee();
    std::string name = FD->getNameInfo().getName().getAsString();
    if (name.compare("barrier") == 0)
      return true;
  }
  return false;
}

static inline bool isLoop(Stmt *s) {
  return (isa<ForStmt>(s) || isa<WhileStmt>(s));
}
// ------------------------------------------------------------------------- //
// --------------------- Start of KernelInitialiser ------------------------ //
// ------------------------------------------------------------------------- //
bool WorkitemCoarsen::KernelInitialiser::VisitFunctionDecl(FunctionDecl *f) {
  // Only function definitions (with bodies), not declarations.
  if ((f->hasBody()) && (f->hasAttr<clang::OpenCLKernelAttr>())) {
    Stmt *FuncBody = f->getBody();
    SourceLocation FuncBodyStart = FuncBody->getLocStart().getLocWithOffset(2);

    std::stringstream FuncBegin;
    FuncBegin << "  unsigned __esdg_idx = 0;";
    if (LocalY > 1)
      FuncBegin << "  unsigned __esdg_idy = 0;";
    if (LocalZ > 1)
      FuncBegin << "  unsigned __esdg_idz = 0;";

    /*
    if (LocalZ && LocalY && LocalX)
      FuncBegin << "3";
    else if (LocalY && LocalX)
      FuncBegin << "2";
    else
      FuncBegin << "1";
    FuncBegin << "] = {0};\n";*/

    //TheRewriter.InsertText(FuncBodyStart, FuncBegin.str(), true, true);
    InsertText(FuncBodyStart, FuncBegin.str());
    OpenLoop(FuncBodyStart);
    CloseLoop(FuncBody->getLocEnd());
  }
  return true;
}

// If there are grouped declarations, split them into individual decls.
bool WorkitemCoarsen::KernelInitialiser::VisitDeclStmt(Stmt *s) {
  DeclStmt *DS = cast<DeclStmt>(s);
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
#ifdef DBG_WRKGRP
      std::cerr << ND->getName().str() << " is declared\n";
#endif

      std::string type = cast<ValueDecl>(ND)->getType().getAsString();
      std::stringstream newDecl;
      newDecl << "\n" << type << " " << key << ";";
      //TheRewriter.InsertText(ND->getLocStart(), newDecl.str(), true);
      InsertText(ND->getLocStart(), newDecl.str());
    }

    // Handle the last declaration which could also be initialised.
    DeclStmt::reverse_decl_iterator Last = DS->decl_rbegin();
    ND = cast<NamedDecl>(*Last);
    std::string key = ND->getName().str();
    std::string type = cast<ValueDecl>(ND)->getType().getAsString();
    std::stringstream newDecl;
    newDecl << "\n" << type << " " << key;
    //TheRewriter.InsertText(ND->getLocStart(), newDecl.str(), true);
    InsertText(ND->getLocStart(), newDecl.str());

    if (s->child_begin() == s->child_end()) {
      //TheRewriter.InsertText(ND->getLocStart(), ";");
      InsertText(ND->getLocStart(), ";");
      return true;
    }

    for (Stmt::child_iterator SI = s->child_begin(), SE = s->child_end();
         SI != SE; ++SI) {
      std::stringstream init;
      init << " = " << TheRewriter.ConvertToString(*SI) << ";";
      //TheRewriter.InsertText(ND->getLocStart(), init.str(), true);
      InsertText(ND->getLocStart(), init.str());
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
  IntegerLiteral *Arg;
  unsigned Index = 0;
  if ((FuncName.compare("get_global_id") == 0) ||
      (FuncName.compare("get_local_id") == 0) ||
      (FuncName.compare("get_local_size") == 0)) {

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
  }
  if (FuncName.compare("get_global_id") == 0) {

    // Replace with a call to get_group_id and then add the local_id to it
    // Remove the semi-colon after the call
    //TheRewriter.RemoveText(Call->getLocEnd().getLocWithOffset(1), 1);

    std::stringstream GlobalId;
    switch(Index) {
    default:
      break;
    case 0:
      GlobalId << "get_group_id(0) * " << LocalX
        << " + __esdg_idx;//"; //__kernel_local_id[0];//";
      break;
    case 1:
      GlobalId << "get_group_id(1) * " << LocalY
        << " + __esdg_idy;//";//__kernel_local_id[1];//";
      break;
    case 2:
      GlobalId << "get_group_id(2) * " << LocalZ
        << " + __esdg_idz;//";//__kernel_local_id[2];//";
      break;
    }
    //TheRewriter.InsertText(Call->getLocStart(), GlobalId.str());
    InsertText(Call->getLocStart(), GlobalId.str());
  }
  else if (FuncName.compare("get_local_id") == 0) {
    TheRewriter.RemoveText(Call->getSourceRange());
    if (Index == 0) {
      //TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(1),
        //                     "__kernel_local_id[0]");
      InsertText(Call->getLocEnd().getLocWithOffset(1),
                 "__esdg_idx");
                 //"__kernel_local_id[0]");
    }
    else if (Index == 1) {
      //TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(1),
        //                     "__kernel_local_id[1]");
      InsertText(Call->getLocEnd().getLocWithOffset(1),
                 "__esdg_idy");
                 //"__kernel_local_id[1]");
    }
    else {
      //TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(1),
        //                     "__kernel_local_id[2]");
      InsertText(Call->getLocEnd().getLocWithOffset(1),
                 "__esdg_idz");
                 //"__kernel_local_id[2]");
    }

  }
  else if (FuncName.compare("get_local_size") == 0) {
    TheRewriter.RemoveText(Call->getSourceRange());
    std::stringstream local;
    if (Index == 0)
      local << LocalX;
    else if (Index == 1)
      local << LocalY;
    else
      local << LocalZ;

    //TheRewriter.InsertText(Call->getLocStart(), local.str());
    InsertText(Call->getLocStart(), local.str());

  }

  return true;
}

// ------------------------------------------------------------------------- //
// --------------------- End of KernelInitialiser -------------------------- //
// ------------------------------------------------------------------------- //

// When it comes to replicating, we will have a map of all the refs and we will
// know in what scopes variables are declared. We will also know if those scopes
// have nested scopes.

// std::map<Stmt*, std::vector<Stmt*>> NestedLoops;
// std::map<Stmt*, std::vector<DeclStmt*>> ScopedDeclStmts;
// std::map<Stmt*, std::vector<CallExpr*>> Barriers;
// std::map<std::string, std::vector<DeclRefExpr*>> AllRefs;

// Recursively visit the inner loops, the function should return true if one of
// its children return true, or if the loop itself has a barrier:
// nestedBarriers = 0;
// if (CheckNestedLoop(NestedLoop)
//    ++nestedBarriers;
// if (hasBarrier || nestedBarriers)
//    HandleBarrierInLoop(this)
//    return true;
// else return false;

static bool isWorkgroupLoop(ForStmt *s) {
#ifdef DBG_WRKGRP
  std::cerr << "isWorkgroupLoop" << std::endl;
#endif
  Stmt *init = s->getInit();
  for (Stmt::child_iterator SI = init->child_begin(), SE = init->child_end();
       SI != SE; ++SI) {
    if (isa<DeclRefExpr>(*SI)) {
      DeclRefExpr *expr = cast<DeclRefExpr>(*SI);
      NamedDecl *ND = cast<NamedDecl>(expr->getDecl());
      std::string name = ND->getName().str();

      return (name.compare("esdg_idx") ||
              name.compare("esdg_idy") ||
              name.compare("esdg_idz"));
    }
  }
}

WorkitemCoarsen::ThreadSerialiser::ThreadSerialiser(Rewriter &R,
                                                    unsigned x,
                                                    unsigned y,
                                                    unsigned z)
  : ASTVisitorBase(R, x, y, z) {

    isFirstLoop = true;
    numDimensions = 1;
    InvalidThreadInit << "bool __kernel_invalid_global_threads[" << x << "]";
    if (y > 1) {
      InvalidThreadInit << "[" << y << "]";
      ++numDimensions;
    }
    if (z > 1) {
      InvalidThreadInit << "[" << z << "]";
      ++numDimensions;
    }
    InvalidThreadInit << " = {false};\n";
    InvalidThreadInit << "unsigned __kernel_total_invalid_global_threads = 0;"
      << std::endl;
    returnFixer = new ReturnFixer(&SourceToInsert, x, y, z);
    breakFixer = new BreakFixer(&SourceToInsert, x, y, z);
}


void WorkitemCoarsen::ThreadSerialiser::RewriteSource() {
#ifdef DBG_WRKGRP
  std::cerr << "ThreadSerialiser::RewriteSource" << std::endl;
#endif

    // We may need to insert some variables to track valid local ids.
    if (!ReturnStmts.empty() && !Barriers.empty()) {
      InsertText(OuterLoop->getLocStart(), InvalidThreadInit.str());
      // FIXME This only works for one dimension!
      OpenWhile << "if (__kernel_invalid_global_threads[__esdg_idx])\n";
      OpenWhile << "  continue;";
    }

  // Visit all the loops which contain barriers, and create regions that are
  // contained by the multi-level while loops
  SearchThroughRegions(OuterLoop);

  // Then we see which variables are only assigned in loop headers, these
  // are then disabled for expansion, though they can still be made local.
  AssignIndVars();

  // Then we need to find all the variables that we need to replicate.
  std::list<DeclStmt*> Stmts;
  FindRefsToExpand(Stmts, OuterLoop);

  // Once we have decided all the text we need to insert, sort it and write it
  SourceToInsert.sort(SortLocations);

  for (std::list<std::pair<SourceLocation, std::string> >::iterator PI =
       SourceToInsert.begin(), PE = SourceToInsert.end(); PI != PE; ++PI) {
    SourceLocation Loc = (*PI).first;
    std::string Text = (*PI).second;
    TheRewriter.InsertText(Loc, Text);
  }
}

void WorkitemCoarsen::ThreadSerialiser::AssignIndVars() {
#ifdef DBG_WRKGRP
  std::cerr << "AssignIndVars" << std::endl;
#endif

  for (std::map<Decl*, std::list<DeclRefExpr*> >::iterator MI
       = PotentialIndVars.begin(), ME = PotentialIndVars.end();
       MI != ME; ++MI) {

    Decl *decl = MI->first;
    std::list<DeclRefExpr*> IndVarRefs = MI->second;
    std::list<DeclRefExpr*> RefAssigns = RefAssignments[decl];

#ifdef DBG_WRKGRP
    std::cerr << "Var: " << cast<NamedDecl>(decl)->getName().str()
      << ". IndVarRefs.size = " << IndVarRefs.size() << " and RefAssigns = "
      << RefAssigns.size() << std::endl;
#endif

    if (IndVarRefs.size() == RefAssigns.size()) {
#ifdef DBG_WRKGRP
      std::cerr << "Found induction variable" << std::endl;
#endif
      IndVars.push_back(decl);
    }
  }
}

void WorkitemCoarsen::ThreadSerialiser::FindRefsToExpand(
  std::list<DeclStmt*> &Stmts, Stmt *Loop) {

#ifdef DBG_WRKGRP
  std::cerr << "FindRefsToExpand" << std::endl;
#endif

  // Again, quit early if there's gonna be nothing to find.
  if ((NestedLoops[Loop].empty()) && (Barriers[Loop].empty()) &&
      ScopedRegions[Loop].empty())
    return;

  // Get the DeclStmts of this loop, append it to the vector that has been
  // passed; so the search space grows as we go deeper into the loops.
  std::list<DeclStmt*> InnerDeclStmts = ScopedDeclStmts[Loop];

  // TODO Combine loops and regions

  if (!NestedLoops[Loop].empty()) {
    std::vector<Stmt*> InnerLoops = NestedLoops[Loop];
    //Stmts.insert(Stmts.end(), InnerDeclStmts.begin(), InnerDeclStmts.end());

    std::list<DeclStmt*>::iterator OrigEnd = Stmts.end();
    Stmts.splice(Stmts.end(), InnerDeclStmts);

    for (std::vector<Stmt*>::iterator LoopI = InnerLoops.begin(),
         LoopE = InnerLoops.end(); LoopI != LoopE; ++LoopI) {
      // Recursively visit inner loops to build up the vector of referable
      // DeclStmts.
      FindRefsToExpand(Stmts, *LoopI);
    }
#ifdef DBG_WRKGRP
    std::cerr << "Erasing elements" << std::endl;
#endif
    // Erase this loop's DeclStmts from the larger set since we search them
    // differently.
    Stmts.erase(OrigEnd, Stmts.end());
  }
  // Now check regions
  else if (!ScopedRegions[Loop].empty()) {
    std::vector<CompoundStmt*> InnerRegions = ScopedRegions[Loop];
    //Stmts.insert(Stmts.end(), InnerDeclStmts.begin(), InnerDeclStmts.end());

    std::list<DeclStmt*>::iterator OrigEnd = Stmts.end();
    Stmts.splice(Stmts.end(), InnerDeclStmts);

    for (std::vector<CompoundStmt*>::iterator LoopI = InnerRegions.begin(),
         LoopE = InnerRegions.end(); LoopI != LoopE; ++LoopI) {
      // Recursively visit inner loops to build up the vector of referable
      // DeclStmts.
      FindRefsToExpand(Stmts, *LoopI);
    }
#ifdef DBG_WRKGRP
    std::cerr << "Erasing elements" << std::endl;
#endif
    // Erase this loop's DeclStmts from the larger set since we search them
    // differently.
    Stmts.erase(OrigEnd, Stmts.end());
  }

  // Start looking for references within this loop:
#ifdef DBG_WRKGRP
  std::cerr << "Look for statements within this loop" << std::endl;
#endif

  // First we check whether the Decl is __local.
  // Second, we can check the DeclStmts within this Loop, and compare the ref
  // locations to that of the barrier(s) also within this loop body.
  std::vector<CallExpr*> InnerBarriers = Barriers[Loop];

  for (std::list<DeclStmt*>::iterator DSI = InnerDeclStmts.begin(),
       DSE = InnerDeclStmts.end(); DSI != DSE; ++DSI) {

    std::vector<DeclRefExpr*> Refs = AllRefs[(*DSI)->getSingleDecl()];
    SourceLocation DeclLoc = (*DSI)->getLocStart();
    bool hasExpanded = false;

    /*
    if ((cast<ValueDecl>((*DSI)->getSingleDecl()))->getType().getAddressSpace())
    {
      ScalarExpand(Loop->getLocStart(), *DSI);
      continue;
    }*/

    for (std::vector<CallExpr*>::iterator BI = InnerBarriers.begin(),
         BE = InnerBarriers.end(); BI != BE; ++BI) {

      // If it's already been replicated, we don't need to check any more
      // barriers.
      if (hasExpanded)
        break;

      SourceLocation BarrierLoc = (*BI)->getLocStart();

      for (std::vector<DeclRefExpr*>::iterator RI = Refs.begin(),
           RE = Refs.end(); RI != RE; ++RI) {

        SourceLocation RefLoc = (*RI)->getLocStart();

        if ((BarrierLoc < RefLoc) && (DeclLoc < BarrierLoc)) {
          SourceLocation InsertLoc;
          if (isa<ForStmt>(Loop)) {
            if (isWorkgroupLoop(cast<ForStmt>(Loop)))
              InsertLoc = FuncBodyStart;
            else
              InsertLoc = Loop->getLocStart();
          }
          ScalarExpand(InsertLoc, *DSI);
          hasExpanded= true;
          break;
        }
      }
    }
  }

#ifdef DBG_WRKGRP
  std::cerr << "Now look through the rest of the Stmts, size = "
    << Stmts.size() << std::endl;
#endif

  // Then just find any references, within the loop, to any of the DeclStmts
  // in the larger set.
  SourceLocation LoopStart;
  SourceLocation LoopEnd;
  if (isa<WhileStmt>(Loop)) {
    LoopStart = (cast<WhileStmt>(Loop))->getBody()->getLocStart();
    LoopEnd = (cast<WhileStmt>(Loop))->getBody()->getLocEnd();
  }
  else if (isa<ForStmt>(Loop)) {
    LoopStart = (cast<ForStmt>(Loop))->getBody()->getLocStart();
    LoopEnd = (cast<ForStmt>(Loop))->getBody()->getLocEnd();
  }
  else {
    LoopStart = (cast<CompoundStmt>(Loop))->getLBracLoc();
    LoopEnd = (cast<CompoundStmt>(Loop))->getRBracLoc();
  }

  for (std::list<DeclStmt*>::iterator DSI = Stmts.begin(), DSE = Stmts.end();
       DSI != DSE;) {

    bool hasExpanded = false;
#ifdef DBG_WRKGRP
    std::cerr << "Checking var: "
      << cast<NamedDecl>((*DSI)->getSingleDecl())->getName().str() << std::endl;
#endif

    std::vector<DeclRefExpr*> Refs = AllRefs[(*DSI)->getSingleDecl()];

    for (std::vector<DeclRefExpr*>::iterator RI = Refs.begin(), RE = Refs.end();
         RI != RE; ++RI) {

      SourceLocation RefLoc = (*RI)->getLocStart();
      if ((LoopStart < RefLoc) && (RefLoc < LoopEnd)) {
        SourceLocation InsertLoc = DeclParents[(*RI)->getDecl()]->getLocStart();
        if (isa<ForStmt>(Loop)) {
          if (isWorkgroupLoop(cast<ForStmt>(Loop)))
            InsertLoc = FuncBodyStart;
        }
        ScalarExpand(InsertLoc, *DSI);
        DSI = Stmts.erase(DSI);
        hasExpanded = true;
        break;
      }
    }
    if (!hasExpanded)
      ++DSI;
  }
#ifdef DBG_WRKGRP
  std::cerr << "Exit FindRefsToExpand" << std::endl;
#endif

}

inline void
WorkitemCoarsen::ThreadSerialiser::ExpandDecl(std::stringstream &NewDecl) {
#ifdef DBG_WRKGRP
  std::cerr << "ExpandDecl" << std::endl;
#endif
  if (LocalX != 0)
    NewDecl << "[" << LocalX << "]";
  if (LocalY > 1)
    NewDecl << "[" << LocalY << "]";
  if (LocalZ > 1)
    NewDecl << "[" << LocalZ << "]";
}

inline void
WorkitemCoarsen::ThreadSerialiser::ExpandRef(std::stringstream &NewRef) {
#ifdef DBG_WRKGRP
  std::cerr << "ExpandRef" << std::endl;
#endif
  if (LocalX != 0)
    NewRef << "[__esdg_idx]"; //"[__kernel_local_id[0]]";
  if (LocalY > 1)
    NewRef << "[__esdg_idy]"; //"[__kernel_local_id[1]]";
  if (LocalZ > 1)
    NewRef << "[__esdg_idz]"; //"[__kernel_local_id[2]]";
}

// Run this after parsing is complete, to access all the referenced variables
// within the DeclStmt which need to be replicated
void
WorkitemCoarsen::ThreadSerialiser::ScalarExpand(SourceLocation InsertLoc,
                                                DeclStmt *DS) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering ScalarExpand " << pthread_self() << std::endl;
#endif

  // First check whether the variable is an induction variable which is just
  // controlling a loop
  bool isIndVar = false;
  for (std::vector<Decl*>::iterator DI = IndVars.begin(), DE = IndVars.end();
       DI != DE; ++DI) {
    if (*DI == DS->getSingleDecl()) {
      isIndVar = true;
      break;
    }
  }
  // If it's an induction variable, do not expand it
  if (isIndVar) {
    CreateLocal(InsertLoc, DS, false);
    return;
  }

  // If the variable is a local variable, we don't need to expand it for
  // each of the work items.
  QualType qualType = (cast<ValueDecl>(DS->getSingleDecl()))->getType();
  bool isVariableLocal = (qualType.getAddressSpace() == LangAS::opencl_local) ?
    true : false;

#ifdef DBG_WRKGRP
  if (isVariableLocal)
    std::cerr << "variable has local storage = " <<
      (int)qualType.getAddressSpace() << std::endl;
  else
    std::cerr << "address space = " << (int)qualType.getAddressSpace()
      << std::endl;
#endif

  bool isScalar = CreateLocal(InsertLoc, DS, !isVariableLocal);

  // Then the original declaration is turned into a reference, creating a
  // scalar access to a new array if necessary.
  if (isScalar)
    RemoveScalarDeclStmt(DS, !isVariableLocal);
  else
    RemoveNonScalarDeclStmt(DS, !isVariableLocal);

  // No need to visit its references if it's __local
  if (isVariableLocal) {
#ifdef DBG_WRKGRP
    std::cerr << "Leaving ScalarExpand" << std::endl;
#endif
    return;
  }

  std::vector<DeclRefExpr*> theRefs = AllRefs[DS->getSingleDecl()];
  // Then visit all the references to turn them into scalar accesses as well.
  for (std::vector<DeclRefExpr*>::iterator RI = theRefs.begin(),
       RE = theRefs.end(); RI != RE; ++RI) {
    if (isScalar)
      AccessScalar(*RI);
    else
      AccessNonScalar(*RI);
  }
#ifdef DBG_WRKGRP
  std::cerr << "Leaving ScalarExpand" << std::endl;
#endif
}

bool WorkitemCoarsen::ThreadSerialiser::CreateLocal(SourceLocation InsertLoc,
                                                    DeclStmt *s,
                                                    bool toExpand) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering CreateLocal ";
  if (toExpand)
    std::cerr << " - going to expand variable ";
  std::cerr << std::endl;
#endif

  bool isScalar = true;
  // Get all the necessary information about the variable
  NamedDecl *ND = cast<NamedDecl>(s->getSingleDecl());

  const clang::QualType QualVarType =
    (cast<ValueDecl>(s->getSingleDecl()))->getType().getUnqualifiedType();
  const clang::Type *VarType = QualVarType.getTypePtr();

  std::string varName = ND->getName().str();

  std::string typeStr;

  // Create the string for the new declaration
  std::stringstream NewDecl;

  if (VarType->isArrayType()) {
    ConstantArrayType *arrayType =
      cast<ConstantArrayType>(const_cast<Type*>(VarType));
    uint64_t arraySize = arrayType->getSize().getZExtValue();
    typeStr = arrayType->getElementType().getUnqualifiedType().getAsString();
    NewDecl << typeStr <<  " " << varName;

    if (toExpand)
      ExpandDecl(NewDecl);

    NewDecl << "[" << arraySize << "];\n";
    isScalar = false;
  }
  else {
    typeStr = QualVarType.getAsString();
    NewDecl << typeStr << " " << varName;

    if (toExpand)
      ExpandDecl(NewDecl);

    NewDecl << ";\n";
  }

  //TheRewriter.InsertText(InsertLoc, NewDecl.str(), true, true);
  InsertText(InsertLoc, NewDecl.str());

#ifdef DBG_WRKGRP
  std::cerr << "Leaving CreateLocal" << std::endl;
#endif

  return isScalar;
}

// Turn the old declaration into a reference
void WorkitemCoarsen::ThreadSerialiser::RemoveScalarDeclStmt(DeclStmt *DS,
                                                             bool toExpand) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering RemoveScalarDeclStmt:\n" << std::endl;
  DS->dumpAll();
#endif

  VarDecl *VD = cast<VarDecl>(DS->getSingleDecl());
  NamedDecl *ND = cast<NamedDecl>(DS->getSingleDecl());
  ValueDecl *ValD = cast<ValueDecl>(DS->getSingleDecl());

  std::string varName = ND->getName().str();

  if (!VD->hasInit()) {
#ifdef DBG_WRKGRP
    std::cerr << "DeclStmt has no initialiser" << std::endl;
#endif
    TheRewriter.RemoveText(DS->getSourceRange());
    return;
  }

  Expr* varInit = VD->getInit();
#ifdef DBG_WRKGRP
  std::cerr << "Initialiser = ";
  varInit->dumpPretty(VD->getASTContext());
  std::cerr << std::endl;
#endif
  SourceLocation AccessLoc = varInit->getLocStart().getLocWithOffset(-1);

  std::stringstream newAccess;
  newAccess << std::endl << varName;

  if (toExpand)
    ExpandRef(newAccess);

  newAccess << " = ";

  //TheRewriter.InsertText(DS->getLocStart(), "//");
  //TheRewriter.InsertText(AccessLoc, "\n");
  //TheRewriter.InsertText(AccessLoc, newAccess.str());
  InsertText(DS->getLocStart(), "//");
  InsertText(AccessLoc, "\n");
  InsertText(AccessLoc, newAccess.str());
#ifdef DBG_WRKGRP
  std::cerr << "Leaving RemoveScalarDeclStmt" << std::endl;
#endif
}
// Turn the old declaration into a reference
void WorkitemCoarsen::ThreadSerialiser::RemoveNonScalarDeclStmt(DeclStmt *DS,
                                                                bool toExpand) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering RemoveNonScalarDeclStmt" << std::endl;
#endif

  NamedDecl *ND = cast<NamedDecl>(DS->getSingleDecl());
  VarDecl *VD = cast<VarDecl>(DS->getSingleDecl());

  if (!VD->hasInit()) {
    TheRewriter.RemoveText(DS->getSourceRange());
    return;
  }

  std::string nameStr = ND->getName().str();

  unsigned index = 0;
  InitListExpr *initList = cast<InitListExpr>(*(DS->child_begin()));
  std::stringstream NewArrayInit;

  if (toExpand) {
    std::stringstream ExpandedName;
    ExpandedName << nameStr;
    ExpandRef(ExpandedName);
    nameStr = ExpandedName.str();
  }

  for (Stmt::child_iterator CI = initList->child_begin(),
       CE = initList->child_end(); CI != CE; ++CI) {

    if (isa<IntegerLiteral>(*CI)) {
      IntegerLiteral *lit = cast<IntegerLiteral>(*CI);
      NewArrayInit << nameStr << "[" << index << "] = "
        << lit->getValue().getZExtValue() << ";" << std::endl;
    }
    ++index;
  }

  // FIXME - What happens when the original initialiser has already been
  // expanded?!
  TheRewriter.RemoveText(DS->getSourceRange());
  //TheRewriter.InsertText(ND->getLocStart(), NewArrayInit.str());
  InsertText(ND->getLocStart(), NewArrayInit.str());
#ifdef DBG_WRKGRP
  std::cerr << "Leaving RemoveNonScalarDeclStmt" << std::endl;
#endif
}

void WorkitemCoarsen::ThreadSerialiser::AccessScalar(DeclRefExpr *Ref) {
  std::stringstream newAccess;
  newAccess << Ref->getDecl()->getName().str();// << "[__kernel_local_id[0]]";
  ExpandRef(newAccess);
  SourceLocation Loc = Ref->getLocStart();
  TheRewriter.RemoveText(Ref->getSourceRange());
  //TheRewriter.InsertText(Loc, newAccess.str());
  InsertText(Loc, newAccess.str());
}

void WorkitemCoarsen::ThreadSerialiser::AccessNonScalar(DeclRefExpr *ref) {
  unsigned offset = ref->getDecl()->getName().str().length();
  SourceLocation Loc = ref->getLocEnd().getLocWithOffset(offset);
  //TheRewriter.InsertText(Loc, "[__kernel_local_id[0]]");
  InsertText(Loc, "[__esdg_idx]"); //"[__kernel_local_id[0]]");
}

SourceLocation
WorkitemCoarsen::ThreadSerialiser::GetOffsetInto(SourceLocation Loc) {
  int offset = Lexer::MeasureTokenLength(Loc,
                                   TheRewriter.getSourceMgr(),
                                   TheRewriter.getLangOpts()) + 3;
  return Loc.getLocWithOffset(offset);
}

SourceLocation
WorkitemCoarsen::ThreadSerialiser::GetOffsetOut(SourceLocation Loc) {
  int offset = Lexer::MeasureTokenLength(Loc,
                                         TheRewriter.getSourceMgr(),
                                         TheRewriter.getLangOpts()) + 2;
  return Loc.getLocWithOffset(offset);
}

// Find out if this loop contains a barrier somehow, it will then
// HandleBarrierInLoop if it finds something.
bool WorkitemCoarsen::ThreadSerialiser::SearchThroughRegions(Stmt *Loop) {
                                                          //bool isOuterLoop) {
#ifdef DBG_WRKGRP
  std::cerr << "SearchThroughRegions" << std::endl;
#endif
  static int depth = -1;
  ++depth;

  if ((NestedLoops[Loop].empty()) && (Barriers[Loop].empty()) &&
      ScopedRegions[Loop].empty()) {
#ifdef DBG_WRKGRP
    std::cerr << "No nested loops and there's no barriers in this one"
      << std::endl;
#endif
    if (!ReturnStmts[Loop].empty())
      FixReturnsInBarrierAbsence(Loop, depth);

    --depth;
    return false;
  }

  unsigned NestedBarriers = 0;
  std::vector<Stmt*> InnerLoops = NestedLoops[Loop];
  for (std::vector<Stmt*>::iterator LoopI
       = InnerLoops.begin(), LoopE = InnerLoops.end(); LoopI != LoopE;
       ++LoopI) {
    if (SearchThroughRegions(*LoopI))
      ++NestedBarriers;
  }

  std::vector<CompoundStmt*> Compounds = ScopedRegions[Loop];
  for (std::vector<CompoundStmt*>::iterator RegionI = Compounds.begin(),
       RegionE = Compounds.end(); RegionI != RegionE; ++RegionI) {
    if (SearchThroughRegions(*RegionI)) {
#ifdef DBG_WRKGRP
      std::cerr << "Found barrier within CompoundStmt" << std::endl;
#endif
      ++NestedBarriers;
    }
  }
  if ((!Barriers[Loop].empty()) || (NestedBarriers != 0)) {

    // Appropriately open and close the workgroup loops if its an original loop
    //if (!isOuterLoop)
      //ParallelRegions.push_back(Loop);
    HandleNonParallelRegion(Loop, depth);
    --depth;
    return true;
  }
  else {
    --depth;
    return false;
  }
}


// Whether a loop contains a barrier, nested or not, we follow these steps:
// - close the main loop before this loop starts
// - open a main loop at the start of the body of the loop
// - close the main loop at the end of the body of the loop
// - open the main loop when the for loop exits
void WorkitemCoarsen::ThreadSerialiser::HandleNonParallelRegion(Stmt *Region,
                                                                int depth)
{
#ifdef DBG_WRKGRP
  std::cerr << "HandleNonParallelRegion\n";
#endif
  // Remove the barriers of this region
  if (Barriers[Region].size() == 1) {
    CallExpr *Call = Barriers[Region].front();
    InsertText(Call->getLocStart(), "//");
    CloseLoop(Call->getLocEnd().getLocWithOffset(2));
    OpenLoop(Call->getLocEnd().getLocWithOffset(3));
  }
  else {
    for (std::vector<CallExpr*>::iterator Call = Barriers[Region].begin(),
       End = Barriers[Region].end(); Call != End; ++Call) {
      InsertText((*Call)->getLocStart(), "//");
      CloseLoop((*Call)->getLocEnd().getLocWithOffset(2));
      OpenLoop((*Call)->getLocEnd().getLocWithOffset(3));
    }
  }

  if (!ReturnStmts[Region].empty())
    returnFixer->FixInBarrierPresence(Region, ReturnStmts[Region],
                                      Barriers[Region], depth);

  // Insert opening and closing loops for all regions, except the outer loop
  if (depth > (numDimensions - 1)) {
    Stmt *LoopBody = NULL;
    if (isa<ForStmt>(Region))
      LoopBody = (cast<ForStmt>(Region))->getBody();
    else if (isa<WhileStmt>(Region))
      LoopBody = (cast<WhileStmt>(Region))->getBody();
    if (!isa<CompoundStmt>(Region)) {
      CloseLoop(Region->getLocStart());
      OpenLoop(GetOffsetInto(LoopBody->getLocStart()));
      CloseLoop(LoopBody->getLocEnd());
      OpenLoop(GetOffsetOut(Region->getLocEnd()));

      // Handle any breaks that may be in this loop
      if (!BreakStmts[Region].empty())
        breakFixer->FixInBarrierPresence(Region, BreakStmts[Region],
                                         Barriers[Region], depth);
    }
    else if (isa<CompoundStmt>(Region)) {
      CompoundStmt *CS = cast<CompoundStmt>(Region);
      CloseLoop(CS->getLBracLoc().getLocWithOffset(-1));
      OpenLoop(GetOffsetInto(CS->getLBracLoc()));
      CloseLoop(CS->getRBracLoc());
      OpenLoop(GetOffsetOut(CS->getLocEnd()));
    }
  }
}

// Returns get converted into continues while in the outer loop. So when at a
// depth of 1, a continue needs to be inserted when exiting the loop instead of
// a break - this would exit the whole kernel otherwise.
void
WorkitemCoarsen::ThreadSerialiser::FixReturnsInBarrierAbsence(Stmt *Region,
                                                              unsigned depth)
{
#ifdef DBG_WRKGRP
  std::cerr << "Entering ThreadSerialiser::FixReturnsInBarrierAbsence, depth = "
    << depth << std::endl;
#endif

  // This is a list of return instructions, paired with their conditional stmt.
  PairedReturnList PRL = ReturnStmts[Region];

  if (depth == 0) {
    // Convert returns in the outer loop to continues.
    for (PairedReturnList::iterator RLI = PRL.begin(),
         RLE = PRL.end(); RLI != RLE; ++RLI)
      InsertText((*RLI).second->getLocStart(), "continue; //");
  }
  else {
    SourceLocation RegionEnd;
    if (isa<ForStmt>(Region))
      RegionEnd = (cast<ForStmt>(Region))->getLocEnd().getLocWithOffset(2);
    if (isa<WhileStmt>(Region))
      RegionEnd = (cast<WhileStmt>(Region))->getLocEnd().getLocWithOffset(2);

    InsertText(Region->getLocStart(), "bool __kernel_invalid_index = false;");

    for (PairedReturnList::iterator RLI = PRL.begin(),
         RLE = PRL.end(); RLI != RLE; ++RLI)
      InsertText((*RLI).second->getLocStart(),
                 "__kernel_invalid_index = true; break; //");

    if (depth > 1)
      InsertText(RegionEnd, "if (__kernel_invalid_index) break;");
    else if (depth == 1)
      InsertText(RegionEnd, "if (__kernel_invalid_index) continue;");

    // if depth == 0 then the continue used inplace of the return is enough
  }

  ReturnStmts.erase(Region);
  //--depth;
}

void WorkitemCoarsen::ThreadSerialiser::SearchForIndVars(Stmt *s) {
#ifdef DBG_WRKGRP
  std::cerr << "SearchForIndVars" << std::endl;
#endif
  if (!s)
    return;

  //for (Stmt::child_iterator CI = s->child_begin(), CE = s->child_end();
    //   CI != CE; ++CI) {
    if (isa<BinaryOperator>(s)) {
      BinaryOperator *BO = cast<BinaryOperator>(s);
      if (BinaryOperator::isAssignmentOp(BO->getOpcode())) {
        Expr *LHS = BO->getLHS();
        if (isa<DeclRefExpr>(LHS)) {
          DeclRefExpr *ref = cast<DeclRefExpr>(LHS);
          Decl *decl = ref->getDecl();

          if (PotentialIndVars.find(decl) == PotentialIndVars.end()) {
            std::list<DeclRefExpr*> RefList;
            RefList.push_back(ref);
            PotentialIndVars.insert(std::make_pair(decl, RefList));
          }
          else {
            PotentialIndVars[decl].push_back(ref);
          }
        }
      }
    }
    else if (isa<UnaryOperator>(s)) {
      UnaryOperator *UO = cast<UnaryOperator>(s);
      for (Stmt::child_iterator UOI = UO->child_begin(), UOE = UO->child_end();
           UOI != UOE; ++UOI) {
        if (isa<DeclRefExpr>(*UOI)) {
          DeclRefExpr *ref = cast<DeclRefExpr>(*UOI);
          Decl *decl = ref->getDecl();

          if (PotentialIndVars.find(decl) == PotentialIndVars.end()) {
            std::list<DeclRefExpr*> RefList;
            RefList.push_back(ref);
            PotentialIndVars.insert(std::make_pair(decl, RefList));
          }
          else {
            PotentialIndVars[decl].push_back(ref);
          }
        }
      }
    }
  //}
}

void WorkitemCoarsen::ThreadSerialiser::CheckForUnary(Stmt *Region,
                                                      Stmt *Then,
                                                      Stmt *unary) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering CheckForUnary" << std::endl;
#endif
  if (isa<ContinueStmt>(unary)) {
    ContinueStmt *CS = cast<ContinueStmt>(unary);
    std::pair<Stmt*, ContinueStmt*> ContinuePair = std::make_pair(Then, CS);
    if (ContinueStmts.find(Region) == ContinueStmts.end()) {
      std::list<std::pair<Stmt*, ContinueStmt*> > NewList;
      NewList.push_back(ContinuePair);
      ContinueStmts.insert(std::make_pair(Region, NewList));
    }
    else
      ContinueStmts[Region].push_back(ContinuePair);
  }
  if (isa<BreakStmt>(unary)) {
#ifdef DBG_WRKGRP
    std::cerr << "Found BreakStmt" << std::endl;
#endif
    BreakStmt *BS = cast<BreakStmt>(unary);
    std::pair<Stmt*, BreakStmt*> BreakPair = std::make_pair(Then, BS);
    if (BreakStmts.find(Region) == BreakStmts.end()) {
      std::list<std::pair<Stmt*, BreakStmt*> > NewList;
      NewList.push_back(BreakPair);
      BreakStmts.insert(std::make_pair(Region, NewList));
    }
    else
      BreakStmts[Region].push_back(BreakPair);
  }
  if (isa<ReturnStmt>(unary)) {
#ifdef DBG_WRKGRP
    std::cerr << "Found ReturnStmt" << std::endl;
#endif
    ReturnStmt *RS = cast<ReturnStmt>(unary);
    std::pair<Stmt*, ReturnStmt*> ReturnPair = std::make_pair(Then, RS);
    if (ReturnStmts.find(Region) == ReturnStmts.end()) {
      std::list<std::pair<Stmt*, ReturnStmt*> > NewList;
      NewList.push_back(ReturnPair);
      ReturnStmts.insert(std::make_pair(Region, NewList));
    }
    else
      ReturnStmts[Region].push_back(ReturnPair);
  }
}

void WorkitemCoarsen::ThreadSerialiser::TraverseConditionalRegion(Stmt *Region,
                                                                  Stmt *s) {
#ifdef DBG_WRKGRP
  std::cerr << "TraverseConditionalRegion" << std::endl;
  s->dumpAll();
#endif
  IfStmt *ifStmt = cast<IfStmt>(s);

  Stmt *Then = ifStmt->getThen();

  if (Then->child_begin() == Then->child_end()) {
    CheckForUnary(Region, ifStmt, Then);
    return;
  }

  if (isa<CompoundStmt>(Then)) {
#ifdef DBG_WRKGRP
    std::cerr << "if statement body is a CompoundStmt" << std::endl;
#endif

    CompoundStmt *CS = cast<CompoundStmt>(Then);
    for (CompoundStmt::body_iterator CI = CS->body_begin(), CE = CS->body_end();
         CI != CE; ++CI) {

      if (!(*CI))
        continue;

#ifdef DBG_WRKGRP
      std::cerr << "iterating through ConditionalRegion" << std::endl;
#endif
      CheckForUnary(Region, ifStmt, *CI);

      if (isa<IfStmt>(*CI))
        TraverseConditionalRegion(Region, *CI);
      else if (isLoop(*CI))
        TraverseRegion(*CI);
    }
  }
  else {
    for (Stmt::child_iterator SI = Then->child_begin(), SE = Then->child_end();
         SI != SE; ++SI) {

      CheckForUnary(Region, ifStmt, *SI);
      if (isa<IfStmt>(*SI))
        TraverseConditionalRegion(Region, *SI);
      else if (isLoop(*SI))
        TraverseRegion(*SI);
    }
  }
}

// We shall create a map, using the outer loop as the key, to contain all it's
// loop children. We shall also then have a map of all the DeclStmts within the
// loop, plus a map of vectors which will hold barriers.
void WorkitemCoarsen::ThreadSerialiser::TraverseRegion(Stmt *s) {
#ifdef DBG_WRKGRP
  std::cerr << "TraverseRegion" << std::endl;
#endif
  Stmt *Body = NULL;
  // create vector to hold loop stmts
  std::vector<Stmt*> InnerLoops;
  // create a vector to hold decl stmts
  std::list<DeclStmt*> InnerDeclStmts;
  // create a vector to hold barriers
  std::vector<CallExpr*> InnerBarriers;
  // create a vector to hold scoped regions
  std::vector<CompoundStmt*> Compounds;

  if (isLoop(s)) {
    if (isa<WhileStmt>(s))
      Body = (cast<WhileStmt>(s))->getBody();
    else if (isa<ForStmt>(s)) {
      ForStmt *For = cast<ForStmt>(s);
      Body = For->getBody();
      SearchForIndVars(For->getInit());
      SearchForIndVars(For->getCond());
      SearchForIndVars(For->getInc());
    }
    // Iterate through the children of s:
    // - Add DeclStmts to vector
    // - Add Barriers to vector,
    // - Add loops to vector, and traverse the loop
    for (Stmt::child_iterator SI = Body->child_begin(),
         SE = Body->child_end(); SI != SE; ++SI) {

      if (isLoop(*SI)) {
        InnerLoops.push_back(*SI);
        // Recursively visit the loops from the parent
        TraverseRegion(*SI);
      }
      else if (isa<DeclStmt>(*SI)) {
        DeclStmt *stmt = cast<DeclStmt>(*SI);
        InnerDeclStmts.push_back(stmt);
        DeclParents.insert(std::make_pair(stmt->getSingleDecl(), s));
      }
      else if (isBarrier(*SI))
        InnerBarriers.push_back(cast<CallExpr>(*SI));


      else if (isa<CompoundStmt>(*SI)) {
        Compounds.push_back(cast<CompoundStmt>(*SI));
        TraverseRegion(*SI);
      }
      else if (isa<IfStmt>(*SI))
        TraverseConditionalRegion(s, *SI);
    }
  }
  // TODO Probably should just merge this into the code above, unless this needs
  // to be handled differently because its not a loop..? Can't remember!
  else if (isa<CompoundStmt>(s)) {
    CompoundStmt *CS = cast<CompoundStmt>(s);
    for (CompoundStmt::body_iterator CI = CS->body_begin(), CE = CS->body_end();
         CI != CE; ++CI) {

      if (isLoop(*CI)) {
        InnerLoops.push_back(*CI);
        // Recursively visit the loops from the parent
        TraverseRegion(*CI);
      }
      else if (isa<DeclStmt>(*CI)) {
        DeclStmt *stmt = cast<DeclStmt>(*CI);
        InnerDeclStmts.push_back(stmt);
        DeclParents.insert(std::make_pair(stmt->getSingleDecl(), s));
      }
      else if (isBarrier(*CI))
        InnerBarriers.push_back(cast<CallExpr>(*CI));

      else if (isa<CompoundStmt>(*CI)) {
        Compounds.push_back(cast<CompoundStmt>(*CI));
        TraverseRegion(*CI);
      }
      else if (isa<IfStmt>(*CI))
        TraverseConditionalRegion(s, *CI);
    }
  }

  // add vectors to the maps, using 's' as the key
  if (!InnerLoops.empty())
    NestedLoops.insert(std::make_pair(s, InnerLoops));

  if (!InnerDeclStmts.empty())
    ScopedDeclStmts.insert(std::make_pair(s, InnerDeclStmts));

  if (!InnerBarriers.empty())
    Barriers.insert(std::make_pair(s, InnerBarriers));

  if (!Compounds.empty())
    ScopedRegions.insert(std::make_pair(s, Compounds));
}

// Use this only as an entry into the kernel
bool WorkitemCoarsen::ThreadSerialiser::VisitForStmt(Stmt *s) {
#ifdef DBG_WRKGRP
  std::cerr << "VisitForStmt";
  if (isFirstLoop)
    std::cerr << " - is main loop";
  std::cerr << std::endl;
#endif
  ForStmt *For = cast<ForStmt>(s);

  if (isFirstLoop) {
    TraverseRegion(For);
    OuterLoop = For;
  }

  isFirstLoop = false;
  return true;
}

// Remove barrier calls and modify calls to kernel builtin functions.
/*
bool WorkitemCoarsen::ThreadSerialiser::VisitCallExpr(Expr *s) {
#ifdef DBG_WRKGRP
  std::cerr << "VisitCallExpr" << std::endl;
#endif
  if (isBarrier(s)) {
    CallExpr *Call = cast<CallExpr>(s);
    InsertText(Call->getLocStart(), "//");
    CloseLoop(Call->getLocEnd().getLocWithOffset(2));
    OpenLoop(Call->getLocEnd().getLocWithOffset(3));
  }
  return true;
}*/
/*
bool WorkitemCoarsen::ThreadSerialiser::VisitReturnStmt(Stmt *s) {
  //TheRewriter.InsertTextAfter(s->getLocEnd(), OpenWhile.str());
  //TheRewriter.InsertTextBefore(s->getLocStart(), CloseWhile.str());
  InsertText(s->getLocStart().getLocWithOffset(-2), CloseWhile.str());
  InsertText(s->getLocStart().getLocWithOffset(-1), OpenWhile.str());
  return true;
}*/

// Create maps of all the references in the tree
bool WorkitemCoarsen::ThreadSerialiser::VisitDeclRefExpr(Expr *expr) {
  DeclRefExpr *RefExpr = cast<DeclRefExpr>(expr);
  std::string VarName = RefExpr->getDecl()->getName().str();

#ifdef DBG_WRKGRP
  std::cerr << "VisitDeclRefExpr for " << VarName << std::endl;
#endif

  // Don't add it if its one of an work-item indexes
  if (VarName.compare("__esdg_idx") == 0)
    return true;
  if (VarName.compare("__esdg_idy") == 0)
    return true;
  if (VarName.compare("__esdg_idz") == 0)
    return true;

  Decl *key = RefExpr->getDecl();
  // Don't add it if it is a function parameter
  for (std::vector<Decl*>::iterator PI = ParamVars.begin(),
       PE = ParamVars.end(); PI != PE; ++PI) {
    if (key == (*PI)) {
#ifdef DBG_WRKGRP
      std::cerr << "Not adding " << VarName << " to AllRefs as it is an Arg"
        << std::endl;
#endif
      return true;
    }
  }

  // Either create a new set for the Ref, or add it an the existing one
  if (AllRefs.find(key) == AllRefs.end()) {
    DeclRefSet refset;
    refset.push_back(RefExpr);
    AllRefs.insert(std::make_pair(key, refset));
  }
  else {
    AllRefs[key].push_back(RefExpr);
  }

  return true;
}

// We visit unary and binary operators to collect Decls that get assigned, so
// we can determine possible induction variables. We also collect this data
// directly from loop headers, if we find that variables are only assigned in
// the header, then we assume it is an induction variable and doesn't need to
// be expanded.
bool WorkitemCoarsen::ThreadSerialiser::VisitUnaryOperator(Expr *expr) {
  UnaryOperator *UO = cast<UnaryOperator>(expr);
  for (Stmt::child_iterator CI = UO->child_begin(), CE = UO->child_end();
       CI != CE; ++CI) {

    if (isa<DeclRefExpr>(*CI)) {
      DeclRefExpr *ref = cast<DeclRefExpr>(*CI);
      Decl *decl = ref->getDecl();

      if (RefAssignments.find(decl) == RefAssignments.end()) {
        std::list<DeclRefExpr*> RefList;
        RefList.push_back(ref);
        RefAssignments.insert(std::make_pair(decl, RefList));
      }
      else {
        RefAssignments[decl].push_back(ref);
      }
    }
  }
  return true;
}

bool WorkitemCoarsen::ThreadSerialiser::VisitBinaryOperator(Expr *expr) {
  BinaryOperator *BO = cast<BinaryOperator>(expr);
  if (BinaryOperator::isAssignmentOp(BO->getOpcode())) {
    Expr *LHS = BO->getLHS();
    if (isa<DeclRefExpr>(LHS)) {
      DeclRefExpr *ref = cast<DeclRefExpr>(LHS);
      Decl *decl = ref->getDecl();

      if (RefAssignments.find(decl) == RefAssignments.end()) {
        std::list<DeclRefExpr*> RefList;
        RefList.push_back(ref);
        RefAssignments.insert(std::make_pair(decl, RefList));
      }
      else {
        RefAssignments[decl].push_back(ref);
      }
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
      ParamVars.push_back((*PI));
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
                                                   unsigned z,
                                                   std::string &name) {

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
  TheCompInst.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDE_DIR,
                                            clang::frontend::Angled,
                                            false, false, false);
  TheCompInst.getHeaderSearchOpts().AddPath(
    CLANG_INCLUDE_DIR, clang::frontend::Angled,
    false, false, false);
  TheCompInst.getHeaderSearchOpts().ResourceDir = CLANG_RESOURCE_DIR;
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
  TheConsumer = new OpenCLASTConsumer(TheRewriter, x, y, z, name);
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
  TheCompInst.getDiagnosticClient().BeginSourceFile(TheCompInst.getLangOpts(),
                                              &TheCompInst.getPreprocessor());
}

template <typename T>
void WorkitemCoarsen::OpenCLCompiler<T>::Parse() {
  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), TheConsumer,
           TheCompInst.getASTContext());
}

