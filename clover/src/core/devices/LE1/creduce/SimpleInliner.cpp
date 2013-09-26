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

#include "SimpleInliner.h"

#include <iostream>
#include <sstream>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

#include "TransformationManager.h"
#include "CommonStatementVisitor.h"

using namespace clang;
using namespace llvm;

static const char *DescriptionMsg =
"A really simple inliner. \
This transformation does a simple source-to-source \
inlining. To avoid the abuse of inlining, I put \
some constraints on the size of a function which \
can be inlined - if a function has less than 10 statements, \
then it's legitimate. \n\
\n\
Steps of inlining: \n\
  * create a tmp var for function return value; \n\
  * create a new block which is a copy of the inlined function; \n\
  * at the top of this newly block, inlined function's parameters \
will be declared as local vars with callexpr's arguments as their \
initialization values (if any) \n\
  * inside this newly block, replace all return statements as \
assignment statements, where the LHS is the created tmp var \
(Note that if the inlined function returns void, then \
this step is skipped) \n\
  * replace the callexpr with tmp var above \n\
\n\
Each transformation iteration only transforms one callexpr, \
also it will keep the inlined function body unchanged. \
If the inlined body has no reference anymore, c_delta \
will remove it entirely. \n";

//static RegisterTransformation<SimpleInliner>
  //       Trans("simple-inliner", DescriptionMsg);

class SimpleInlinerCollectionVisitor : public 
  RecursiveASTVisitor<SimpleInlinerCollectionVisitor> {

public:

  explicit SimpleInlinerCollectionVisitor(SimpleInliner *Instance)
    : ConsumerInstance(Instance),
      NumStmts(0)
  { }

  bool VisitStmt(Stmt *S);

  bool VisitCallExpr(CallExpr *CE);

  bool TraverseConstructorInitializer(CXXCtorInitializer *Init);

  unsigned int getNumStmts(void) {
    return NumStmts;
  }

  void setNumStmts(unsigned int Num) {
    NumStmts = Num;
  }

private:

  SimpleInliner *ConsumerInstance;

  unsigned int NumStmts;
};

class SimpleInlinerFunctionVisitor : public 
  RecursiveASTVisitor<SimpleInlinerFunctionVisitor> {

public:

  explicit SimpleInlinerFunctionVisitor(SimpleInliner *Instance)
    : ConsumerInstance(Instance)
  { }

  bool VisitReturnStmt(ReturnStmt *RS);

  bool VisitDeclRefExpr(DeclRefExpr *DRE);

private:

  SimpleInliner *ConsumerInstance;

};

class SimpleInlinerFunctionStmtVisitor : public 
        RecursiveASTVisitor<SimpleInlinerFunctionStmtVisitor> {
public:

  explicit SimpleInlinerFunctionStmtVisitor(SimpleInliner *Instance)
    : ConsumerInstance(Instance)
  { }

  bool VisitFunctionDecl(FunctionDecl *FD);

private:
  SimpleInliner *ConsumerInstance;
  
};

class SimpleInlinerStmtVisitor : public 
  CommonStatementVisitor<SimpleInlinerStmtVisitor> {

public:

  explicit SimpleInlinerStmtVisitor(SimpleInliner *Instance)
    : ConsumerInstance(Instance)
  { }

  bool VisitCallExpr(CallExpr *CallE);

private:

  SimpleInliner *ConsumerInstance;
};

bool SimpleInlinerCollectionVisitor::VisitStmt(Stmt *S)
{
  Stmt::StmtClass SC = S->getStmtClass();
  switch (SC) {
  case Stmt::BreakStmtClass:
  case Stmt::CompoundStmtClass:
  case Stmt::ContinueStmtClass:
  case Stmt::DeclStmtClass:
  case Stmt::DoStmtClass:
  case Stmt::ForStmtClass:
  case Stmt::GotoStmtClass:
  case Stmt::IndirectGotoStmtClass:
  case Stmt::IfStmtClass:
  case Stmt::ReturnStmtClass:
  case Stmt::CaseStmtClass:
  case Stmt::SwitchStmtClass:
  case Stmt::WhileStmtClass:
  case Stmt::BinaryOperatorClass:
    NumStmts++;
    break;
  default:
    break;
  }
  return true;
}

bool SimpleInlinerCollectionVisitor::VisitCallExpr(CallExpr *CE)
{
  FunctionDecl *FD = CE->getDirectCallee();
  if (!FD)
    return true;

  ConsumerInstance->AllCallExprs.push_back(CE);
  ConsumerInstance->CalleeToCallerMap[CE] = ConsumerInstance->CurrentFD;

  FunctionDecl *CanonicalFD = FD->getCanonicalDecl();
  unsigned int NumCalls = ConsumerInstance->FunctionDeclNumCalls[CanonicalFD];
  NumCalls++;
  ConsumerInstance->FunctionDeclNumCalls[CanonicalFD] = NumCalls;

  NumStmts++;
  return true;
}

// Overload the default traverse function, because we cannot inline 
// Ctor's initializer
bool SimpleInlinerCollectionVisitor::TraverseConstructorInitializer(
       CXXCtorInitializer *Init) 
{
  return true;
}

bool SimpleInlinerFunctionVisitor::VisitReturnStmt(ReturnStmt *RS)
{
  ConsumerInstance->ReturnStmts.push_back(RS);
  return true;
}

bool SimpleInlinerFunctionVisitor::VisitDeclRefExpr(DeclRefExpr *DRE)
{
  const ValueDecl *OrigDecl = DRE->getDecl();
  const ParmVarDecl *PD = dyn_cast<ParmVarDecl>(OrigDecl);
  if (PD)
     ConsumerInstance->ParmRefs.push_back(DRE); 
  return true;
}

bool SimpleInlinerFunctionStmtVisitor::VisitFunctionDecl(FunctionDecl *FD)
{
  if (!FD->isThisDeclarationADefinition())
    return true;

  ConsumerInstance->CurrentFD = FD;
  ConsumerInstance->CollectionVisitor->setNumStmts(0);
  ConsumerInstance->CollectionVisitor->TraverseDecl(FD);

  if (!FD->isVariadic()) {
#ifdef DEBUGCL
    std::cerr << "!FD->isVariadic" << std::endl;
#endif
    ConsumerInstance->FunctionDeclNumStmts[FD->getCanonicalDecl()] = 
      ConsumerInstance->CollectionVisitor->getNumStmts();
  }
  return true;
}

bool SimpleInlinerStmtVisitor::VisitCallExpr(CallExpr *CallE) 
{
  if (ConsumerInstance->TheCallExpr == CallE) {
    ConsumerInstance->TheStmt = CurrentStmt;
    ConsumerInstance->NeedParen = NeedParen;
    // Stop recursion
    return false;
  }
  return true;
}

void SimpleInliner::Initialize(ASTContext &context) 
{
  Transformation::Initialize(context);
  NameQueryWrap = 
    new TransNameQueryWrap(RewriteHelper->getTmpVarNamePrefix());
  CollectionVisitor = new SimpleInlinerCollectionVisitor(this);
  FunctionVisitor = new SimpleInlinerFunctionVisitor(this);
  FunctionStmtVisitor = new SimpleInlinerFunctionStmtVisitor(this);
  StmtVisitor = new SimpleInlinerStmtVisitor(this);
}

bool SimpleInliner::HandleTopLevelDecl(DeclGroupRef D) 
{
  for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    FunctionStmtVisitor->TraverseDecl(*I);
  }
  return true;
}

void SimpleInliner::HandleTranslationUnit(ASTContext &Ctx)
{
#ifdef DEBUGCL
  std::cerr << "SimpleInliner::HandleTranslationUnit" << std::endl;
#endif
  doAnalysis();
  if (QueryInstanceOnly) {
#ifdef DEBUGCL
    std::cerr << "QueryInstanceOnly" << std::endl;
#endif
    return;
  }

  if (TransformationCounter > ValidInstanceNum) {
#ifdef DEBUGCL
    std::cerr << "TransformationCounter > ValidInstanceNum" << std::endl;
#endif
    TransError = TransMaxInstanceError;
    return;
  }

#ifdef DEBUGCL
  if (!CurrentFD)
    std::cerr << "NULL CurrentFD" << std::endl;
  if (!TheCallExpr)
    std::cerr << "NULL TheCallExpr" << std::endl;
#endif
  TransAssert(CurrentFD && "NULL CurrentFD!");
  TransAssert(TheCallExpr && "NULL TheCallExpr!");

  Ctx.getDiagnostics().setSuppressAllDiagnostics(false);

  NameQueryWrap->TraverseDecl(Ctx.getTranslationUnitDecl());
  NamePostfix = NameQueryWrap->getMaxNamePostfix() + 1;

  FunctionVisitor->TraverseDecl(CurrentFD);
  StmtVisitor->TraverseDecl(TheCaller);

  TransAssert(TheStmt && "NULL TheStmt!");
  replaceCallExpr();

  if (Ctx.getDiagnostics().hasErrorOccurred() ||
      Ctx.getDiagnostics().hasFatalErrorOccurred())
    TransError = TransInternalError;

#ifdef DEBUGCL
  std::cerr << "Leaving HandleTranslationUnit" << std::endl;
#endif
}

bool SimpleInliner::isValidArgExpr(const Expr *E)
{
#ifdef DEBUGCL
  std::cerr << "isValidArgExpr??" << std::endl;
  E->dumpAll();
#endif
  TransAssert(E && "NULL Expr!");
  switch(E->getStmtClass()) {
  case Expr::FloatingLiteralClass:
  case Expr::StringLiteralClass:
  case Expr::IntegerLiteralClass:
  case Expr::GNUNullExprClass:
  case Expr::CharacterLiteralClass: // Fall-through
    return true;
  
  case Expr::ParenExprClass:
    return isValidArgExpr(cast<ParenExpr>(E)->getSubExpr());

  case Expr::ImplicitCastExprClass:
  case Expr::CStyleCastExprClass: // Fall-through
    return isValidArgExpr(cast<CastExpr>(E)->getSubExpr());

  case Expr::MemberExprClass:
    return true;

  case Expr::ArraySubscriptExprClass: {
    const ArraySubscriptExpr *AE = cast<ArraySubscriptExpr>(E);
    return isValidArgExpr(AE->getIdx());
  }

  case Expr::DeclRefExprClass:
    return true;

  default:
    return false;
  }
  TransAssert(0 && "Unreachable code!");
  return false;
}

bool SimpleInliner::hasValidArgExprs(const CallExpr *CE)
{
  for(CallExpr::const_arg_iterator I = CE->arg_begin(), E = CE->arg_end();
      I != E; ++I) {
    if (!isValidArgExpr(*I))
      return false;
  }
  return true;
}

void SimpleInliner::getValidFunctionDecls(void)
{
#ifdef DEBUGCL
  std::cerr << "SimpleInliner::getValidFunctionDecls" << std::endl;
  std::cerr << "FunctionDeclNumStmts.size = " << FunctionDeclNumStmts.size()
    << std::endl;
#endif
  for (FunctionDeclToNumStmtsMap::iterator I = FunctionDeclNumStmts.begin(),
       E = FunctionDeclNumStmts.end(); I != E; ++I) {
    FunctionDecl *FD = (*I).first;
#ifdef DEBUGCL
    std::cerr << FD->getName().str() << std::endl;
#endif
    unsigned int NumStmts = (*I).second;
    unsigned int NumCalls = FunctionDeclNumCalls[FD];

    //if (((NumCalls == 1) && (NumStmts <= SingleMaxNumStmts)) ||
      //  ((NumCalls > 1) && (NumStmts <= MaxNumStmts))) {
      ValidFunctionDecls.insert(FD);
    //}
  }
}

void SimpleInliner::doAnalysis(void)
{
#ifdef DEBUGCL
  std::cerr << "SimpleInliner::doAnalysis" << std::endl;
  std::cerr << "Size of AllCallExprs = " << AllCallExprs.size() << std::endl;
#endif
  getValidFunctionDecls();

  for (SmallVector<CallExpr *, 10>::iterator CI = AllCallExprs.begin(),
       CE = AllCallExprs.end(); CI != CE; ++CI) {

    FunctionDecl *CalleeDecl = (*CI)->getDirectCallee(); 
    TransAssert(CalleeDecl && "Bad CalleeDecl!");
    FunctionDecl *CanonicalDecl = CalleeDecl->getCanonicalDecl();
#ifdef DEBUGCL
    std::cerr << "CanonicalDecl = " << CanonicalDecl->getName().str()
      << std::endl;
#endif
    if (!ValidFunctionDecls.count(CanonicalDecl)) {
#ifdef DEBUGCL
      std::cerr << "!ValidFunctionDecls.count" << std::endl;
#endif
      continue;
    }

    if (!hasValidArgExprs(*CI)) {
#ifdef DEBUGCL
      std::cerr << "!hasValidArgExprs" << std::endl;
#endif
      continue;
    }

    ValidInstanceNum++;
#ifdef DEBUGCL
    std::cerr << "TransformationCounter = " << TransformationCounter
      << " and ValidInstanceNum = " << ValidInstanceNum << std::endl;
#endif
    if (TransformationCounter == ValidInstanceNum) {
#ifdef DEBUGCL
      std::cerr << "TransformationCounter == ValidInstanceNum" << std::endl;
#endif
      // It's possible the direct callee is not a definition
      if (!CalleeDecl->isThisDeclarationADefinition()) {
        CalleeDecl = CalleeDecl->getFirstDeclaration();
        for(FunctionDecl::redecl_iterator RI = CalleeDecl->redecls_begin(),
            RE = CalleeDecl->redecls_end(); RI != RE; ++RI) {
          if ((*RI)->isThisDeclarationADefinition()) {
            CalleeDecl = (*RI);
            break;
          }
        }
      }
      TransAssert(CalleeDecl->isThisDeclarationADefinition() && 
                  "Bad CalleeDecl!");
      CurrentFD = CalleeDecl;
      TheCaller = CalleeToCallerMap[(*CI)];
      TransAssert(TheCaller && "NULL TheCaller!");
#ifdef DEBUGCL
      std::cerr << "Assigning TheCallExpr" << std::endl;
#endif
      TheCallExpr = (*CI);
    }
  }
}

std::string SimpleInliner::getNewTmpName(void)
{
  std::stringstream SS;
  SS << RewriteHelper->getTmpVarNamePrefix() << NamePostfix;
  NamePostfix++;
  return SS.str();
}

void SimpleInliner::createReturnVar(void)
{
  const Type *FDType = CurrentFD->getResultType().getTypePtr();
  const Type *CallExprType = TheCallExpr->getCallReturnType().getTypePtr();

  // We don't need tmp var
  if (FDType->isVoidType() && CallExprType->isVoidType()) {
    return; 
  }

  TmpVarName = getNewTmpName();
  std::string VarStr = TmpVarName;
  CurrentFD->getResultType().getAsStringInternal(VarStr, 
                               Context->getPrintingPolicy());
  VarStr += ";";
  RewriteHelper->addLocalVarToFunc(VarStr, TheCaller);
}

void SimpleInliner::generateParamStrings(void)
{
  unsigned int ArgNum = TheCallExpr->getNumArgs();
  FunctionDecl *FD = TheCallExpr->getDirectCallee();
  unsigned int Idx;

  for(Idx = 0; Idx < FD->getNumParams(); ++Idx) {
    const ParmVarDecl *PD = FD->getParamDecl(Idx);
    std::string ParmStr = PD->getNameAsString();
    PD->getType().getAsStringInternal(ParmStr, 
                                      Context->getPrintingPolicy());
    if (Idx < ArgNum) {
      const Expr *Arg = TheCallExpr->getArg(Idx);
      ParmStr += " = ";
      std::string ArgStr("");
      RewriteHelper->getExprString(Arg, ArgStr);
      ParmStr += ArgStr;
    }
    ParmStr += ";\n";
    ParmStrings.push_back(ParmStr);
  }
}

void SimpleInliner::insertReturnStmt
      (std::vector< std::pair<ReturnStmt *, int> > &SortedReturnStmts,
       ReturnStmt *RS, int Off)
{
  std::pair<ReturnStmt *, int> ReturnStmtOffPair(RS, Off);
  if (SortedReturnStmts.empty()) {
    SortedReturnStmts.push_back(ReturnStmtOffPair);
    return;
  }

  std::vector< std::pair<ReturnStmt *, int> >::iterator I, E;
  for(I = SortedReturnStmts.begin(), E = SortedReturnStmts.end(); I != E; ++I) {
    int TmpOff = (*I).second;
    if (Off < TmpOff)
      break;
  }

  if (I == E)
    SortedReturnStmts.push_back(ReturnStmtOffPair);
  else 
    SortedReturnStmts.insert(I, ReturnStmtOffPair);
}

void SimpleInliner::sortReturnStmtsByOffs(const char *StartBuf, 
       std::vector< std::pair<ReturnStmt *, int> > &SortedReturnStmts)
{
  for (ReturnStmtsVector::iterator I = ReturnStmts.begin(), 
       E = ReturnStmts.end(); I != E; ++I) {
    ReturnStmt *RS = (*I);
    SourceLocation RSLocStart = RS->getLocStart();
    const char *RSStartBuf = SrcManager->getCharacterData(RSLocStart);
    int Off = RSStartBuf - StartBuf;
    TransAssert((Off >= 0) && "Bad Offset!");
    insertReturnStmt(SortedReturnStmts, RS, Off);
  }
}

void SimpleInliner::copyFunctionBody(void)
{
  Stmt *Body = CurrentFD->getBody();
  TransAssert(Body && "NULL Body!");

  std::string FuncBodyStr("");
  RewriteHelper->getStmtString(Body, FuncBodyStr);
  TransAssert(FuncBodyStr[0] == '{');

  SourceLocation StartLoc = Body->getLocStart();
  const char *StartBuf = SrcManager->getCharacterData(StartLoc);

  std::vector< std::pair<ReturnStmt *, int> > SortedReturnStmts;
  sortReturnStmtsByOffs(StartBuf, SortedReturnStmts);

  // Now we start rewriting
  int Delta = 1; // skip the first { symbol
  FuncBodyStr.insert(Delta, "\n");
  ++Delta;
  for(SmallVector<std::string, 10>::iterator I = ParmStrings.begin(),
       E = ParmStrings.end(); I != E; ++I) {
    std::string PStr = (*I);
    FuncBodyStr.insert(Delta, PStr);
    Delta += PStr.size();
  }

  // restore the effect of {
  Delta--;
  int ReturnSZ = 6;
  std::string TmpVarStr = TmpVarName + " = ";
  int TmpVarNameSize = static_cast<int>(TmpVarStr.size());

  for(std::vector< std::pair<ReturnStmt *, int> >::iterator
      I = SortedReturnStmts.begin(), E = SortedReturnStmts.end(); 
      I != E; ++I) {

    ReturnStmt *RS = (*I).first;
    int Off = (*I).second + Delta;
    Expr *Exp = RS->getRetValue();
    if (Exp) {
      const Type *T = Exp->getType().getTypePtr();
      if (!T->isVoidType()) {
        FuncBodyStr.replace(Off, ReturnSZ, TmpVarStr);
        Delta += (TmpVarNameSize - ReturnSZ);
        continue;
      }
    }
    FuncBodyStr.replace(Off, ReturnSZ, "");
    Delta -= ReturnSZ;
  }

  RewriteHelper->addStringBeforeStmt(TheStmt, FuncBodyStr, NeedParen);
}

void SimpleInliner::removeFunctionBody(void)
{
  SourceRange FDRange = CurrentFD->getSourceRange();
  TheRewriter.RemoveText(FDRange);
}

void SimpleInliner::replaceCallExpr(void)
{
  // Create a new tmp var for return value
  createReturnVar();
  generateParamStrings();
  copyFunctionBody();
  RewriteHelper->replaceExprNotInclude(TheCallExpr, TmpVarName);

  FunctionDecl *CanonicalFD = CurrentFD->getCanonicalDecl();
  if (FunctionDeclNumCalls[CanonicalFD] == 1)
    removeFunctionBody();
}

SimpleInliner::~SimpleInliner(void)
{
#ifdef DEBUGCL
  std::cerr << "Destroying SimpleInliner" << std::endl;
#endif
  delete NameQueryWrap;
  delete CollectionVisitor;
  delete FunctionVisitor;
  delete FunctionStmtVisitor;
  delete StmtVisitor;
}

