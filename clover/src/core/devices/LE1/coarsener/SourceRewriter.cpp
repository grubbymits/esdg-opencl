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
  if (RewriteBuf == NULL) {
#ifdef DBG_OUTPUT
    std::cout << "ERROR: No RewriteBuf in CreateWorkgroup" << std::endl;
#endif
    return false;
  }

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

  if (!InitCompiler.isParallel())
    return HandleBarriers();
  else {
    FinalKernel = InitKernelSource;
    return true;
  }
}

bool WorkitemCoarsen::HandleBarriers() {
#ifdef DBG_WRKGRP
  std::cerr << "HandleBarriers" << std::endl;
#endif
  OpenCLCompiler<ThreadSerialiser> SerialCompiler(LocalX, LocalY, LocalZ,
                                                  KernelName);
  SerialCompiler.setFile(InitKernelFilename);
  SerialCompiler.Parse();

  const RewriteBuffer *RewriteBuf = SerialCompiler.getRewriteBuf();
  if (RewriteBuf == NULL) {
#ifdef DBG_OUTPUT
    std::cout << "ERROR: No RewriteBuf in HandleBarriers" << std::endl;
#endif
    return false;
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
  if (LocalZ > 0) {
    //OpenWhile  << "while (__kernel_local_id[2] < " << LocalZ << ") {\n";
    OpenWhile << "for (__esdg_idz = 0; __esdg_idz < " << LocalZ
      << "; ++__esdg_idz) {\n";
  }
  if (LocalY > 0) {
    //OpenWhile << "while (__kernel_local_id[1] < " << LocalY << ") {\n";
    OpenWhile << "for (__esdg_idy = 0; __esdg_idy < " << LocalY
      << "; ++__esdg_idy) {\n";
  }
  if (LocalX > 0) {
    //OpenWhile << "while (__kernel_local_id[0] < " << LocalX << ") {\n";
    OpenWhile << "for (__esdg_idx = 0; __esdg_idx < " << LocalX
      << "; ++__esdg_idx) {\n";
  }

  // We reset them at the end just in case a proceeding loop uses a expanded
  // variable. The variable should have only been expanded because we select
  // the variables very, very lazily. The loop would contain a barrier of sorts
  // and a barrier cannot be executed because of a thread local variable...
  if (LocalX > 0) {
    //CloseWhile << "\n__kernel_local_id[0]++;\n";
    CloseWhile  << "\n} __esdg_idx = 0;\n";
    //CloseWhile << "__kernel_local_id[0] = 0;\n";
  }
  if (LocalY > 0) {
    //CloseWhile << " __kernel_local_id[1]++;\n";
    CloseWhile << "\n} __esdg_idy = 0;\n";
    //CloseWhile << "__kernel_local_id[1] = 0;\n";
  }
  if (LocalZ > 0) {
    //CloseWhile << "__kernel_local_id[2]++;\n";
    CloseWhile << "\n} __esdg_idz = 0;\n";
    //CloseWhile << "\n__kernel_local_id[2] = 0;\n";
  }

  foundBarrier = false;
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
    if (LocalY > 0)
      FuncBegin << "  unsigned __esdg_idy = 0;";
    if (LocalZ > 0)
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
    std::string finalClose = "\n__ESDG_END: ;";
    finalClose.append(CloseWhile.str());
    InsertText(FuncBody->getLocEnd(), finalClose);
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

bool WorkitemCoarsen::KernelInitialiser::VisitReturnStmt(Stmt *s) {
  ReturnStmts.push_back(s);
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
  else if (isBarrier(Call))
    foundBarrier = true;

  return true;
}

void WorkitemCoarsen::KernelInitialiser::FixReturns() {
  if (ReturnStmts.empty())
    return;

  for (std::vector<Stmt*>::iterator RI = ReturnStmts.begin(),
       RE = ReturnStmts.end(); RI != RE; ++RI)
    InsertText((*RI)->getLocStart(), "goto __ESDG_END; //");
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

static bool isWorkgroupLoop(Stmt *s) {
  bool isWorkgroup = false;
  if (isa<ForStmt>(s)) {
    Stmt *init = (cast<ForStmt>(s))->getInit();
    for (Stmt::child_iterator SI = init->child_begin(), SE = init->child_end();
         SI != SE; ++SI) {
      if (isa<DeclRefExpr>(*SI)) {
        DeclRefExpr *expr = cast<DeclRefExpr>(*SI);
        NamedDecl *ND = cast<NamedDecl>(expr->getDecl());
        std::string name = ND->getName().str();
        isWorkgroup |= (name.compare("__esdg_idx") ||
                        name.compare("__esdg_idy") ||
                        name.compare("__esdg_idz"));
      }
    }
  }
#ifdef DBG_WRKGRP
  if (isWorkgroup)
    std::cerr << "Region is a workgroup loop" << std::endl;
#endif
  return isWorkgroup;
}

WorkitemCoarsen::ThreadSerialiser::ThreadSerialiser(Rewriter &R,
                                                    unsigned x,
                                                    unsigned y,
                                                    unsigned z)
  : ASTVisitorBase(R, x, y, z) {

    isFirstLoop = true;
    foundBarrier = true;
    numDimensions = 1;
    if (y)
      ++numDimensions;
    if (z)
      ++numDimensions;
    /*

    InvalidThreadInit << "bool __kernel_invalid_global_threads[" << x << "]";
    if (y > 0) {
      InvalidThreadInit << "[" << y << "]";
      ++numDimensions;
    }
    if (z > 0) {
      InvalidThreadInit << "[" << z << "]";
      ++numDimensions;
    }
    InvalidThreadInit << " = {false};\n";
    InvalidThreadInit << "unsigned __kernel_total_invalid_global_threads = 0;"
      << std::endl;
    returnFixer = new ReturnFixer(&SourceToInsert, x, y, z);
    breakFixer = new BreakFixer(&SourceToInsert, x, y, z);
    */
}

WorkitemCoarsen::ThreadSerialiser::~ThreadSerialiser() {
  //delete returnFixer;
  //delete breakFixer;
}

void WorkitemCoarsen::ThreadSerialiser::RewriteSource() {
#ifdef DBG_WRKGRP
  std::cerr << "ThreadSerialiser::RewriteSource" << std::endl;
#endif

  // Visit all the loops which contain barriers, and create regions that are
  // contained by the multi-level while loops
  SearchThroughRegions(OuterLoop, Barriers.empty());

  // Then we see which variables are only assigned in loop headers, these
  // are then disabled for expansion, though they can still be made local.
  //AssignIndVars();

  // Then we need to find all the variables that we need to replicate.
  std::list<DeclStmt*> Stmts;
  //FindRefsToExpand(Stmts, OuterLoop);
  FindRefsToExpand();

  // Once we have decided all the text we need to insert, sort it and write it
  SourceToInsert.sort(SortLocations);

  for (std::list<std::pair<SourceLocation, std::string> >::iterator PI =
       SourceToInsert.begin(), PE = SourceToInsert.end(); PI != PE; ++PI) {
    SourceLocation Loc = (*PI).first;
    std::string Text = (*PI).second;
    TheRewriter.InsertText(Loc, Text);
  }
}

void WorkitemCoarsen::ThreadSerialiser::FindRefsToExpand() {
#ifdef DBG_WRKGRP
  std::cerr << "FindRefsToExpand" << std::endl;
#endif
  // Iterate through all Regions
  for (std::map<Stmt*, std::list<DeclStmt*> >::iterator I
       = ScopedDeclStmts.begin(), E = ScopedDeclStmts.end();
       I != E; ++I) {
    Stmt *Region = (*I).first;
    std::list<DeclStmt*> declStmts = (*I).second;

    // Iterate through all the variable declarations
    for (declstmt_iterator DSI = declStmts.begin(), DSE = declStmts.end();
         DSI != DSE; ++DSI) {

      Decl *decl = (*DSI)->getSingleDecl();
#ifdef DBG_WRKGRP
      std::cerr << "Variable: " << (cast<NamedDecl>(decl))->getName().str()
        << std::endl;
#endif

      SourceLocation declLoc = (*DSI)->getLocStart();
      std::vector<DeclRefExpr*> declRefs = AllRefs[decl];

      // And all their references
      for (declref_iterator DRI = declRefs.begin(), DRE = declRefs.end();
           DRI != DRE; ++DRI) {

        bool hasExpanded = false;
        SourceLocation refLoc = (*DRI)->getLocStart();

        // Compare declaration locations and their references with fission
        // locations
        for (std::vector<SourceLocation>::iterator SI = FissionLocs.begin(),
             SE = FissionLocs.end(); SI != SE; ++SI) {

          SourceLocation fissionLoc = (*SI);

          // skip fission points before the declaration
          if (fissionLoc < declLoc)
            continue;

          if (fissionLoc < refLoc) {
            SourceLocation InsertLoc = Region->getLocStart();
            if (isWorkgroupLoop(Region))
              InsertLoc = FuncBodyStart;

#ifdef DBG_WRKGRP
            std::cerr << "Fission location between declaration and reference"
              << std::endl;
#endif
            ScalarExpand(InsertLoc, *DSI);
            hasExpanded = true;
            break;
          }
        }
        if (hasExpanded)
          break;
      }
    }
  }
}
/*
void WorkitemCoarsen::ThreadSerialiser::FindRefsToExpand(
  std::list<DeclStmt*> &Stmts, Stmt *Region) {

#ifdef DBG_WRKGRP
  std::cerr << "FindRefsToExpand" << std::endl;
#endif

  // Again, quit early if there's gonna be nothing to find.
  if ((NestedRegions[Region].empty()) && (Barriers[Region].empty()))
      //ScopedRegions[Loop].empty())
    return;

  // Get the DeclStmts of this loop, append it to the vector that has been
  // passed; so the search space grows as we go deeper into the loops.
  std::list<DeclStmt*> InnerDeclStmts = ScopedDeclStmts[Region];

  // TODO Combine loops and regions

  if (!NestedRegions[Region].empty()) {
    std::list<Stmt*> InnerRegions = NestedRegions[Region];
    //Stmts.insert(Stmts.end(), InnerDeclStmts.begin(), InnerDeclStmts.end());

    declstmt_iterator OrigEnd = Stmts.end();
    Stmts.splice(Stmts.end(), InnerDeclStmts);

    for (region_iterator RI = InnerRegions.begin(),
         RE = InnerRegions.end(); RI != RE ; ++RI) {
      // Recursively visit inner loops to build up the vector of referable
      // DeclStmts.
      FindRefsToExpand(Stmts, *RI);
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
  std::list<CallExpr*> InnerBarriers = Barriers[Region];

  for (declstmt_iterator DSI = InnerDeclStmts.begin(),
       DSE = InnerDeclStmts.end(); DSI != DSE; ++DSI) {

    std::vector<DeclRefExpr*> Refs = AllRefs[(*DSI)->getSingleDecl()];
    SourceLocation DeclLoc = (*DSI)->getLocStart();
    bool hasExpanded = false;

    for (barrier_iterator BI = InnerBarriers.begin(),
         BE = InnerBarriers.end(); BI != BE; ++BI) {

      // If it's already been replicated, we don't need to check any more
      // barriers.
      if (hasExpanded)
        break;

      SourceLocation BarrierLoc = (*BI)->getLocStart();

      for (declref_iterator RI = Refs.begin(),
           RE = Refs.end(); RI != RE; ++RI) {

        SourceLocation RefLoc = (*RI)->getLocStart();

        if ((BarrierLoc < RefLoc) && (DeclLoc < BarrierLoc)) {
          SourceLocation InsertLoc;
          if (isa<ForStmt>(Region)) {
            if (isWorkgroupLoop(cast<ForStmt>(Region))) {
              InsertLoc = FuncBodyStart;
            }
            else
              InsertLoc = Region->getLocStart();
          }
          ScalarExpand(InsertLoc, *DSI);
          DSI = Stmts.erase(DSI);
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
  SourceLocation RegionStart;
  SourceLocation RegionEnd;
  if (isa<WhileStmt>(Region)) {
    RegionStart = (cast<WhileStmt>(Region))->getBody()->getLocStart();
    RegionEnd = (cast<WhileStmt>(Region))->getBody()->getLocEnd();
  }
  else if (isa<ForStmt>(Region)) {
    RegionStart = (cast<ForStmt>(Region))->getBody()->getLocStart();
    RegionEnd = (cast<ForStmt>(Region))->getBody()->getLocEnd();
  }
  else if (isa<CompoundStmt>(Region)) {
    RegionStart = (cast<CompoundStmt>(Region))->getLBracLoc();
    RegionEnd = (cast<CompoundStmt>(Region))->getRBracLoc();
  }
  else {
    RegionStart = Region->getLocStart();
    RegionEnd = Region->getLocEnd().getLocWithOffset(1);
  }

  for (declstmt_iterator DSI = Stmts.begin(), DSE = Stmts.end();
       DSI != DSE;) {

    bool hasExpanded = false;
#ifdef DBG_WRKGRP
    std::cerr << "Checking var: "
      << cast<NamedDecl>((*DSI)->getSingleDecl())->getName().str() << std::endl;
#endif

    std::vector<DeclRefExpr*> Refs = AllRefs[(*DSI)->getSingleDecl()];

    for (declref_iterator RI = Refs.begin(), RE = Refs.end();
         RI != RE; ++RI) {

      SourceLocation RefLoc = (*RI)->getLocStart();

      if ((RegionStart < RefLoc) && (RefLoc < RegionEnd)) {
        SourceLocation InsertLoc = DeclParents[(*RI)->getDecl()]->getLocStart();
        if (isa<ForStmt>(Region)) {
          if (isWorkgroupLoop(cast<ForStmt>(Region))) {
            InsertLoc = FuncBodyStart;
          }
        }
        if (isa<ForStmt>(DeclParents[(*RI)->getDecl()])) {
          if (isWorkgroupLoop(cast<ForStmt>(DeclParents[(*RI)->getDecl()])))
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

}*/

inline void
WorkitemCoarsen::ThreadSerialiser::ExpandDecl(std::stringstream &NewDecl) {
#ifdef DBG_WRKGRP
  //std::cerr << "ExpandDecl" << std::endl;
#endif
  if (LocalX != 0)
    NewDecl << "[" << LocalX << "]";
  if (LocalY > 0)
    NewDecl << "[" << LocalY << "]";
  if (LocalZ > 0)
    NewDecl << "[" << LocalZ << "]";
}

inline void
WorkitemCoarsen::ThreadSerialiser::ExpandRef(std::stringstream &NewRef) {
#ifdef DBG_WRKGRP
  //std::cerr << "ExpandRef" << std::endl;
#endif
  if (LocalX != 0)
    NewRef << "[__esdg_idx]"; //"[__kernel_local_id[0]]";
  if (LocalY > 0)
    NewRef << "[__esdg_idy]"; //"[__kernel_local_id[1]]";
  if (LocalZ > 0)
    NewRef << "[__esdg_idz]"; //"[__kernel_local_id[2]]";
}

// Run this after parsing is complete, to access all the referenced variables
// within the DeclStmt which need to be replicated
void
WorkitemCoarsen::ThreadSerialiser::ScalarExpand(SourceLocation InsertLoc,
                                                DeclStmt *DS) {
#ifdef DBG_WRKGRP
  //std::cerr << "Entering ScalarExpand " << pthread_self() << std::endl;
#endif
  bool threadDep = true;
  if (threadDepVars.find(DS->getSingleDecl()) == threadDepVars.end())
    threadDep = false;

  // If the variable is a local variable, we don't need to expand it for
  // each of the work items.
  QualType qualType = (cast<ValueDecl>(DS->getSingleDecl()))->getType();
  bool isVariableLocal = (qualType.getAddressSpace() == LangAS::opencl_local) ?
    true : false;

  bool toExpand = !isVariableLocal & threadDep;

  bool isScalar = CreateLocal(InsertLoc, DS, toExpand);//!isVariableLocal);

  // Then the original declaration is turned into a reference, creating a
  // scalar access to a new array if necessary.
  if (isScalar)
    RemoveScalarDeclStmt(DS, toExpand); //!isVariableLocal);
  else
    RemoveNonScalarDeclStmt(DS, toExpand); //!isVariableLocal);

  // No need to visit its references if it's __local
  if (isVariableLocal || !threadDep)
    return;

  std::vector<DeclRefExpr*> theRefs = AllRefs[DS->getSingleDecl()];
  // Then visit all the references to turn them into scalar accesses as well.
  for (declref_iterator RI = theRefs.begin(),
       RE = theRefs.end(); RI != RE; ++RI) {
    if (isScalar)
      AccessScalar(*RI);
    else
      AccessNonScalar(*RI);
  }
#ifdef DBG_WRKGRP
  //std::cerr << "Leaving ScalarExpand" << std::endl;
#endif
}

bool WorkitemCoarsen::ThreadSerialiser::CreateLocal(SourceLocation InsertLoc,
                                                    DeclStmt *s,
                                                    bool toExpand) {
#ifdef DBG_WRKGRP
  //std::cerr << "Entering CreateLocal ";
  //if (toExpand)
    //std::cerr << " - going to expand variable ";
  //std::cerr << std::endl;
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
  std::cerr << "Created local: " << NewDecl.str() << std::endl;
#endif

  return isScalar;
}

// Turn the old declaration into a reference
void WorkitemCoarsen::ThreadSerialiser::RemoveScalarDeclStmt(DeclStmt *DS,
                                                             bool toExpand) {
#ifdef DBG_WRKGRP
  //std::cerr << "Entering RemoveScalarDeclStmt:\n" << std::endl;
#endif

  VarDecl *VD = cast<VarDecl>(DS->getSingleDecl());
  NamedDecl *ND = cast<NamedDecl>(DS->getSingleDecl());

  std::string varName = ND->getName().str();

  if (!VD->hasInit()) {
#ifdef DBG_WRKGRP
    //std::cerr << "DeclStmt has no initialiser" << std::endl;
#endif
    TheRewriter.RemoveText(DS->getSourceRange());
    return;
  }

  Expr* varInit = VD->getInit();
#ifdef DBG_WRKGRP
  //std::cerr << "Initialiser = ";
  //varInit->dumpPretty(VD->getASTContext());
  //std::cerr << std::endl;
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
  //std::cerr << "Leaving RemoveScalarDeclStmt" << std::endl;
#endif
}
// Turn the old declaration into a reference
void WorkitemCoarsen::ThreadSerialiser::RemoveNonScalarDeclStmt(DeclStmt *DS,
                                                                bool toExpand) {
#ifdef DBG_WRKGRP
  //std::cerr << "Entering RemoveNonScalarDeclStmt" << std::endl;
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
  //std::cerr << "Leaving RemoveNonScalarDeclStmt" << std::endl;
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

  // FIXME This is weird, why is this separate from scalar?
  if (LocalY > 0)
    InsertText(Loc, "[__esdg_idx][__esdg_idx]"); //"[__kernel_local_id[0]]");
  else
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
bool
WorkitemCoarsen::ThreadSerialiser::SearchThroughRegions(Stmt *Region,
                                                        bool isParentParallel) {
#ifdef DBG_WRKGRP
  std::cerr << "SearchThroughRegions, Parent is ";
  if (!isParentParallel)
    std::cerr << "not ";
  std::cerr << "parallel" << std::endl;
  if (isa<IfStmt>(Region))
    std::cerr << "Region is also an IfStmt" << std::endl;
#endif
  static int depth = -1;
  bool foundBarrier = false;
  ++depth;

  // Loop regions can contain breaks and continues, but these will often be
  // contained in conditional regions; for this reason we have mapped those
  // unaries to the loop if the conditional region did not contain a barrier.
  // if (foundStmts && BARRIER)
  //    any continue or break statements need to become barriers and the unary
  //    is executed just once for the whole workgroup.
  //
  //  for () {            for () {
  //    if ()               thread_loop() { }
  //      break;            if () {
  //    barrier();            thread_loop() { }
  //  }                       break;
  //                        }
  //                        // barrier()
  //                        thread_loop() { }
  //                      }
  //
  // This means that conditional regions need to be treated as normal regions
  // because we need to open and close the thread loop as we would for barriers.
  // The only difference is that instead of the barrier being removed, we keep
  // the unary in, but it executes outside of the thread loop. Like we search
  // for nested barriers, we're going to need to search for nested unaries.
  // In the example above, the region containing the break is acyclic and does
  // not contain a barrier - so from this small perspective, nothing has to be
  // changed. If we tell the conditional child region that its parent is not
  // parallel then we can do something about it.

  // SearchThroughRegions(Stmt *Region, bool isParentParallel)
  //  if (!InnerContinues.empty() && !InnerBarriers.empty())
  //    FixBarrierContinues(InnerContinues);
  //
  //  if (!InnerReturns.empty() && (!isParentParallel ||
  //      (isLoop && !InnerBarrier.empty())))
  //      FixBarrierReturns(InnerReturns);

  // return statements are similar, though they do not have to live within a
  // loop, but they always need to be handled. Return statements are barriers
  // when:
  // - if they preceed any barrier calls
  // - they are in a cyclic region that contains a barrier, like the example
  //   above.
  // Maybe easiest to do a comparison on SourceLocations between all returns and
  // all the barriers, handling returns as necessary and removing them from the
  // list. Then we can SearchThroughRegions and fix any remaining statements
  // that weren't simply handled by location.

  if ((NestedRegions[Region].empty()) && (Barriers[Region].empty())) {
#ifdef DBG_WRKGRP
    std::cerr << "No nested loops and there's no barriers in this one"
      << std::endl;
#endif
    if (!isParentParallel && isa<IfStmt>(Region)) {
#ifdef DBG_WRKGRP
      std::cerr << "But this is a IfStmt and the parent is not parallel"
        << std::endl;
#endif
      if (!ContinueStmts[Region].empty() || !BreakStmts[Region].empty() ||
          !ReturnStmts[Region].empty()) {
#ifdef DBG_WRKGRP
        std::cerr << "And the region contains unaries!" << std::endl;
#endif
        HandleNonParallelRegion(Region, depth);
        foundBarrier = true;
      }
    }
    else if (!ReturnStmts[Region].empty()) {
      HandleNonParallelRegion(Region, depth);
      foundBarrier = true;
    }

    --depth;
    return foundBarrier;
  }

  unsigned NestedBarriers = 0;
  std::list<Stmt*> InnerRegions = NestedRegions[Region];
  for (region_iterator RI = InnerRegions.begin(),
       RE = InnerRegions.end(); RI != RE; ++RI) {
    bool parallelRegion = ((NestedBarriers == 0) && Barriers[Region].empty());
    if (SearchThroughRegions(*RI, parallelRegion))
      ++NestedBarriers;
  }

  if (!Barriers[Region].empty() || (NestedBarriers != 0) ||
      !ReturnStmts[Region].empty()) {
    HandleNonParallelRegion(Region, depth);
    --depth;
    return true;
  }
  else {
    --depth;
    return false;
  }
}

inline void WorkitemCoarsen::ThreadSerialiser::FixUnary(Stmt *Unary) {
  unsigned offset = 0;
  if (isa<BreakStmt>(Unary))
    offset = 6;
  else if (isa<ContinueStmt>(Unary))
    offset = 10;
  else if (isa<ReturnStmt>(Unary))
    offset = 7;

  CloseLoop(Unary->getLocStart().getLocWithOffset(-1));
  OpenLoop(Unary->getLocEnd().getLocWithOffset(offset));
  FissionLocs.push_back(Unary->getLocStart());
}

inline void WorkitemCoarsen::ThreadSerialiser::FixBarrier(CallExpr *Call) {
  InsertText(Call->getLocStart(), "//");
  CloseLoop(Call->getLocEnd().getLocWithOffset(2));
  OpenLoop(Call->getLocEnd().getLocWithOffset(3));
  FissionLocs.push_back(Call->getLocStart());
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
  if (!Barriers[Region].empty()) {
    if (Barriers[Region].size() == 1) {
      FixBarrier(Barriers[Region].front());
    }
    else if (Barriers[Region].size () > 1) {
      for (barrier_iterator Call = Barriers[Region].begin(),
           End = Barriers[Region].end(); Call != End; ++Call)
        FixBarrier(*Call);
    }
  }
  // Use any continue stmts as fission points too
  if (!ContinueStmts[Region].empty()) {
    if (ContinueStmts[Region].size() == 1)
      FixUnary(ContinueStmts[Region].front());
    else {
      for (continue_iterator CI = ContinueStmts[Region].begin(),
           CE = ContinueStmts[Region].end(); CI != CE; ++CI)
        FixUnary(*CI);
    }
  }
  // As well as any breaks
  if (!BreakStmts[Region].empty()) {
    if (BreakStmts[Region].size() == 1)
      FixUnary(BreakStmts[Region].front());
    else {
      for (break_iterator BI = BreakStmts[Region].begin(),
           BE = BreakStmts[Region].end(); BI != BE; ++BI)
        FixUnary(*BI);
    }
  }
  if (!ReturnStmts[Region].empty()) {
#ifdef DBG_WRKGRP
    std::cerr << "This region contains returns" << std::endl;
#endif
    for (return_iterator RI = ReturnStmts[Region].begin(),
         RE = ReturnStmts[Region].end(); RI != RE; ++RI)
      FixUnary(*RI);
  }

  // Insert opening and closing loops for all regions, except the outer loop
  if (depth > (numDimensions - 1)) {
    FissionLocs.push_back(Region->getLocStart());

    Stmt *LoopBody = NULL;
    if (isa<IfStmt>(Region)) {
      CloseLoop(Region->getLocStart().getLocWithOffset(-1));
      OpenLoop(Region->getLocEnd().getLocWithOffset(1));

      IfStmt *ifStmt = cast<IfStmt>(Region);
      OpenLoop(ifStmt->getThen()->getLocStart().getLocWithOffset(1));
      CloseLoop(ifStmt->getThen()->getLocEnd());

      if (Stmt *Else = ifStmt->getElse()) {
        OpenLoop(Else->getLocStart().getLocWithOffset(1));
        CloseLoop(Else->getLocEnd());
      }
      return;
    }
    if (isa<ForStmt>(Region))
      LoopBody = (cast<ForStmt>(Region))->getBody();
    else if (isa<WhileStmt>(Region))
      LoopBody = (cast<WhileStmt>(Region))->getBody();
    if (!isa<CompoundStmt>(Region)) {
      CloseLoop(Region->getLocStart().getLocWithOffset(-1));
      OpenLoop(GetOffsetInto(LoopBody->getLocStart()));
      CloseLoop(LoopBody->getLocEnd());
      OpenLoop(GetOffsetOut(Region->getLocEnd()));
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
  std::list<ReturnStmt*> returns = ReturnStmts[Region];

  if (depth == 0) {
    // Convert returns in the outer loop to continues.
    for (return_iterator RI = returns.begin(),
         RE = returns.end(); RI != RE; ++RI)
      InsertText((*RI)->getLocStart(), "continue; //");
  }
  else {
    SourceLocation RegionEnd;
    if (isa<ForStmt>(Region))
      RegionEnd = (cast<ForStmt>(Region))->getLocEnd().getLocWithOffset(2);
    else if (isa<WhileStmt>(Region))
      RegionEnd = (cast<WhileStmt>(Region))->getLocEnd().getLocWithOffset(2);
    else
      RegionEnd = Region->getLocEnd().getLocWithOffset(2);

    InsertText(Region->getLocStart(), "bool __kernel_invalid_index = false;");

    for (return_iterator RI = returns.begin(),
         RE = returns.end(); RI != RE; ++RI)
      InsertText((*RI)->getLocStart(),
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

inline void WorkitemCoarsen::ThreadSerialiser::FindThreadDeps(Stmt *s,
                                                              bool depRegion) {
#ifdef DBG_WRKGRP
  //std::cerr << "FindThreadDeps" << std::endl;
#endif
  if (isa<BinaryOperator>(s))
    CheckBinaryOpDep(cast<Expr>(s), depRegion);
  else if (isa<DeclStmt>(s))
    CheckDeclStmtDep(s, depRegion);
  else if (isa<UnaryOperator>(s))
    CheckUnaryOpDep(cast<Expr>(s), depRegion);
  else if (isa<ArraySubscriptExpr>(s))
    CheckArrayDeps(cast<Expr>(s), depRegion);
}

#define NONE      0
#define BARRIER   1
#define RETURN    2
#define BREAK     4
#define CONTINUE  8

static inline unsigned CheckForUnary(Stmt *Region, Stmt *unary) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering CheckForUnary" << std::endl;
  //unary->dumpAll();
#endif
  if (isa<ContinueStmt>(unary)) {
#ifdef DBG_WRKGRP
    std::cerr << "FOUND CONTINUESTMT" << std::endl;
#endif
    return CONTINUE;
  }
  else if (isa<BreakStmt>(unary)) {
#ifdef DBG_WRKGRP
    std::cerr << "FOUND BREAKSTMT" << std::endl;
#endif
    return BREAK;
  }
  else if (isa<ReturnStmt>(unary)) {
#ifdef DBG_WRKGRP
    std::cerr << "FOUND RETURNSTMT" << std::endl;
#endif
    return RETURN;
  }
  else
    return NONE;
}

// TODO Not sure if isConditional should be passed from parent or not, since
// conditional regions will pass their parent, as the new parent...
unsigned WorkitemCoarsen::ThreadSerialiser::TraverseRegion(Stmt *Parent,
                                                           Stmt *Region,
                                                           bool isConditional,
                                                           bool isThreadDep) {
#ifdef DBG_WRKGRP
  std::cerr << "TraverseRegion";
  if (isConditional)
    std::cerr << " - isConditional";
  if (isThreadDep)
    std::cerr << " - isThreadDep";
  std::cerr << std::endl;
#endif

  unsigned foundStmts = 0;
  Stmt *Body = NULL;
  std::list<Stmt*> InnerRegions;
  std::list<DeclStmt*> InnerDeclStmts;
  std::list<CallExpr*> InnerBarriers;
  std::list<ContinueStmt*> InnerContinues;
  std::list<BreakStmt*> InnerBreaks;
  std::list<ReturnStmt*> InnerReturns;

  // Check conditional regions. if-statements are first class regions IFF they
  // contain a barrier.
  if (isa<IfStmt>(Region)) {
    IfStmt *ifStmt = cast<IfStmt>(Region);
    Expr *Cond = ifStmt->getCond();
    FindThreadDeps(Cond, isThreadDep);
    isThreadDep |= SearchExpr(Cond);

    Stmt *Then = ifStmt->getThen();
    if (Then->child_begin() != Then->child_end())
      foundStmts |= TraverseRegion(Region, Then, true, isThreadDep);
    else {
      switch(CheckForUnary(Region, Then)) {
      default:
        break;
      case CONTINUE:
        InnerContinues.push_back(cast<ContinueStmt>(Then));
        foundStmts |= CONTINUE;
        break;
      case BREAK:
        InnerBreaks.push_back(cast<BreakStmt>(Then));
        foundStmts |= BREAK;
        break;
      case RETURN:
        InnerReturns.push_back(cast<ReturnStmt>(Then));
        foundStmts |= RETURN;
      }
      FindThreadDeps(Then, isThreadDep);
    }

    if (Stmt *Else = ifStmt->getElse())
      foundStmts |= TraverseRegion(Region, Else, true, isThreadDep);
  }
  // Check through the body of a loop
  else if (isLoop(Region)) {
    if (isa<WhileStmt>(Region)) {
      WhileStmt *While = (cast<WhileStmt>(Region));
      Body = While->getBody();
      FindThreadDeps(While->getCond(), isThreadDep);
      isThreadDep |= SearchExpr(While->getCond());
    }
    else if (isa<ForStmt>(Region)) {
      ForStmt *For = cast<ForStmt>(Region);
      Body = For->getBody();

      // Don't check for dependencies in the outer thread loop
      if (Parent != Region) {
        FindThreadDeps(For->getInit(), isThreadDep);
        FindThreadDeps(For->getCond(), isThreadDep);
        FindThreadDeps(For->getInc(), isThreadDep);
        isThreadDep |= SearchExpr(cast<Expr>(For->getCond()));
        isThreadDep |= SearchExpr(cast<Expr>(For->getInc()));
      }
    }
    // Iterate through the children of s:
    // - Add DeclStmts to vector
    // - Add Barriers to vector,
    // - Add loops to vector, and traverse the loop
    for (Stmt::child_iterator SI = Body->child_begin(),
         SE = Body->child_end(); SI != SE; ++SI) {

      if (isLoop(*SI) || isa<CompoundStmt>(*SI)) {
        // Recursively visit the loops from the parent
        foundStmts |= TraverseRegion(Region, *SI, isConditional, isThreadDep);
        InnerRegions.push_back(*SI);
      }
      else if (isa<DeclStmt>(*SI)) {
        DeclStmt *stmt = cast<DeclStmt>(*SI);
        InnerDeclStmts.push_back(stmt);
        DeclParents.insert(std::make_pair(stmt->getSingleDecl(), Region));
      }
      else if (isBarrier(*SI))
        InnerBarriers.push_back(cast<CallExpr>(*SI));
      else if (isa<IfStmt>(*SI)) {
        IfStmt *ifStmt = cast<IfStmt>(*SI);
        Expr *Cond = ifStmt->getCond();
        //isThreadDep |= SearchExpr(Cond);
        foundStmts |= TraverseRegion(Region, *SI, true,
                                     (isThreadDep | SearchExpr(Cond)));
        InnerRegions.push_back(*SI);
      }
      else {
        switch(CheckForUnary(Region, *SI)) {
        default:
          break;
        case CONTINUE:
          InnerContinues.push_back(cast<ContinueStmt>(*SI));
          foundStmts |= CONTINUE;
          break;
        case BREAK:
          InnerBreaks.push_back(cast<BreakStmt>(*SI));
          foundStmts |= BREAK;
          break;
        case RETURN:
          InnerReturns.push_back(cast<ReturnStmt>(*SI));
          foundStmts |= RETURN;
        }
      }
      FindThreadDeps(*SI, isThreadDep);
    }
  }
  // Or check through a CompoundStmt - this could be a scoped region or an
  // then/else body.
  else if (isa<CompoundStmt>(Region)) {
    CompoundStmt *CS = cast<CompoundStmt>(Region);
    for (Stmt::child_iterator CI = CS->body_begin(),
         CE = CS->child_end(); CI != CE; ++CI) {

      if (isLoop(*CI) || isa<CompoundStmt>(*CI)) {
        // Recursively visit the loops from the parent
        foundStmts |= TraverseRegion(Region, *CI, isConditional, isThreadDep);
        InnerRegions.push_back(*CI);
      }
      else if (isa<DeclStmt>(*CI)) {
        DeclStmt *stmt = cast<DeclStmt>(*CI);
        InnerDeclStmts.push_back(stmt);
        DeclParents.insert(std::make_pair(stmt->getSingleDecl(), Region));
      }
      else if (isBarrier(*CI))
        InnerBarriers.push_back(cast<CallExpr>(*CI));
      else if (isa<IfStmt>(*CI)) {
        IfStmt *ifStmt = cast<IfStmt>(*CI);
        Expr *Cond = ifStmt->getCond();
        //isThreadDep |= SearchExpr(Cond);
        foundStmts |= TraverseRegion(Region, *CI, true,
                                     (isThreadDep | SearchExpr(Cond)));
        InnerRegions.push_back(*CI);
      }
      else {
        switch(CheckForUnary(Region, *CI)) {
        default:
          break;
        case CONTINUE:
          InnerContinues.push_back(cast<ContinueStmt>(*CI));
          foundStmts |= CONTINUE;
          break;
        case BREAK:
          InnerBreaks.push_back(cast<BreakStmt>(*CI));
          foundStmts |= BREAK;
          break;
        case RETURN:
          InnerReturns.push_back(cast<ReturnStmt>(*CI));
          foundStmts |= RETURN;
        }
      }
      FindThreadDeps(*CI, isThreadDep);
    }
  }
  else if (Region->child_begin() != Region->child_end()) {
    for (Stmt::child_iterator SI = Region->child_begin(),
         SE = Region->child_end(); SI != SE; ++SI) {

      if (isLoop(*SI) || isa<CompoundStmt>(*SI)) {
        foundStmts |= TraverseRegion(Region, *SI, isConditional, isThreadDep);
        InnerRegions.push_back(*SI);
      }
      else if (isa<DeclStmt>(*SI)) {
        DeclStmt *stmt = cast<DeclStmt>(*SI);
        InnerDeclStmts.push_back(stmt);
        DeclParents.insert(std::make_pair(stmt->getSingleDecl(), Region));
      }
      else if (isBarrier(*SI))
        InnerBarriers.push_back(cast<CallExpr>(*SI));
      else if (isa<IfStmt>(*SI)) {
        IfStmt *ifStmt = cast<IfStmt>(*SI);
        Expr *Cond = ifStmt->getCond();
        //isThreadDep |= SearchExpr(Cond);
        foundStmts |= TraverseRegion(Region, *SI, true,
                                     (isThreadDep | SearchExpr(Cond)));
        InnerRegions.push_back(*SI);
      }
      else {
        switch(CheckForUnary(Region, *SI)) {
        default:
          break;
        case CONTINUE:
          InnerContinues.push_back(cast<ContinueStmt>(*SI));
          foundStmts |= CONTINUE;
          break;
        case BREAK:
          InnerBreaks.push_back(cast<BreakStmt>(*SI));
          foundStmts |= BREAK;
          break;
        case RETURN:
          InnerReturns.push_back(cast<ReturnStmt>(*SI));
          foundStmts |= RETURN;
        }
      }
      FindThreadDeps(*SI, isThreadDep);
    }
  }

  // If and Else regions are visited from an IfStmt. If the IfStmt is within a
  // non-parallel region and contains a control-flow stmt, the IfStmt needs to
  // become a barrier and the bodies need to be put in a loop.
  // for ()                         for ()
  //   if ()                          thread_loop() { }
  //     break;                       if ()
  //   else                             thread_loop() { }
  //     something();                   break;
  //   barrier()                      else
  //                                    thread_loop() { something() }
  //                                  thread_loop() { }
  //                                  // barrier()
  // When we come to fix this, we can visit the IfStmt knowing that it lives in
  // a non-parallel region, we will see that the 'Then' contains a break but we
  // also need to then fix the else. We could do this lazily by:
  // - closing the loop at the beginning of the IfStmt
  // - open the loop at the beginning of the Then and the Else
  // - close the loop at the end of the Then and the Else
  // - close the loop at the beginning of the unary
  // - open the loop at the end of the unary
  // So we need to make any interesting stmts to the actual IfStmt and not the
  // conditional bodies, and when we come to fix it, we can just check if the
  // region is an IfStmt

  // Map statements to the region.
  Stmt *InsertRegion = (isa<IfStmt>(Parent)) ? Parent : Region;

  if (!InnerContinues.empty())
    ContinueStmts.insert(std::make_pair(InsertRegion, InnerContinues));
  if (!InnerBreaks.empty())
    BreakStmts.insert(std::make_pair(InsertRegion, InnerBreaks));
  if (!InnerReturns.empty())
    ReturnStmts.insert(std::make_pair(InsertRegion, InnerReturns));
  if (!InnerBarriers.empty())
    Barriers.insert(std::make_pair(InsertRegion, InnerBarriers));
  if (!InnerDeclStmts.empty())
    ScopedDeclStmts.insert(std::make_pair(InsertRegion, InnerDeclStmts));
  if (!InnerRegions.empty())
    NestedRegions.insert(std::make_pair(InsertRegion, InnerRegions));

  return foundStmts;
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
    TraverseRegion(For, For, false, false);
    OuterLoop = For;
  }

  isFirstLoop = false;
  return true;
}

// Create maps of all the references in the tree
bool WorkitemCoarsen::ThreadSerialiser::VisitDeclRefExpr(Expr *expr) {
  DeclRefExpr *RefExpr = cast<DeclRefExpr>(expr);
  std::string VarName = RefExpr->getDecl()->getName().str();

#ifdef DBG_WRKGRP
  //std::cerr << "VisitDeclRefExpr for " << VarName << std::endl;
  //expr->dumpAll();
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

inline void WorkitemCoarsen::ThreadSerialiser::AddDep(Decl *decl) {
  if (!isVarThreadDep(decl)) {
#ifdef DBG_WRKGRP
    std::cerr << "Adding " << cast<NamedDecl>(decl)->getName().str() <<
      " to thread deps " << std::endl;
#endif
    threadDepVars.insert(std::make_pair(decl, true));
  }
}

inline bool WorkitemCoarsen::ThreadSerialiser::isVarThreadDep(Decl *decl) {
#ifdef DBG_WRKGRP
  NamedDecl *ND = cast<NamedDecl>(decl);
  //std::string name = ND->getName().str();
  //std::cerr << "is " << name << " already added to thread deps?";
#endif
  if (threadDepVars.find(decl) != threadDepVars.end()) {
#ifdef DBG_WRKGRP
    //std::cerr << " - yes" << std::endl;
#endif
    return true;
  }
  else {
#ifdef DBG_WRKGRP
    //std::cerr << " - no" << std::endl;
#endif
    return false;
  }
}

bool WorkitemCoarsen::ThreadSerialiser::SearchExpr(Expr *s) {
#ifdef DBG_WRKGRP
  //std::cerr << "Search Expr" << std::endl;
#endif
  if (isa<DeclRefExpr>(s)) {
    DeclRefExpr *DRE = cast<DeclRefExpr>(s);
    if (isVarThreadDep(DRE->getDecl()))
      return true;

    NamedDecl *ND = cast<NamedDecl>(DRE->getDecl());
    std::string name = ND->getName().str();
    if ((name.compare("__esdg_idx") == 0) ||
      (name.compare("__esdg_idy") == 0) ||
      (name.compare("__esdg_idz") == 0)) {
      AddDep(ND);
#ifdef DBG_WRKGRP
      //std::cerr << "it is thread dependent" << std::endl;
#endif
      return true;
    }
  }
  else {
    for (Stmt::child_iterator CI = s->child_begin(), CE = s->child_end();
         CI != CE; ++CI) {
      if (SearchExpr(cast<Expr>(*CI)))
        return true;
    }
  }

  return false;
}

void WorkitemCoarsen::ThreadSerialiser::CheckUnaryOpDep(Expr *expr,
                                                        bool depRegion) {
#ifdef DBG_WRKGRP
  std::cerr << "Checking Unary for thread dep";
  if (depRegion)
    std::cerr << " in a ThreadDep Region";
  std::cerr << std::endl;
  //expr->dumpAll();
#endif
  UnaryOperator *UO = cast<UnaryOperator>(expr);
  for (Stmt::child_iterator CI = UO->child_begin(), CE = UO->child_end();
       CI != CE; ++CI) {

    //FindThreadDeps(*CI);

    if (isa<DeclRefExpr>(*CI)) {
      DeclRefExpr *ref = cast<DeclRefExpr>(*CI);
      if (depRegion)
        AddDep(ref->getDecl());
      if (SearchExpr(ref)) {
        AddDep(ref->getDecl());
      }
    }
    else
      FindThreadDeps(*CI, depRegion);
  }
}

void WorkitemCoarsen::ThreadSerialiser::CheckBinaryOpDep(Expr *expr,
                                                         bool depRegion) {
#ifdef DBG_WRKGRP
  std::cerr << "Entering CheckBinaryOpDep";
  if (depRegion)
    std::cerr << " in a ThreadDep Region";
  std::cerr << std::endl;
  //expr->dumpAll();
#endif
  BinaryOperator *BO = cast<BinaryOperator>(expr);
  Expr *LHS = BO->getLHS();
  Expr *RHS = BO->getRHS();
  if (BinaryOperator::isAssignmentOp(BO->getOpcode())) {
    if (isa<DeclRefExpr>(LHS)) {
      DeclRefExpr *ref = cast<DeclRefExpr>(LHS);
      if (depRegion)
        AddDep(ref->getDecl());
      if (SearchExpr(RHS)) {
        AddDep(ref->getDecl());
      }
    }
    else if (isa<ExtVectorElementExpr>(LHS)) {
      ExtVectorElementExpr *vExpr = cast<ExtVectorElementExpr>(LHS);
      Expr *base = vExpr->getBase();
      if (isa<DeclRefExpr>(base)) {
        DeclRefExpr *ref = cast<DeclRefExpr>(base);
        if (depRegion)
          AddDep(ref->getDecl());
        if (SearchExpr(RHS))
          AddDep(ref->getDecl());
      }
    }
    else
      FindThreadDeps(LHS, depRegion);
  }
#ifdef DBG_WRKGRP
  //std::cerr << "Leaving CheckBinaryOpDep" << std::endl;
#endif
}

inline void WorkitemCoarsen::ThreadSerialiser::CheckArrayDeps(Expr *expr,
                                                              bool depRegion) {
#ifdef DBG_WRKGRP
  std::cerr << "Checking array for deps";
  if (depRegion)
    std::cerr << " in a ThreadDep Region";
  std::cerr << std::endl;
  //expr->dumpAll();
#endif
  ArraySubscriptExpr *ASE = cast<ArraySubscriptExpr>(expr);
#ifdef DBG_WRKGRP
  //std::cerr << "Base = " << std::endl;
  //ASE->getBase()->dumpAll();
  //std::cerr << "Index = " << std::endl;
  //ASE->getIdx()->dumpAll();
#endif
  CastExpr *CE = cast<CastExpr>(ASE->getBase());
  DeclRefExpr *DRE = cast<DeclRefExpr>(CE->getSubExpr());
  if (depRegion)
    AddDep(DRE->getDecl());

  if (SearchExpr(ASE->getIdx()))
    AddDep(DRE->getDecl());
}

void WorkitemCoarsen::ThreadSerialiser::CheckDeclStmtDep(Stmt *s,
                                                         bool depRegion) {
  DeclStmt *DS = cast<DeclStmt>(s);
  NamedDecl *ND = cast<NamedDecl>(DS->getSingleDecl());
  std::string name = ND->getName().str();
#ifdef DBG_WRKGRP
  std::cerr << "CheckDeclStmtDep for " << name;
  if (depRegion)
    std::cerr << " in a ThreadDep Region";
  std::cerr << std::endl;
  //s->dumpAll();
#endif

  if (depRegion)
    AddDep(ND);

  if (s->child_begin() != s->child_end()) {
    if (isa<Expr>(*(s->child_begin()))) {
      if (SearchExpr(cast<Expr>(*(s->child_begin())))) {
#ifdef DBG_WRKGRP
        //std::cerr << "Adding " << name << " to threadDepVars" << std::endl;
#endif
        AddDep(ND);
      }
    }
  }
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

