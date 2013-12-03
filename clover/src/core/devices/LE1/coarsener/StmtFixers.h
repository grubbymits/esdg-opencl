#ifndef _STMT_FIXERS_H
#define _STMT_FIXERS_H

#include "clang/Basic/SourceManager.h"
#include <list>
#include <string>

namespace clang {
  class Stmt;
  class CallExpr;
  class BreakStmt;
  class ContinueStmt;
  class ReturnStmt;
}

namespace Coal {

template <typename T> class StmtFixer {
public:
  StmtFixer(std::list<std::pair<clang::SourceLocation, std::string > > *list,
            unsigned x, unsigned y, unsigned z);
  virtual ~StmtFixer() { }
  virtual void FixInBarrierPresence(clang::Stmt* Region,
                                    //std::list<std::pair<clang::Stmt*, T> > &PSL,
                                    std::vector<T> &Unaries,
                                    std::vector<clang::CallExpr*> &InnerBarriers,
                                    unsigned depth);
protected:
  void InsertText(clang::SourceLocation InsertLoc, std::string text) {
    SourceToInsert->push_back(std::make_pair(InsertLoc, text));
  }
  std::string TotalValidCheck;
  std::string CurrentValidCheck;
  std::string UnaryConvert;
  std::string InvalidCounter;
  std::string InvalidArray;
  std::string InvalidCounterInit;
  std::string InvalidArrayInit;
  std::string AccessInvalidTotal;
  std::string AccessInvalidArray;
private:
  std::list<std::pair<clang::SourceLocation, std::string> > *SourceToInsert;
  unsigned localX, localY, localZ;
};

template <typename T> class LocalStmtFixer : public Coal::StmtFixer<T> {
public:
  LocalStmtFixer(std::list<std::pair<clang::SourceLocation, std::string > > *list,
                 unsigned x, unsigned y, unsigned z);
  void FixInBarrierPresence(clang::Stmt *Region,
                            //std::list<std::pair<clang::Stmt*, T> > &PSL,
                            std::vector<T> &Unaries,
                            std::vector<clang::CallExpr*> &InnerBarriers,
                            unsigned depth);
};

class ReturnFixer : public StmtFixer<clang::ReturnStmt*> {
public:
  ReturnFixer(std::list<std::pair<clang::SourceLocation, std::string > > *list,
              unsigned x, unsigned y, unsigned z);
};

class BreakFixer : public LocalStmtFixer<clang::BreakStmt*> {
public:
  BreakFixer(std::list<std::pair<clang::SourceLocation, std::string > > *list,
             unsigned x, unsigned y, unsigned z);
};

class ContinueFixer : public LocalStmtFixer<clang::ContinueStmt*> {
public:
  ContinueFixer(std::list<std::pair<clang::SourceLocation, std::string > > *list,
                unsigned x, unsigned y, unsigned z);
};

}

#endif
