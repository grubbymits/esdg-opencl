#ifndef _WORKITEM_COARSEN_H
#define _WORKITEM_COARSEN_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <string>
#include <sstream>
#include <iostream>

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
  bool CreateWorkgroup(std::string &Filename, std::string &kernel);
  bool ExpandMacros();
  bool HandleBarriers();
  std::string &getInitialisedKernel() { return InitKernelSource; }
  std::string &getFinalKernel() { return FinalKernel; }

private:
  unsigned LocalX, LocalY, LocalZ;
  clang::CompilerInstance TheCompiler;
  llvm::Module *Module;
  std::string OrigFilename;
  std::string KernelName;
  std::string InitKernelSource;
  std::string InitKernelFilename;
  std::string FinalKernel;

template <typename T> class OpenCLCompiler {

public:
  OpenCLCompiler(unsigned x, unsigned y, unsigned z, std::string &name);
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
  OpenCLASTConsumer(clang::Rewriter &R, unsigned x, unsigned y, unsigned z,
                    std::string &name)
    : Visitor(R, x, y, z), KernelName(name) { }

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR) {
    for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end();
         b != e; ++b) {
      if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(*b)) {
        if (FD->hasBody()) {
          // Only visit the specified kernel
          if (FD->getNameAsString().compare(KernelName) != 0) {
            // Delete any other kernel in the source
            if (FD->hasAttr<clang::OpenCLKernelAttr>()) {
              clang::SourceLocation Start = FD->getLocStart();
              clang::SourceLocation Finish = FD->getBody()->getLocEnd();
              Visitor.RemoveText(Start, Finish);
            }
            continue;
          }
          // Traverse the declaration using our AST visitor.
          Visitor.TraverseDecl(*b);
        }
      }
    }
    return true;
  }
  bool needsScalarFixes() const {
    return Visitor.needsToFixScalarAccesses();
  }
  void FixAllScalarAccesses() {
    Visitor.FixAllScalarAccesses();
  }

private:
  std::string KernelName;
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
  void RemoveText(clang::SourceLocation Start, clang::SourceLocation End) {
    TheRewriter.RemoveText(clang::SourceRange(Start, End));
  }
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
  //bool VisitForStmt(clang::Stmt *s);
  bool VisitWhileStmt(clang::Stmt *s);
  bool VisitCallExpr(clang::Expr *s);
  bool VisitReturnStmt(clang::Stmt *s);
  bool WalkUpFromUnaryContinueStmt(clang::UnaryOperator *s);
  bool VisitDeclRefExpr(clang::Expr *expr);
  bool VisitFunctionDecl(clang::FunctionDecl *f);

  virtual bool needsToFixScalarAccesses() const {
    return true;
    //return (!NewScalarRepls.empty());
  }

  virtual void FixAllScalarAccesses();

private:
  clang::SourceLocation GetOffsetInto(clang::SourceLocation Loc);
  clang::SourceLocation GetOffsetOut(clang::SourceLocation Loc);

  void TraverseLoop(clang::Stmt *s);
  void HandleBarrierInLoop(clang::Stmt *Loop);
  bool CheckWithinEnclosedLoop(clang::SourceLocation InsertLoc,
                               clang::DeclStmt *s,
                               clang::Stmt *Scope);
  bool SearchNestedLoops(clang::Stmt *Loop, bool isOuterLoop);
  void ScalarReplicate(clang::SourceLocation InsertLoc,
                       clang::DeclStmt *theDecl);

  void AccessScalar(clang::Decl *decl);
  void AccessScalar(clang::DeclRefExpr *Ref);

private:
  std::map<clang::Stmt*, std::vector<clang::Stmt*> > NestedLoops;
  std::map<clang::Stmt*, std::vector<clang::DeclStmt*> > ScopedDeclStmts;
  std::map<clang::Stmt*, std::vector<clang::CallExpr*> > Barriers;
  std::map<clang::Decl*, std::vector<clang::DeclRefExpr*> > AllRefs;
  clang::SourceLocation FuncBodyStart;
  clang::SourceLocation FuncStart;
  clang::WhileStmt *OuterLoop;
  //DeclRefSetMap AllRefs;
  std::vector<std::string> ParamVars;

}; // end class ThreadSerialiser

}; // end class WorkitemCoarsen

#endif
