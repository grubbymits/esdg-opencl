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
#include <pthread.h>

#define CLANG_RESOURCE_DIR  "/opt/esdg-opencl/lib/clang/3.2/"
#define CLANG_INCLUDE_DIR   "/opt/esdg-opencl/lib/clang/3.2/include"
#define LIBCLC_INCLUDE_DIR  "/opt/esdg-opencl/include"

namespace llvm {
  class Module;
  class raw_ostream;
}

namespace clang {
  class FunctionDecl;
  class Stmt;
  class Expr;
  class CallExpr;
  class DeclRefExpr;
  class UnaryOperator;
  class FileManager;

}

//template <class, T, class Allocator = std::allocator<T> > class StmtFixer;
namespace Coal {

class ReturnFixer;
class BreakFixer;

typedef std::vector<clang::Stmt*> StmtSet;
typedef std::vector<clang::DeclRefExpr*> DeclRefSet;
typedef std::vector<clang::NamedDecl*> NamedDeclSet;
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;
typedef std::map<std::string, NamedDeclSet> NamedDeclSetMap;

typedef std::list<std::pair<clang::Stmt*, clang::ReturnStmt*> > PairedReturnList;
typedef std::list<std::pair<clang::Stmt*, clang::ContinueStmt*> > PairedContinueList;
typedef std::list<std::pair<clang::Stmt*, clang::BreakStmt*> > PairedBreakList;
typedef std::list<std::pair<clang::SourceLocation, std::string> > StringList;

class WorkitemCoarsen {

public:
  WorkitemCoarsen(unsigned x, unsigned y, unsigned z);
  bool CreateWorkgroup(std::string &Filename, std::string &kernel);
  bool HandleBarriers();
  void DeleteTempFiles();
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
  std::vector<std::string> TempFiles;
  pthread_mutex_t p_inline_mutex;

template <typename T> class OpenCLCompiler {

public:
  OpenCLCompiler(unsigned x, unsigned y, unsigned z, std::string &name);
  ~OpenCLCompiler();
  void setFile(std::string input);
  void Parse();
  const clang::RewriteBuffer *getRewriteBuf() {
    TheConsumer->RewriteSource();
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
          // Delete any other kernel in the source
          if ((FD->hasAttr<clang::OpenCLKernelAttr>()) &&
              (FD->getNameAsString().compare(KernelName) != 0)) {
              clang::SourceLocation Start = FD->getLocStart();
              clang::SourceLocation Finish = FD->getBody()->getLocEnd();
              Visitor.RemoveText(Start, Finish);
              continue;
          }
          if (!FD->hasAttr<clang::OpenCLKernelAttr>())
            continue;
          /*
          else {
            if (FD->isInlineSpecified()) {
              Visitor.InsertText(FD->getLocStart(), "//");
              Visitor.InsertText(FD->getLocStart().getLocWithOffset(6), "\n");
            }
          }*/
          // Traverse the declaration using our AST visitor.
          Visitor.TraverseDecl(*b);
        }
      }
    }
    //Visitor.RewriteSource();
    return true;
  }
  void RewriteSource() {
    Visitor.RewriteSource();
  }
  /*
  bool needsScalarFixes() const {
    return Visitor.needsToFixScalarAccesses();
  }
  void FixAllScalarAccesses() {
    Visitor.FixAllScalarAccesses();
  }*/

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
  //virtual bool needsToFixScalarAccesses() const = 0;
  //virtual void FixAllScalarAccesses() = 0;
  virtual void RewriteSource();

  void InsertText(clang::SourceLocation InsertLoc, std::string text) {
    SourceToInsert.push_back(std::make_pair(InsertLoc, text));
  }
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
  StringList SourceToInsert;
};

private:

class KernelInitialiser : public ASTVisitorBase<KernelInitialiser> {
public:
  KernelInitialiser(clang::Rewriter &R, unsigned x, unsigned y, unsigned z)
    : ASTVisitorBase(R, x, y, z) { }
  bool VisitFunctionDecl(clang::FunctionDecl *f);
  bool VisitDeclStmt(clang::Stmt *s);
  bool VisitCallExpr(clang::Expr *s);
};

class ThreadSerialiser : public ASTVisitorBase<ThreadSerialiser> {
public:
  ThreadSerialiser(clang::Rewriter &R, unsigned x, unsigned y, unsigned z);
  ~ThreadSerialiser() {
    delete returnFixer;
    delete breakFixer;
  }
  void RewriteSource();

  //bool VisitForStmt(clang::Stmt *s);
  bool VisitForStmt(clang::Stmt *s);
  //bool VisitCallExpr(clang::Expr *s);
  //bool VisitReturnStmt(clang::Stmt *s);
  bool WalkUpFromUnaryContinueStmt(clang::UnaryOperator *s);
  bool VisitDeclRefExpr(clang::Expr *expr);
  bool VisitUnaryOperator(clang::Expr *expr);
  bool VisitBinaryOperator(clang::Expr *expr);
  bool VisitFunctionDecl(clang::FunctionDecl *f);

private:
  clang::SourceLocation GetOffsetInto(clang::SourceLocation Loc);
  clang::SourceLocation GetOffsetOut(clang::SourceLocation Loc);

  void TraverseRegion(clang::Stmt *s);
  void TraverseConditionalRegion(clang::Stmt* Region,
                                 clang::Stmt* s);
  void CheckForUnary(clang::Stmt *Region,
                     clang::Stmt *Then,
                     clang::Stmt *unary);
  void HandleNonParallelRegion(clang::Stmt *Region, int depth);
  void FixReturnsInBarrierAbsence(clang::Stmt* Region,
                                  unsigned depth);
  //void HandleBreaks(clang::Stmt *Region);
  bool CheckWithinEnclosedLoop(clang::SourceLocation InsertLoc,
                               clang::DeclStmt *s,
                               clang::Stmt *Scope);
  void SearchForIndVars(clang::Stmt *s);
  bool SearchThroughRegions(clang::Stmt *Loop);
  void AssignIndVars(void);
  void FindRefsToExpand(std::list<clang::DeclStmt*> &Stmts,
                        clang::Stmt *Loop);
  void ExpandDecl(std::stringstream &NewDecl);
  void ExpandRef(std::stringstream &NewRef);
  void ScalarExpand(clang::SourceLocation InsertLoc,
                    clang::DeclStmt *theDecl);
  bool CreateLocal(clang::SourceLocation InsertLoc,
                   clang::DeclStmt *s,
                   bool toExpand);
  void RemoveScalarDeclStmt(clang::DeclStmt *DS,
                            bool toExpand);
  void RemoveNonScalarDeclStmt(clang::DeclStmt *DS,
                               bool toExpand);

  void AccessScalar(clang::DeclRefExpr *Ref);
  void AccessNonScalar(clang::DeclRefExpr *Ref);

private:
  std::map<clang::Stmt*, std::vector<clang::Stmt*> > NestedLoops;
  std::map<clang::Stmt*, std::list<clang::DeclStmt*> > ScopedDeclStmts;
  std::map<clang::Stmt*, std::vector<clang::CallExpr*> > Barriers;
  std::map<clang::Stmt*, std::vector<clang::CompoundStmt*> > ScopedRegions;

  std::map<clang::Stmt*, PairedContinueList> ContinueStmts;
  std::map<clang::Stmt*, PairedBreakList> BreakStmts;
  std::map<clang::Stmt*, PairedReturnList> ReturnStmts;

  std::map<clang::Decl*, std::vector<clang::DeclRefExpr*> > AllRefs;
  std::map<clang::Decl*, clang::Stmt*> DeclParents;
  std::map<clang::Decl*, std::list<clang::DeclRefExpr*> > RefAssignments;
  std::map<clang::Decl*, std::list<clang::DeclRefExpr*> > PotentialIndVars;

  std::vector<clang::Decl*> IndVars;
  std::vector<clang::FunctionDecl*> AllFunctions;
  std::vector<clang::FunctionDecl*> CalledFunctions;


  clang::SourceLocation FuncBodyStart;
  clang::SourceLocation FuncStart;
  bool isFirstLoop;
  clang::ForStmt *OuterLoop;
  std::vector<clang::Decl*> ParamVars;
  std::stringstream InvalidThreadInit;
  ReturnFixer *returnFixer;
  BreakFixer *breakFixer;

}; // end class ThreadSerialiser

}; // end class WorkitemCoarsen

}
#endif