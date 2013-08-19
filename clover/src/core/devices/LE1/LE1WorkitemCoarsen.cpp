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

typedef std::vector<Stmt*> StmtSet;
typedef std::vector<DeclRefExpr*> DeclRefSet;
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;

bool WorkitemCoarsen::CreateWorkgroup(std::string &Filename, std::string
                                      &kernel) {
#ifdef DEBUGCL
  std::cerr << "CreateWorkgroup\n";
#endif
  OrigFilename = Filename;
  KernelName = kernel;

  // Firstly, we need to expand the macros within the source code because
  // it is not possible to rewrite their locations.
  std::string SourceContainer;
  llvm::raw_string_ostream ExpandedSource(SourceContainer);
  if(!ExpandMacros()) {
    std::cerr << "!ERROR : Failed to expand macros" << std::endl;
    return false;
  }

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
#ifdef DEBUGCL
  std::cerr << "Finished initialising workgroup\n";
#endif

  std::ofstream init_kernel;
  InitKernelFilename = std::string("init.");
  InitKernelFilename.append(OrigFilename);
  init_kernel.open(InitKernelFilename.c_str());
  init_kernel << InitKernelSource;
  init_kernel.close();
  return HandleBarriers();
}

// Use a RewriteAction to expand all macros within the original source file
bool WorkitemCoarsen::ExpandMacros() {
  std::string log;
  llvm::raw_string_ostream macro_log(log);
  CompilerInstance CI;
  RewriteMacrosAction act;

  CI.getTargetOpts().Triple = "le1";
  CI.getFrontendOpts().Inputs.push_back(FrontendInputFile(OrigFilename,
                                                          IK_OpenCL));
  // Overwrite original
  CI.getFrontendOpts().OutputFile = OrigFilename;

  CI.getHeaderSearchOpts().AddPath(LIBCLC_INCLUDE_DIR,
                                   frontend::Angled,
                                   false, false, false);
  CI.getHeaderSearchOpts().AddPath(CLANG_INCLUDE_DIR, frontend::Angled,
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
  return true;
}

bool WorkitemCoarsen::HandleBarriers() {
  OpenCLCompiler<ThreadSerialiser> SerialCompiler(LocalX, LocalY, LocalZ,
                                                  KernelName);
  SerialCompiler.setFile(InitKernelFilename);
  SerialCompiler.Parse();
  if (SerialCompiler.needsScalarFixes())
    SerialCompiler.FixAllScalarAccesses();

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
  std::cerr << "Finalised kernel:\n";
#endif

  return true;
}

template <typename T>
WorkitemCoarsen::ASTVisitorBase<T>::ASTVisitorBase(Rewriter &R,
                                                   unsigned x,
                                                   unsigned y,
                                                   unsigned z)
    : LocalX(x), LocalY(y), LocalZ(z), TheRewriter(R) {

  if (LocalZ > 1) {
    OpenWhile  << "while (__kernel_local_id[2] < " << LocalZ << ") {\n";
  }
  if (LocalY > 1) {
    OpenWhile << "while (__kernel_local_id[1] < " << LocalY << ") {\n";
  }
  if (LocalX > 1) {
    OpenWhile << "while (__kernel_local_id[0] < " << LocalX << ") {\n";
  }

  if (LocalX > 1) {
    CloseWhile << "\n__kernel_local_id[0]++;\n";
    CloseWhile  << "}\n";
    CloseWhile << "__kernel_local_id[0] = 0;\n";
  }
  if (LocalY > 1) {
    CloseWhile << " __kernel_local_id[1]++;\n";
    CloseWhile << "}\n";
    CloseWhile << "__kernel_local_id[1] = 0;\n";
  }
  if (LocalZ > 1) {
    CloseWhile << "__kernel_local_id[2]++;\n";
    CloseWhile << "}\n";
    CloseWhile << "\n__kernel_local_id[2] = 0;\n";
  }

}

template <typename T>
//void OpenCLCompiler<T>::KernelInitialiser::CloseLoop(SourceLocation Loc) {
void WorkitemCoarsen::ASTVisitorBase<T>::CloseLoop(SourceLocation Loc) {
  TheRewriter.InsertText(Loc, CloseWhile.str(), true, true);
}

template <typename T>
void WorkitemCoarsen::ASTVisitorBase<T>::OpenLoop(SourceLocation Loc) {
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
    FuncBegin << "] = {0};\n";

    TheRewriter.InsertText(FuncBodyStart, FuncBegin.str(), true, true);
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
#ifdef DEBUGCL
      std::cerr << ND->getName().str() << " is declared\n";
#endif

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
        << " + __kernel_local_id[0];//";
      break;
    case 1:
      GlobalId << "get_group_id(1) * " << LocalY
        << " + __kernel_local_id[1];//";
      break;
    case 2:
      GlobalId << "get_group_id(2) * " << LocalZ
        << " + __kernel_local_id[2];//";
      break;
    }
    TheRewriter.InsertText(Call->getLocStart(), GlobalId.str());
  }
  else if (FuncName.compare("get_local_id") == 0) {
    TheRewriter.RemoveText(Call->getSourceRange());
    if (Index == 0)
      TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(1),
                             "__kernel_local_id[0]");
    else if (Index == 1)
      TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(1),
                             "__kernel_local_id[1]");
    else
      TheRewriter.InsertText(Call->getLocEnd().getLocWithOffset(1),
                             "__kernel_local_id[2]");

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

    TheRewriter.InsertText(Call->getLocStart(), local.str());

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

void WorkitemCoarsen::ThreadSerialiser::FixAllScalarAccesses() {
#ifdef DEBUGCL
  std::cerr << "FixAllScalarAccesses" << std::endl;
#endif
  // Search nested loops will always return true for the outer loop
  // as there is definitely a barrier somewhere, and it is accessible
  // from the outer loop
  SearchNestedLoops(OuterLoop, true);

  // Then we see which variables are only assigned in loop headers, these
  // are then disabled for expansion, though they can still be made local.
  AssignIndVars();

  // Then we need to find all the variables that we need to replicate.
  std::list<DeclStmt*> Stmts;
  FindRefsToExpand(Stmts, OuterLoop);

}

void WorkitemCoarsen::ThreadSerialiser::AssignIndVars() {
#ifdef DEBUGCL
  std::cerr << "AssignIndVars" << std::endl;
#endif

  for (std::map<Decl*, std::list<DeclRefExpr*> >::iterator MI
       = PotentialIndVars.begin(), ME = PotentialIndVars.end();
       MI != ME; ++MI) {

    Decl *decl = MI->first;
    std::list<DeclRefExpr*> IndVarRefs = MI->second;
    std::list<DeclRefExpr*> RefAssigns = RefAssignments[decl];

#ifdef DEBUGCL
    std::cerr << "Var: " << cast<NamedDecl>(decl)->getName().str()
      << ". IndVarRefs.size = " << IndVarRefs.size() << " and RefAssigns = "
      << RefAssigns.size() << std::endl;
#endif

    if (IndVarRefs.size() == RefAssigns.size()) {
#ifdef DEBUGCL
      std::cerr << "Found induction variable" << std::endl;
#endif
      IndVars.push_back(decl);
    }
  }
}

void WorkitemCoarsen::ThreadSerialiser::FindRefsToExpand(
  std::list<DeclStmt*> &Stmts, Stmt *Loop) {

#ifdef DEBUGCL
  std::cerr << "FindRefsToExpand" << std::endl;
#endif

  // Again, quit early if there's gonna be nothing to find.
  if ((NestedLoops[Loop].empty()) && (Barriers[Loop].empty()))
    return;

  // Get the DeclStmts of this loop, append it to the vector that has been
  // passed; so the search space grows as we go deeper into the loops.
  std::list<DeclStmt*> InnerDeclStmts = ScopedDeclStmts[Loop];

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
#ifdef DEBUGCL
    std::cerr << "Erasing elements" << std::endl;
#endif
    // Erase this loop's DeclStmts from the larger set since we search them
    // differently.
    Stmts.erase(OrigEnd, Stmts.end());
  }

  // Start looking for references within this loop:
#ifdef DEBUGCL
  std::cerr << "Look for statements within this loop" << std::endl;
#endif

  // First we can check the DeclStmts within this Loop, and compare the ref
  // locations to that of the barrier(s) also within this loop body.
  std::vector<CallExpr*> InnerBarriers = Barriers[Loop];

  for (std::list<DeclStmt*>::iterator DSI = InnerDeclStmts.begin(),
       DSE = InnerDeclStmts.end(); DSI != DSE; ++DSI) {

    std::vector<DeclRefExpr*> Refs = AllRefs[(*DSI)->getSingleDecl()];
    SourceLocation DeclLoc = (*DSI)->getLocStart();
    bool hasExpanded = false;

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
          ScalarExpand(Loop->getLocStart(), *DSI);
          hasExpanded= true;
          break;
        }
      }
    }
  }

#ifdef DEBUGCL
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
  else {
    LoopStart = (cast<ForStmt>(Loop))->getBody()->getLocStart();
    LoopEnd = (cast<ForStmt>(Loop))->getBody()->getLocEnd();
  }

  for (std::list<DeclStmt*>::iterator DSI = Stmts.begin(), DSE = Stmts.end();
       DSI != DSE;) {

    bool hasExpanded = false;
#ifdef DEBUGCL
    std::cerr << "Checking var: "
      << cast<NamedDecl>((*DSI)->getSingleDecl())->getName().str() << std::endl;
#endif

    std::vector<DeclRefExpr*> Refs = AllRefs[(*DSI)->getSingleDecl()];

    for (std::vector<DeclRefExpr*>::iterator RI = Refs.begin(), RE = Refs.end();
         RI != RE; ++RI) {

      SourceLocation RefLoc = (*RI)->getLocStart();
      if ((LoopStart < RefLoc) && (RefLoc < LoopEnd)) {
        SourceLocation InsertLoc = DeclParents[(*RI)->getDecl()]->getLocStart();
        ScalarExpand(InsertLoc, *DSI);
        DSI = Stmts.erase(DSI);
        hasExpanded = true;
        break;
      }
    }
    if (!hasExpanded)
      ++DSI;
  }
#ifdef DEBUGCL
  std::cerr << "Exit FindRefsToExpand" << std::endl;
#endif

}

// Run this after parsing is complete, to access all the referenced variables
// within the DeclStmt which need to be replicated
void
WorkitemCoarsen::ThreadSerialiser::ScalarExpand(SourceLocation InsertLoc,
                                                DeclStmt *theDecl) {
  bool isIndVar = false;
  for (std::vector<Decl*>::iterator DI = IndVars.begin(), DE = IndVars.end();
       DI != DE; ++DI) {
    if (*DI == theDecl->getSingleDecl()) {
      isIndVar = true;
      break;
    }
  }
  if (isIndVar) {
    CreateLocal(InsertLoc, theDecl);
    return;
  }
#ifdef DEBUGCL
  std::cerr << "ScalarExpand: " << std::endl;
#endif
  NamedDecl *ND = cast<NamedDecl>(theDecl->getSingleDecl());
  std::string varName = ND->getName().str();
  std::stringstream NewDecl;

  ValueDecl *VD = cast<ValueDecl>(ND);
  clang::QualType QualVarType = VD->getType();
  const clang::Type *VarType = QualVarType.getTypePtr();
  std::string typeStr;

  if (VarType->isArrayType()) {
    ConstantArrayType *arrayType =
      cast<ConstantArrayType>(const_cast<Type*>(VarType));
    uint64_t arraySize = arrayType->getSize().getZExtValue();
    typeStr = arrayType->getElementType().getAsString();
    NewDecl << typeStr <<  " " << varName << "[" << arraySize << "]";
  }
  else {
    typeStr = QualVarType.getAsString();
    NewDecl << typeStr << " " << varName;
  }

  if (LocalX != 0)
    NewDecl << "[" << LocalX << "]";
  if (LocalY > 1)
    NewDecl << "[" << LocalY << "]";
  if (LocalZ > 1)
    NewDecl << "[" << LocalZ << "]";

  NewDecl << ";\n";
  TheRewriter.InsertText(InsertLoc, NewDecl.str(), true, true);

  // If the statement wasn't initialised, remove the whole statement
  if (theDecl->child_begin() == theDecl->child_end())
    TheRewriter.RemoveText(theDecl->getSourceRange());
  else {
    // Otherwise, just remove the type definition and make the initialisation
    // access a scalar.
    TheRewriter.RemoveText(ND->getLocStart(), typeStr.length());
    AccessScalar(theDecl->getSingleDecl());
  }

  std::vector<DeclRefExpr*> theRefs = AllRefs[theDecl->getSingleDecl()];
  // Then visit all the references to turn them into scalar accesses as well.
  for (std::vector<DeclRefExpr*>::iterator RI = theRefs.begin(),
       RE = theRefs.end(); RI != RE; ++RI)
    AccessScalar(*RI);
}

void WorkitemCoarsen::ThreadSerialiser::CreateLocal(SourceLocation InsertLoc,
                                                    DeclStmt *s) {
  NamedDecl *ND = cast<NamedDecl>(s->getSingleDecl());
  std::string varName = ND->getName().str();
  std::string type = cast<ValueDecl>(ND)->getType().getAsString();
  std::stringstream NewDecl;
  NewDecl << type << " " << varName << ";\n";
  TheRewriter.InsertText(InsertLoc, NewDecl.str(), true, true);

  // If the statement wasn't initialised, remove the whole statement
  if (s->child_begin() == s->child_end())
    TheRewriter.RemoveText(s->getSourceRange());
  else {
    // Otherwise, just remove the type definition and make the initialisation
    // access a scalar.
    TheRewriter.RemoveText(ND->getLocStart(), type.length());
  }
}

// FIXME Scalar access only works for x dimension values!
void WorkitemCoarsen::ThreadSerialiser::AccessScalar(Decl *decl) {
  NamedDecl *ND = cast<NamedDecl>(decl);
  unsigned offset = ND->getName().str().length();
  offset += cast<ValueDecl>(ND)->getType().getAsString().length();
  // increment because of a space between type and name
  ++offset;
  SourceLocation Loc = ND->getLocStart().getLocWithOffset(offset);
  if(TheRewriter.InsertText(Loc, "[__kernel_local_id[0]]", true))
    std::cerr << "ERROR - location not writable!\n";
}

// FIXME Scalar access only works for x dimension values!
void WorkitemCoarsen::ThreadSerialiser::AccessScalar(DeclRefExpr *Ref) {
  unsigned offset = Ref->getDecl()->getName().str().length();
  SourceLocation Loc = Ref->getLocEnd().getLocWithOffset(offset);
  if(TheRewriter.InsertText(Loc, "[__kernel_local_id[0]]", true))
    std::cerr << "ERROR - location not writable! Offset = "
      << offset << std::endl;
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
bool WorkitemCoarsen::ThreadSerialiser::SearchNestedLoops(Stmt *Loop,
                                                          bool isOuterLoop) {
#ifdef DEBUGCL
  std::cerr << "SearchNestedLoops" << std::endl;
#endif

  if ((NestedLoops[Loop].empty()) && (Barriers[Loop].empty())) {
#ifdef DEBUGCL
    std::cerr << "No nested loops and there's no barriers in this one"
      << std::endl;
#endif
    return false;
  }

  unsigned NestedBarriers = 0;
  std::vector<Stmt*> InnerLoops = NestedLoops[Loop];
  for (std::vector<Stmt*>::iterator LoopI
       = InnerLoops.begin(), LoopE = InnerLoops.end(); LoopI != LoopE;
       ++LoopI) {
    if (SearchNestedLoops(*LoopI, false))
      ++NestedBarriers;
  }
  if ((!Barriers[Loop].empty()) || (NestedBarriers != 0)) {

    // Appropriately open and close the workgroup loops if its an original loop
    if (!isOuterLoop)
      HandleBarrierInLoop(Loop);
    return true;
  }
  else
    return false;
}


// Whether a loop contains a barrier, nested or not, we follow these steps:
// - close the main loop before this loop starts
// - open a main loop at the start of the body of the loop
// - close the main loop at the end of the body of the loop
// - open the main loop when the for loop exits
void WorkitemCoarsen::ThreadSerialiser::HandleBarrierInLoop(Stmt *Loop) {
#ifdef DEBUGCL
  std::cerr << "HandleBarrierInLoop\n";
#endif
  // Record that this loop contains a barrier
  //LoopsWithBarrier.push_back(Loop);

  Stmt *LoopBody = NULL;
  if (isa<ForStmt>(Loop))
    LoopBody = (cast<ForStmt>(Loop))->getBody();
  else if (isa<WhileStmt>(Loop))
    LoopBody = (cast<WhileStmt>(Loop))->getBody();
  else
    return;

  CloseLoop(Loop->getLocStart());
  OpenLoop(GetOffsetInto(LoopBody->getLocStart()));
  CloseLoop(LoopBody->getLocEnd());
  OpenLoop(GetOffsetOut(Loop->getLocEnd()));
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

void WorkitemCoarsen::ThreadSerialiser::SearchForIndVars(Stmt *s) {
#ifdef DEBUGCL
  std::cerr << "SearchForIndVars" << std::endl;
#endif

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

// We shall create a map, using the outer loop as the key, to contain all it's
// loop children. We shall also then have a map of all the DeclStmts within the
// loop, plus a map of vectors which will hold barriers.
void WorkitemCoarsen::ThreadSerialiser::TraverseLoop(Stmt *s) {
#ifdef DEBUGCL
  std::cerr << "TraverseLoop" << std::endl;
#endif
  Stmt *Body = NULL;
  // create vector to hold loop stmts
  std::vector<Stmt*> InnerLoops;
  // create a vector to hold decl stmts
  std::list<DeclStmt*> InnerDeclStmts;
  // create a vector to hold barriers
  std::vector<CallExpr*> InnerBarriers;

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

    if ((isa<WhileStmt>(*SI)) || (isa<ForStmt>(*SI))) {
      InnerLoops.push_back(*SI);
      // Recursively visit the loops from the parent
      TraverseLoop(*SI);
    }
    else if (isa<DeclStmt>(*SI)) {
      DeclStmt *stmt = cast<DeclStmt>(*SI);
      InnerDeclStmts.push_back(stmt);
      DeclParents.insert(std::make_pair(stmt->getSingleDecl(), s));
    }
    else if (isBarrier(*SI))
      InnerBarriers.push_back(cast<CallExpr>(*SI));

  }

  // add vectors to the maps, using 's' as the key
  if (!InnerLoops.empty())
    NestedLoops.insert(std::make_pair(s, InnerLoops));
  if (!InnerDeclStmts.empty())
    ScopedDeclStmts.insert(std::make_pair(s, InnerDeclStmts));
  if (!InnerBarriers.empty())
    Barriers.insert(std::make_pair(s, InnerBarriers));
}

// Use this only as an entry into the kernel
bool WorkitemCoarsen::ThreadSerialiser::VisitWhileStmt(Stmt *s) {
  static bool isFirstLoop = true;
#ifdef DEBUGCL
  std::cerr << "VisitWhileStmt" << std::endl;
#endif
  WhileStmt *While = cast<WhileStmt>(s);
#ifdef DEBUGCL
  std::cerr << "Got while" << std::endl;
#endif

  if (isFirstLoop) {
    TraverseLoop(While);
    OuterLoop = While;
  }

  isFirstLoop = false;
  return true;

  Expr *Cond = While->getCond();
  if (Cond == NULL) {
#ifdef DEBUGCL
    std::cerr << "but Cond is NULL" << std::endl;
#endif
    return true;
  }

  for (Stmt::child_iterator CI = Cond->child_begin(), CE = Cond->child_end();
       CI != CE; ++CI) {
    if (isa<BinaryOperator>(*CI)) {
#ifdef DEBUGCL
      std::cerr << "Found BinaryOperator child of while" << std::endl;
#endif
      for (Stmt::child_iterator BI = (*CI)->child_begin(),
           BE = (*CI)->child_end(); BI != BE; ++BI) {
        if (isa<DeclRefExpr>(*BI)) {
          NamedDecl *ND = cast<NamedDecl>((cast<DeclRefExpr>(*BI))->getDecl());
          std::string VarName = ND->getName().str();
#ifdef DEBUGCL
          std::cerr << "While conditional = " << VarName << std::endl;
#endif
          if (VarName.compare("__kernel_local_id") == 0) {
            TraverseLoop(While);
            OuterLoop = While;
            break;
          }
        }
      }
    }
  }

  return true;
}

// Remove barrier calls and modify calls to kernel builtin functions.
bool WorkitemCoarsen::ThreadSerialiser::VisitCallExpr(Expr *s) {
#ifdef DEBUGCL
  std::cerr << "VisitCallExpr" << std::endl;
#endif
  if (isBarrier(s)) {
    CallExpr *Call = cast<CallExpr>(s);
    TheRewriter.InsertText(Call->getLocStart(), "//", true, true);
    CloseLoop(Call->getLocEnd().getLocWithOffset(2));
    OpenLoop(Call->getLocEnd().getLocWithOffset(2));
  }
  return true;
}

bool WorkitemCoarsen::ThreadSerialiser::VisitReturnStmt(Stmt *s) {
  TheRewriter.InsertTextAfter(s->getLocEnd(), OpenWhile.str());
  TheRewriter.InsertTextBefore(s->getLocStart(), CloseWhile.str());
  return true;
}

// TODO Need to handle continues and breaks
bool WorkitemCoarsen::ThreadSerialiser::WalkUpFromUnaryContinueStmt(
  UnaryOperator *S) {
  return true;
}

// Create maps of all the references in the tree
bool WorkitemCoarsen::ThreadSerialiser::VisitDeclRefExpr(Expr *expr) {
  DeclRefExpr *RefExpr = cast<DeclRefExpr>(expr);
  std::string VarName = RefExpr->getDecl()->getName().str();

  // Don't add it if its one of an work-item indexes
  if (VarName.compare("__kernel_local_id") == 0)
    return true;

  // Don't add it if it is a function parameter
  for (std::vector<std::string>::iterator PI = ParamVars.begin(),
       PE = ParamVars.end(); PI != PE; ++PI) {
    if ((*PI).compare(VarName) == 0)
      return true;
  }

  Decl *key = RefExpr->getDecl();
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
void WorkitemCoarsen::OpenCLCompiler<T>::expandMacros(llvm::raw_ostream *source)
{
  clang::RewriteMacrosInInput(TheCompInst.getPreprocessor(), source);
}

template <typename T>
void WorkitemCoarsen::OpenCLCompiler<T>::Parse() {
  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), TheConsumer,
           TheCompInst.getASTContext());
}

