#ifndef _WORKITEM_COARSEN_H
#define _WORKITEM_COARSEN_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <string>
#include <sstream>

#define CLANG_RESOURCE_DIR  "/opt/esdg-opencl/lib/clang/3.2/"
#define CLANG_INCLUDE_DIR   "/opt/esdg-opencl/lib/clang/3.2/include"
#define LIBCLC_INCLUDE_DIR  "/opt/esdg-opencl/include"

namespace llvm {
  class Module;
  class raw_ostream;
}

class clang::FunctionDecl;
class clang::Stmt;
class clang::Expr;
class clang::CallExpr;
class clang::DeclRefExpr;
class clang::UnaryOperator;
class clang::FileManager;


typedef std::vector<clang::Stmt*> StmtSet;
typedef std::vector<clang::DeclRefExpr*> DeclRefSet;
typedef std::vector<clang::NamedDecl*> NamedDeclSet;
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;
typedef std::map<std::string, NamedDeclSet> NamedDeclSetMap;

class WorkitemCoarsen {

public:
  WorkitemCoarsen(unsigned x, unsigned y, unsigned z) :
    LocalX(x), LocalY(y), LocalZ(z) { }
  bool CreateWorkgroup(std::string &Filename);
  bool HandleBarriers();
  std::string &getInitialisedKernel() { return InitKernelSource; }
  std::string &getFinalKernel() { return FinalKernel; }

private:
  unsigned LocalX, LocalY, LocalZ;
  clang::CompilerInstance TheCompiler;
  llvm::Module *Module;
  std::string OrigFilename;
  std::string InitKernelSource;
  std::string InitKernelFilename;
  std::string FinalKernel;

template <typename T> class OpenCLCompiler {

public:
  OpenCLCompiler(unsigned x, unsigned y, unsigned z);
  ~OpenCLCompiler();
  void setFile(std::string input);
  void expandMacros(llvm::raw_ostream *source);
  void Parse();

  bool needsScalarFixes() {
    return TheConsumer->needsScalarFixes();
  }
  void FixAllScalarAccesses() {
    TheConsumer->FixAllScalarAccesses();
  }

  const clang::RewriteBuffer *getRewriteBuf() {
    return TheRewriter.getRewriteBufferFor(SourceMgr->getMainFileID());
  }

private:

class OpenCLASTConsumer : public clang::ASTConsumer {

public:
  OpenCLASTConsumer(clang::Rewriter &R, unsigned x, unsigned y, unsigned z)
    : Visitor(R, x, y, z) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR) {
    for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end();
         b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
      return true;
    }
  bool needsScalarFixes() const {
    return Visitor.needsToFixScalarAccesses();
  }
  void FixAllScalarAccesses() {
    Visitor.FixAllScalarAccesses();
  }

private:
  T Visitor;

};  // end class OpenCLASTConsumer

private:
  clang::CompilerInstance TheCompInst;
  clang::FileManager *FileMgr;
  clang::SourceManager *SourceMgr;
  clang::Rewriter TheRewriter;
  OpenCLASTConsumer *TheConsumer;

}; // end class OpenCLCompiler

// Base class for our visitors, ensures an interface for fixing scalars
// and handles the trivial tasks of creating the frequently used strings
// to open and close loops
public:

template <typename T> class ASTVisitorBase :
  public clang::RecursiveASTVisitor<T> {
public:
  ASTVisitorBase(clang::Rewriter &R, unsigned x, unsigned y, unsigned z);
  virtual bool needsToFixScalarAccesses() const = 0;
  virtual void FixAllScalarAccesses() = 0;
protected:
  void CloseLoop(clang::SourceLocation Loc);
  void OpenLoop(clang::SourceLocation Loc);
  unsigned LocalX;
  unsigned LocalY;
  unsigned LocalZ;
  std::stringstream OpenWhile;
  std::stringstream CloseWhile;
  clang::Rewriter &TheRewriter;
};

private:

class KernelInitialiser : public ASTVisitorBase<KernelInitialiser> {
public:
  KernelInitialiser(clang::Rewriter &R, unsigned x, unsigned y, unsigned z)
    : ASTVisitorBase(R, x, y, z) { }
  bool VisitFunctionDecl(clang::FunctionDecl *f);
  bool VisitDeclStmt(clang::Stmt *s);
  bool VisitCallExpr(clang::Expr *s);

  virtual bool needsToFixScalarAccesses() const { return false; }
  virtual void FixAllScalarAccesses() { return; }
};

class ThreadSerialiser : public ASTVisitorBase<ThreadSerialiser> {
public:
  ThreadSerialiser(clang::Rewriter &R, unsigned x, unsigned y, unsigned z)
    : ASTVisitorBase(R, x, y, z) { }
  bool VisitForStmt(clang::Stmt *s);
  bool VisitCallExpr(clang::Expr *s);
  bool WalkUpFromUnaryContinueStmt(clang::UnaryOperator *s);
  bool VisitDeclStmt(clang::Stmt *s);
  bool VisitDeclRefExpr(clang::Expr *expr);
  bool VisitParenExpr(clang::Expr *expr);
  bool VisitFunctionDecl(clang::FunctionDecl *f);

  virtual bool needsToFixScalarAccesses() const {
    return (!NewScalarRepls.empty());
  }

  virtual void FixAllScalarAccesses();

private:
  clang::SourceLocation GetOffsetInto(clang::SourceLocation Loc);
  clang::SourceLocation GetOffsetOut(clang::SourceLocation Loc);
  void FindRefsToReplicate(clang::Stmt *s);
  void FindScopedVariables(clang::Stmt *s);
  void HandleBarrierInLoop(clang::ForStmt *Loop);
  bool BarrierInLoop(clang::ForStmt *s);
  void CreateLocalVariable(clang::DeclRefExpr *Ref, bool ScalarRepl);
  void AccessScalar(clang::Decl *decl);
  void AccessScalar(clang::DeclRefExpr *Ref);

private:
  std::stringstream LocalArray;
  std::stringstream SavedLocalArray;

  clang::SourceLocation FuncBodyStart;
  clang::SourceLocation FuncStart;
  StmtSetMap StmtRefs;
  DeclRefSetMap AllRefs;
  std::vector<std::string> ParamVars;
  std::vector<clang::NamedDecl*> Decls;
  std::map<std::string, clang::SourceLocation> DeclLocs;
  std::map<std::string, clang::NamedDecl*> NewLocalDecls;
  std::map<std::string, clang::NamedDecl*> ScopedVariables;
  std::map<std::string, clang::NamedDecl*> NewScalarRepls;
  std::map<std::string, clang::DeclStmt*> DeclStmts;
  std::map<clang::SourceLocation, clang::CallExpr*>LoopBarriers;
  std::map<clang::SourceLocation, clang::ForStmt*>LoopsWithBarrier;
  std::map<clang::SourceLocation, clang::ForStmt*>LoopsWithoutBarrier;
  std::vector<clang::ForStmt*>LoopsToDistribute;
  std::vector<clang::CallExpr*> BarrierCalls;
  clang::CallExpr* LoopBarrier;

}; // end class ThreadSerialiser

}; // end class WorkitemCoarsen

#endif
