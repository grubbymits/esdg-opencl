#include "StmtFixers.h"
#include "SourceRewriter.h"

#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

using namespace clang;
using namespace Coal;

template <typename T>
StmtFixer<T>::StmtFixer(StringList *list, unsigned x, unsigned y, unsigned z)
  : SourceToInsert(list), localX(x), localY(y), localZ(z) {
}

ReturnFixer::ReturnFixer(StringList *list, unsigned x, unsigned y, unsigned z)
  : StmtFixer(list, x, y, z) {

  InvalidCounter = "__kernel_total_invalid_global_threads";
  InvalidArray = "__kernel_invalid_global_threads";

  InvalidCounterInit = "unsigned ";
  InvalidCounterInit.append(InvalidCounter).append(" = 0;");

  std::stringstream arrayInit;
  std::stringstream arrayAccess;
  std::stringstream convert;

  arrayInit << "bool " << InvalidArray << "[" << x << "]";
  arrayAccess << InvalidArray << "[__esdg_idx]";
  if (y > 1) {
    arrayInit << "[" << y << "]";
    arrayAccess << "[__esdg_idy]";
  }
  if (z > 1) {
    arrayInit << "[" << z << "]";
    arrayAccess << "[__esdg_idz]";
  }
  arrayInit << " = { false };";

  AccessInvalidArray = arrayAccess.str();
  InvalidArrayInit = arrayInit.str();

  convert << " { ++" << InvalidCounter << ";\n";
  convert << AccessInvalidArray << " = true;\ncontinue;\n} //";

  UnaryConvert = convert.str();

  // FIXME This only works for one dimension!
  std::stringstream validCheck;
  validCheck << "\nif (" << InvalidCounter << " == " << x << ") return;";
  TotalValidCheck = validCheck.str();

}

template <typename T>
LocalStmtFixer<T>::LocalStmtFixer(StringList *list,
                                  unsigned x,
                                  unsigned y,
                                  unsigned z)
  : StmtFixer<T>(list, x, y, z) {

  this->InvalidCounter = "__kernel_total_invalid_local_threads";
  this->InvalidArray = "__kernel_invalid_local_threads";

  this->InvalidCounterInit = "unsigned ";
  this->InvalidCounterInit.append(this->InvalidCounter).append(" = 0;\n");

  std::stringstream arrayInit;
  std::stringstream arrayAccess;
  std::stringstream convert;

  arrayInit << "bool " << this->InvalidArray << "[" << x << "]";
  arrayAccess << this->InvalidArray << "[__esdg_idx]";
  if (y > 1) {
    arrayInit << "[" << y << "]";
    arrayAccess << "[__esdg_idy]";
  }
  if (z > 1) {
    arrayInit << "[" << z << "]";
    arrayAccess << "[__esdg_idz]";
  }
  arrayInit << " = { false };\n";

  this->AccessInvalidArray = arrayAccess.str();
  this->InvalidArrayInit = arrayInit.str();

  convert << " { ++" << this->InvalidCounter << ";\n";
  convert << this->AccessInvalidArray << " = true;\ncontinue;\n} //";

  this->UnaryConvert = convert.str();

  // FIXME This only works for one dimension!
  std::stringstream totalValidCheck;
  totalValidCheck << "\nif (" << this->InvalidCounter << " == " << x << ")";
  this->TotalValidCheck = totalValidCheck.str();

  this->CurrentValidCheck = "\nif (";
  this->CurrentValidCheck.append(this->AccessInvalidArray).append(") continue;\n");
}

BreakFixer::BreakFixer(StringList *list, unsigned x, unsigned y, unsigned z)
  : LocalStmtFixer(list, x, y, z) {

  this->TotalValidCheck.append(" break;");
}

ContinueFixer::ContinueFixer(StringList *list,
                             unsigned x,
                             unsigned y,
                             unsigned z)
  : LocalStmtFixer(list, x, y, z) {

  this->TotalValidCheck.append(" continue;");
}

// FixInBarrierPresence needs to convert the code from something like this:
//
//for () {
//  some_work();
//  barrier();
//  if ()
//    return;
//  more_work();
//}
//
// To something like this:
//
//bool invalid_threads[local_size];
//unsigned total_invalid_threads = 0;

//for () {
//  while() {
//    some_work();
//  }
//  while() {
//    if () {
//      ++invalid_threads;
//      invalid_index[local_id] = true;
//      continue;
//    }
//    if (invalid_threads == local_size)
//      return;
//    more_work();
//  }
//}
// The while loops also have to check against invalid indexes. This is performed
// in RewriteSource by appending the appropriate source to the OpenWhile string.
template <typename T>
void StmtFixer<T>::FixInBarrierPresence(Stmt *Region,
                                        std::list<std::pair<Stmt*, T> > &PSL,
                                        std::vector<CallExpr*> &InnerBarriers,
                                        unsigned depth) {

#ifdef DBG_WRKGRP
  std::cerr << "Entering FixInBarrierPresence" << std::endl;
#endif

  Stmt *RegionBody;
  if (isa<ForStmt>(Region))
    RegionBody = (cast<ForStmt>(Region))->getBody();
  else if (isa<WhileStmt>(Region))
    RegionBody = (cast<WhileStmt>(Region))->getBody();

  // This region has barriers and they will define the new regions. Insert
  // total thread valid checks at each barrier location.
  // All the new loops in this region should also check the whether the current
  // thread is valid.
  for (std::vector<CallExpr*>::iterator CI = InnerBarriers.begin(),
       CE = InnerBarriers.end(); CI != CE; ++CI) {
    InsertText((*CI)->getLocEnd().getLocWithOffset(2), TotalValidCheck);
    InsertText((*CI)->getLocEnd().getLocWithOffset(4), CurrentValidCheck);
  }

  InsertText(RegionBody->getLocStart().getLocWithOffset(4),
             CurrentValidCheck);

  // The valid counter and array both need to be reset after the region.

  if (PSL.size() == 1) {
#ifdef DBG_WRKGRP
    std::cerr << "Statement list is only one element long" << std::endl;
#endif
    Stmt *cond = PSL.front().first;
    T s = PSL.front().second;
    InsertText(s->getLocStart(), UnaryConvert);
    return;
  }
  // else
  for (typename std::list<std::pair<Stmt*, T> >::iterator SLI = PSL.begin(),
       SLE = PSL.end(); SLI != SLE; ++SLI) {

    Stmt *cond = (*SLI).first;
    T s = (*SLI).second;
    InsertText(s->getLocStart(), UnaryConvert);
  }

}

template <typename T>
void LocalStmtFixer<T>::FixInBarrierPresence(Stmt *Region,
  std::list<std::pair<Stmt*, T> > &PSL,
  std::vector<CallExpr*> &InnerBarriers, unsigned depth) {

  StmtFixer<T>::FixInBarrierPresence(Region, PSL, InnerBarriers, depth);

  this->InsertText(Region->getLocStart(), this->InvalidCounterInit);
  this->InsertText(Region->getLocStart(), this->InvalidArrayInit);
}

