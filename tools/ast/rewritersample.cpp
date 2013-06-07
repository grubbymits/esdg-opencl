#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace std;

typedef std::vector<Stmt*> StmtSet;
typedef std::vector<DeclRefExpr*> DeclRefSet;
typedef std::map<std::string, StmtSet> StmtSetMap;
typedef std::map<std::string, DeclRefSet> DeclRefSetMap;

class KernelInitializer : public RecursiveASTVisitor<KernelInitializer>
{
private:
  unsigned LocalX;
  unsigned LocalY;
  unsigned LocalZ;
  std::stringstream OpenWhile;
  std::stringstream CloseWhile;
  Rewriter &TheRewriter;

private:
  void CloseLoop(SourceLocation Loc) {
    TheRewriter.InsertText(Loc, CloseWhile.str(), true, true);
  }

  void OpenLoop(SourceLocation Loc) {
    TheRewriter.InsertText(Loc, OpenWhile.str(), true, true);
  }

public:
  KernelInitializer(Rewriter &R, unsigned x, unsigned y, unsigned z)
    : LocalX(x), LocalY(y), LocalZ(z), TheRewriter(R)
  {
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

  bool VisitFunctionDecl(FunctionDecl *f) {
    // Only function definitions (with bodies), not declarations.
    if (f->hasBody()) {
      SourceLocation FuncStart =
        f->getBody()->getLocStart().getLocWithOffset(1);
      Stmt *FuncBody = f->getBody();
      SourceLocation FuncBodyStart =
        FuncBody->getLocStart().getLocWithOffset(2);

      stringstream FuncBegin;
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

  bool VisitDeclStmt(Stmt *s) {
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
    bool VisitCallExpr(Expr *s) {
      CallExpr *Call = cast<CallExpr>(s);
      FunctionDecl* FD = Call->getDirectCallee();
      DeclarationName DeclName = FD->getNameInfo().getName();
      string FuncName = DeclName.getAsString();

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

    // TODO Visit ForStmts and ensure that they have open and close brackets,
    // and do the same for if statements.


};

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class ThreadSerializer : public RecursiveASTVisitor<ThreadSerializer>
{
private:
  unsigned LocalX;
  unsigned LocalY;
  unsigned LocalZ;
  std::stringstream OpenWhile;
  std::stringstream CloseWhile;

private:
  void CloseLoop(SourceLocation Loc) {
    TheRewriter.InsertText(Loc, CloseWhile.str(), true, true);
  }

  void OpenLoop(SourceLocation Loc) {
    TheRewriter.InsertText(Loc, OpenWhile.str(), true, true);
  }

  void CreateLocalVariable(DeclRefExpr *Ref, bool ScalarRepl) {
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

  void AccessScalar(Decl *decl) {
    NamedDecl *ND = cast<NamedDecl>(decl);
    unsigned offset = ND->getName().str().length();
    offset += cast<ValueDecl>(ND)->getType().getAsString().length();
    // increment because of a space between type and name
    ++offset;
    SourceLocation Loc = ND->getLocStart().getLocWithOffset(offset);
    TheRewriter.InsertText(Loc, "[__kernel_local_id[0]]", true);
  }

  // TODO Try and pass just a NamedDecl so we can use this for DeclStmts too
  void AccessScalar(DeclRefExpr *Ref) {
    std::cout << "Creating scalar access for "
      << Ref->getDecl()->getName().str() << std::endl;
    unsigned offset = Ref->getDecl()->getName().str().length();
    SourceLocation loc = Ref->getLocEnd().getLocWithOffset(offset);
    TheRewriter.InsertText(loc, "[__kernel_local_id[0]]", true);
  }

public:
    ThreadSerializer(Rewriter &R, unsigned x, unsigned y, unsigned z)
        : LocalX(x), LocalY(y), LocalZ(z), TheRewriter(R)
    {
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

    bool BarrierInLoop(ForStmt* s) {
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
          string FuncName = DeclName.getAsString();

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
    bool VisitForStmt(Stmt *s) {
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
          const DeclStmt *CondVar = Loop->getConditionVariableDeclStmt();
          Expr *Cond = Loop->getCond();
          Expr *Inc = Loop->getInc();

          stringstream LoopHeader;
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
    bool VisitCallExpr(Expr *s) {
      std::cout << "VisitCallExpr\n";
      CallExpr *Call = cast<CallExpr>(s);
      FunctionDecl* FD = Call->getDirectCallee();
      DeclarationName DeclName = FD->getNameInfo().getName();
      string FuncName = DeclName.getAsString();

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

    bool WalkUpFromUnaryContinueStmt(UnaryOperator *S) {
      return true;
    }

    bool VisitDeclStmt(Stmt *s) {
      DeclStmt *DS = cast<DeclStmt>(s);
      if (DS->isSingleDecl()) {
        std::cout << "Got DeclStmt = " << TheRewriter.ConvertToString(DS);
        std::cout << "Children:\n";
        for (Stmt::child_iterator DI = DS->child_begin(), DE = DS->child_end();
             DI != DE; ++DI) {
          std::cout << TheRewriter.ConvertToString(*DI);
        }
        NamedDecl *ND = cast<NamedDecl>(DS->getSingleDecl());
        std::string key = ND->getName().str();
        DeclStmts.insert(std::make_pair(key, DS));
      }
      return true;
    }

  bool VisitDeclRefExpr(Expr *expr) {
    DeclRefExpr *RefExpr = cast<DeclRefExpr>(expr);
    std::string key = RefExpr->getDecl()->getName().str();
    std::cout << "VisitDeclRefExpr " << key << std::endl;

    if (AllRefs.find(key) == AllRefs.end()) {
      std::cout << "Creating new vector for " << key << std::endl;
      DeclRefSet refset;
      refset.push_back(RefExpr);
      AllRefs.insert(std::make_pair(key, refset));
    }
    else {
      std::cout << "Adding " << key << " to existing vector\n";
      AllRefs[key].push_back(RefExpr);
    }

    SourceLocation RefLoc = RefExpr->getLocStart();

    // If we've already added it, don't do it again, but we may need to access
    // an array instead
    if (NewScalarRepls.find(key) != NewScalarRepls.end()) {
      std::cout << "key already exists in NewScalarRepls\n";
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


    bool VisitFunctionDecl(FunctionDecl *f) {
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

private:
    void AddBraces(Stmt *s);

    SourceLocation FuncBodyStart;
    SourceLocation FuncStart;
    StmtSetMap StmtRefs;
    DeclRefSetMap AllRefs;
    std::vector<std::string> ParamVars;
    std::vector<NamedDecl*> Decls;
    std::map<std::string, SourceLocation> DeclLocs;
    std::map<std::string, NamedDecl*> NewLocalDecls;
    std::map<std::string, NamedDecl*> NewScalarRepls;
    std::map<std::string, DeclStmt*> DeclStmts;
    std::map<SourceLocation, CallExpr*>LoopBarriers;
    std::map<SourceLocation, ForStmt*>LoopsWithBarrier;
    std::map<SourceLocation, ForStmt*>LoopsWithoutBarrier;
    std::vector<ForStmt*>LoopsToDistribute;
    std::vector<CallExpr*> BarrierCalls;
    CallExpr* LoopBarrier;

    Rewriter &TheRewriter;
};


template <typename T> class OpenCLASTConsumer : public ASTConsumer
{
public:
    OpenCLASTConsumer(Rewriter &R, unsigned x, unsigned y, unsigned z)
        : Visitor(R, x, y, z)
    {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end();
             b != e; ++b)
            // Traverse the declaration using our AST visitor.
            Visitor.TraverseDecl(*b);
        return true;
    }

    T &getVisitor() { return Visitor; }

private:
    T Visitor;
};

template <typename T> class OpenCLCompiler {
public:
    OpenCLCompiler() {
      s_log = new llvm::raw_string_ostream(log);

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
      TheCompInst.getHeaderSearchOpts().ResourceDir =
        "/usr/local/lib/clang/3.2/";
      TheCompInst.getHeaderSearchOpts().UseBuiltinIncludes = true;
      TheCompInst.getHeaderSearchOpts().UseStandardSystemIncludes = false;

      TheCompInst.getPreprocessorOpts().Includes.push_back("clc/clc.h");
      TheCompInst.getPreprocessorOpts().addMacroDef(
        "cl_clang_storage_class_specifiers");
      TheCompInst.getInvocation().setLangDefaults(TheCompInst.getLangOpts(),
                                                clang::IK_AST);


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
      //TheConsumer = new T(TheRewriter);
      TheConsumer = new OpenCLASTConsumer<T>(TheRewriter, 10, 0, 0);
    }

    ~OpenCLCompiler() { delete TheConsumer; }

    void setFile(char* input) {
      // Set the main file handled by the source manager to the input file.
      const FileEntry *FileIn = FileMgr->getFile(input);
      SourceMgr->createMainFileID(FileIn);
      TheCompInst.getDiagnosticClient().BeginSourceFile(
          TheCompInst.getLangOpts(),
          &TheCompInst.getPreprocessor());
    }

    void setFile(std::string source) {
      //TheCompInst.createDiagnostics(0, NULL, new clang::TextDiagnosticPrinter(
        //  s_log, &TheCompInst.getDiagnosticOpts()));
      TheCompInst.getPreprocessorOpts()
        .addRemappedFile("esdg_kernel.cl",
                         llvm::MemoryBuffer::getMemBuffer(source));
      /*
      SourceMgr->createFileIDForMemBuffer(
        llvm::MemoryBuffer::getMemBuffer(source));
      TheCompInst.getDiagnosticClient().BeginSourceFile(
          TheCompInst.getLangOpts(),
          &TheCompInst.getPreprocessor());*/
      std::cout << "Have set file\n";
    }

    void Parse() {
      // Parse the file to AST, registering our consumer as the AST consumer.
      ParseAST(TheCompInst.getPreprocessor(), TheConsumer,
               TheCompInst.getASTContext());
      std::cout << "Have parsed file\n";
    }

    const RewriteBuffer *getRewriteBuf() {
      std::cout << "Emptying to return the rewrite buffer\n";
      return TheRewriter.getRewriteBufferFor(SourceMgr->getMainFileID());
    }

    OpenCLASTConsumer<T> *getConsumer() { return TheConsumer; }

private:
    CompilerInstance TheCompInst;
    FileManager *FileMgr;
    SourceManager *SourceMgr;
    Rewriter TheRewriter;
    OpenCLASTConsumer<T> *TheConsumer;
    std::string log;
    llvm::raw_string_ostream *s_log;
};

int main(int argc, char *argv[])
{
    if (argc != 2) {
        llvm::errs() << "Usage: rewritersample <filename>\n";
        return 1;
    }

    OpenCLCompiler<KernelInitializer> CLCompiler;
    CLCompiler.setFile(argv[1]);
    CLCompiler.Parse();

    // At this point the rewriter's buffer should be full with the rewritten
    // file contents.
    const RewriteBuffer *RewriteBuf = CLCompiler.getRewriteBuf();
    if (RewriteBuf == NULL)
      return 0;

    // Now parse the code again, scanning for variable declarations,
    // refs and while loops. This information will then tell us whether
    // a closed loop has made a reference out of scope.
    std::string source = string(RewriteBuf->begin(), RewriteBuf->end());

    std::ofstream init_kernel;
    init_kernel.open("esdg_init_kernel.cl");
    init_kernel << source;
    init_kernel.close();

    OpenCLCompiler<ThreadSerializer> SerialCompiler;
    SerialCompiler.setFile("esdg_init_kernel.cl");
    SerialCompiler.Parse();

    RewriteBuf = SerialCompiler.getRewriteBuf();
    if (RewriteBuf == NULL)
      return 0;

    std::ofstream final_kernel;
    final_kernel.open("esdg_final_kernel.cl");
    final_kernel << string(RewriteBuf->begin(), RewriteBuf->end());
    final_kernel.close();


    return 0;
}
