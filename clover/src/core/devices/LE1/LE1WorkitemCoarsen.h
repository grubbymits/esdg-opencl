#ifndef _WORKITEM_COARSEN_H
#define _WORKITEM_COARSEN_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <string>
#include <sstream>

namespace llvm {
  class Module;
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
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;

class WorkitemCoarsen {

public:
  WorkitemCoarsen(unsigned x, unsigned y, unsigned z) :
    LocalX(x), LocalY(y), LocalZ(z) { }
  bool CreateWorkgroup(std::string &Filename);
  bool HandleBarriers();
  bool Compile(std::string &filename, std::string &source);
  std::string &getInitialisedKernel() { return InitKernelSource; }
  std::string &getFinalKernel() { return FinalKernel; }

private:
  bool ProduceBytecode(std::string &filename, std::string &source);
  bool ProduceAssembly(std::string &filename);
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
  void Parse();
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

private:
class KernelInitialiser : public clang::RecursiveASTVisitor<KernelInitialiser> {
public:
  KernelInitialiser(clang::Rewriter &R, unsigned x, unsigned y, unsigned z);
  bool VisitFunctionDecl(clang::FunctionDecl *f);
  bool VisitDeclStmt(clang::Stmt *s);
  bool VisitCallExpr(clang::Expr *s);
  // TODO Visit ForStmts and ensure that they have open and close brackets,
  // and do the same for if statements.

private:
  void CloseLoop(clang::SourceLocation Loc);
  void OpenLoop(clang::SourceLocation Loc);

private:
  unsigned LocalX;
  unsigned LocalY;
  unsigned LocalZ;
  std::stringstream OpenWhile;
  std::stringstream CloseWhile;
  clang::Rewriter &TheRewriter;

}; // end class KernelInitialiser

class ThreadSerialiser : public clang::RecursiveASTVisitor<ThreadSerialiser>
{
public:
  ThreadSerialiser(clang::Rewriter &R, unsigned x, unsigned y, unsigned z);
  bool VisitForStmt(clang::Stmt *s);
  bool VisitCallExpr(clang::Expr *s);
  bool WalkUpFromUnaryContinueStmt(clang::UnaryOperator *s);
  bool VisitDeclStmt(clang::Stmt *s);
  bool VisitDeclRefExpr(clang::Expr *expr);
  bool VisitFunctionDecl(clang::FunctionDecl *f);

private:
  void CloseLoop(clang::SourceLocation Loc);
  void OpenLoop(clang::SourceLocation Loc);
  bool BarrierInLoop(clang::ForStmt *s);
  void CreateLocalVariable(clang::DeclRefExpr *Ref, bool ScalarRepl);
  void AccessScalar(clang::Decl *decl);
  void AccessScalar(clang::DeclRefExpr *Ref);

private:
  unsigned LocalX;
  unsigned LocalY;
  unsigned LocalZ;
  std::stringstream OpenWhile;
  std::stringstream CloseWhile;
  clang::SourceLocation FuncBodyStart;
  clang::SourceLocation FuncStart;
  StmtSetMap StmtRefs;
  DeclRefSetMap AllRefs;
  std::vector<std::string> ParamVars;
  std::vector<clang::NamedDecl*> Decls;
  std::map<std::string, clang::SourceLocation> DeclLocs;
  std::map<std::string, clang::NamedDecl*> NewLocalDecls;
  std::map<std::string, clang::NamedDecl*> NewScalarRepls;
  std::map<std::string, clang::DeclStmt*> DeclStmts;
  std::map<clang::SourceLocation, clang::CallExpr*>LoopBarriers;
  std::map<clang::SourceLocation, clang::ForStmt*>LoopsWithBarrier;
  std::map<clang::SourceLocation, clang::ForStmt*>LoopsWithoutBarrier;
  std::vector<clang::ForStmt*>LoopsToDistribute;
  std::vector<clang::CallExpr*> BarrierCalls;
  clang::CallExpr* LoopBarrier;

  clang::Rewriter &TheRewriter;

}; // end class ThreadSerialiser

}; // end class WorkitemCoarsen

#endif
