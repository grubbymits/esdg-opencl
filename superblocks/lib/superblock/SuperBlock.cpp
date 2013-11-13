/**
 *
 * Author: Zachary Sherman (zajush), Flint Mu (flintmu), Joshua Lim (joshlzh)
 * EECS 583 Final Project
 *
 * SuperBlock.cpp
 * Superblock Formation
 *
 */

#include <list>
#include <set>
#include <string>
#include <vector>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/BasicBlock.h"
#include "llvm/Constants.h"
#include "llvm/DataLayout.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
//#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "../../include/SuperBlock.h"

using namespace std;
using namespace llvm;

// data collection
STATISTIC(OriginalCodeSize,  "Original code size (# Lines)");
STATISTIC(FinalCodeSize,     "Final code size (# Lines)");

// command line arguments
static cl::opt<double>
PROB("prob", cl::init(0.8),
     cl::desc("The threshold probability used to identify traces"));

static cl::opt<bool>
CUMULATIVE_PROB("cumulative-prob", cl::init(false),
                cl::desc("If true, will take cumulative probability into consideration during trace formation"));

static cl::opt<double>
CODE_EXPANSION_THRESHOLD("code-expansion-threshold", cl::init(2),
                         cl::desc("The threshold value is the maximum times of the original code size to allow after doing expansion, so the initial value of 2 means we allow the code size after expansion to be at most 2 times of the original code size"));


char SuperBlock::ID = 0;
static RegisterPass<SuperBlock> X("superblock", "Superblock Formation");

SuperBlock::SuperBlock() : FunctionPass(ID) {
  currCodeSize = 0;
  originalCodeSize = 0;
}

void SuperBlock::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<ProfileInfo>();
  AU.addRequired<LoopInfo>();
}

bool SuperBlock::runOnFunction(Function& F) {
  if (F.isDeclaration()) {
    return false;
  }

  PI = &getAnalysis<ProfileInfo>();
  LI = &getAnalysis<LoopInfo>();

  // first, find the list of superblocks
  constructSuperBlocks(F);

  // then, remove side entrances by duplicating basic blocks
  fixSideEntrances();

  // each basic block is mapped to the tail of the superblock if it belongs to a superblock
  // (for code rearrangement purposes)
  map<BasicBlock*, BasicBlock*>* revSuperBlocks =
    new map<BasicBlock*, BasicBlock*>();
  for (map<BasicBlock*, list<BasicBlock*> >::iterator sp = superBlocks.begin(), sp_e = superBlocks.end();
       sp != sp_e; ++sp) {
    revSuperBlocks->insert(pair<BasicBlock*, BasicBlock*>(sp->first, sp->second.back()));
    for (list<BasicBlock*>::iterator it = sp->second.begin(), it_e = --sp->second.end(); it != it_e; ++it) {
      revSuperBlocks->insert(pair<BasicBlock*, BasicBlock*>(*it, sp->second.back()));
    }
  }

  // rearrange code layout to improve I-cache performance
  set<BasicBlock*>* visitedBBs = new set<BasicBlock*>();     // keeps track of which BBs that are visited
  set<BasicBlock*>* placedBBs = new set<BasicBlock*>();      // keeps track of which BBs have already been placed in code
  placedBBs->insert(F.begin());
  rearrangeCodeLayout(F.begin(), visitedBBs, placedBBs, revSuperBlocks);

  printsuperBlocks();

  delete revSuperBlocks;
  delete visitedBBs;
  delete placedBBs;

  // collect statistics
  OriginalCodeSize = (unsigned) originalCodeSize;
  FinalCodeSize = (unsigned) currCodeSize;

  // the CFG is modified, hence this true statement
  return true;
}

BasicBlock* SuperBlock::bestSuccessor(BasicBlock* currBB,
                                      double& cum_prob,
                                      set<BasicBlock*>* unvisitedBBs,
				      set<pair<const BasicBlock*, const BasicBlock*> >* backEdges) {
  double maxEdgeWeight = 0.0;
  double totalExFreq = PI->getExecutionCount(currBB);
  BasicBlock* bestSuccBB = NULL;

  for (succ_iterator succ_b = succ_begin(currBB), succ_e = succ_end(currBB);
       succ_b != succ_e; ++succ_b) {
    //iterates through all successorBB of currBB
    double curEdgeWeight = PI->getEdgeWeight(PI->getEdge(currBB, *succ_b));
    if (curEdgeWeight > maxEdgeWeight) {
      maxEdgeWeight = curEdgeWeight;
      bestSuccBB = *succ_b;
    }
  }

  // no best successor is found if edge weight < threshold
  double cur_prob = maxEdgeWeight/totalExFreq;
  if (cur_prob < PROB) {
    return NULL;
  }

  // if this edge is found in backEdge set, then this edge is a backedge
  pair<const BasicBlock*, const BasicBlock*> tmpBackEdgeCheck(currBB, bestSuccBB);
  if (backEdges->find(tmpBackEdgeCheck) != backEdges->end()) {
    return NULL;
  }

  // check if succBB belongs in same loop, if neither is in loop, condition will fail so it's ok
  if (LI->getLoopFor(currBB) != LI->getLoopFor(bestSuccBB)) {
    return NULL;
  }

  // we don't want to return a previously visited node
  if (unvisitedBBs->find(bestSuccBB) == unvisitedBBs->end()) {
    return NULL;
  }

  // cumulative probability check
  if (CUMULATIVE_PROB && cum_prob*cur_prob < PROB) {
    return NULL;
  }

  cum_prob *= cur_prob;
  return bestSuccBB;
}

BasicBlock* SuperBlock::bestPredecessor(BasicBlock* currBB,
                                        double& cum_prob,
                                        set<BasicBlock*>* unvisitedBBs,
				        set<pair<const BasicBlock*, const BasicBlock*> >* backEdges) {
  double maxEdgeWeight = 0.0;
  double totalExFreq = PI->getExecutionCount(currBB);
  BasicBlock* bestPredBB = NULL;

  for (pred_iterator pred_b = pred_begin(currBB), pred_e = pred_end(currBB);
       pred_b != pred_e; ++pred_b) {
    //iterates through all predecessorBB of currBB
    double curEdgeWeight = PI->getEdgeWeight(PI->getEdge(*pred_b, currBB));
    if (curEdgeWeight > maxEdgeWeight) {
      maxEdgeWeight = curEdgeWeight;
      bestPredBB = *pred_b;
    }
  }

  // no best predecessor is found if edge weight < threshold
  double cur_prob = maxEdgeWeight/totalExFreq;
  if (cur_prob < PROB) {
    return NULL;
  }

  // if this edge is found in backEdge set, then this edge is a backedge
  pair<const BasicBlock*, const BasicBlock*> tmpBackEdgeCheck(bestPredBB, currBB);
  if (backEdges->find(tmpBackEdgeCheck) != backEdges->end()) {
    return NULL;
  }

  if (LI->getLoopFor(currBB) != LI->getLoopFor(bestPredBB)) {
    return NULL;
  }

  // we don't want to get any previously visited node
  if (unvisitedBBs->find(bestPredBB) == unvisitedBBs->end()) {
    return NULL;
  }

  // cumulative probability check
  if (CUMULATIVE_PROB && cum_prob*cur_prob < PROB) {
    return NULL;
  }

  cum_prob *= cur_prob;
  return bestPredBB;
}

void SuperBlock::constructSuperBlocks(Function& F) {
  superBlocks.clear();

  // key: execution count, value: unvisited BBs
  map<double, list<BasicBlock*> >* execBBs = 
    new map<double, list<BasicBlock*> >();

  // stores the set of unvisited BBs
  set<BasicBlock*>* unvisitedBBs =
    new set<BasicBlock*>();

  // stores the set of backedges described as (from, to) pair
  set<pair<const BasicBlock*, const BasicBlock*> >* backEdges =
    new set<pair<const BasicBlock*, const BasicBlock*> >();

  // retrieve all backedges contained in this current function
  SmallVector<pair<const BasicBlock*, const BasicBlock*>, 32> backEdgesVector; //(F.size());

  // transfer everything from backEdgesVector to backEdges
  backEdges->insert(backEdgesVector.begin(), backEdgesVector.end());

  // insert all the BBs in this function into set of unvisitedBBs
  for (Function::iterator funcIter = F.begin(), funcIterEnd = F.end();
       funcIter != funcIterEnd; ++funcIter) {
    // gets number of instructions in this BB
    originalCodeSize += funcIter->size();
    currCodeSize += funcIter->size();

    double execCount = PI->getExecutionCount(funcIter);
    map<double, list<BasicBlock*> >::iterator it = execBBs->find(execCount);
    if (it == execBBs->end()) {
      list<BasicBlock*> arg(1, funcIter);
      execBBs->insert(pair<double, list<BasicBlock*> >(execCount, arg));
    } else {
      it->second.push_back(funcIter);
    }

    unvisitedBBs->insert(funcIter);
  }
  
  // traverse each BB in descending order of their execution counts
  for (map<double, list<BasicBlock*> >::reverse_iterator arg_b = execBBs->rbegin(), arg_e = execBBs->rend();
       arg_b != arg_e; ++arg_b) {
    for (list<BasicBlock*>::iterator cur_b = arg_b->second.begin(), cur_e = arg_b->second.end();
         cur_b != cur_e; ++cur_b) {
      if (unvisitedBBs->find(*cur_b) == unvisitedBBs->end()) {
        continue;
      }

      // use this current BB as seed and remove from set
      unvisitedBBs->erase(*cur_b);

      list<BasicBlock*> trace;
      list<BasicBlock*>::iterator traceIter;

      trace.push_back(*cur_b);
      
      // run trace selection algorithm
      BasicBlock* currBB = *cur_b;
      BasicBlock* nextBB;
      double cum_prob = 1;
      
      // grow trace forward
      while (1) {
        nextBB = bestSuccessor(currBB, cum_prob, unvisitedBBs, backEdges);
        if (nextBB == NULL) {
          break;
        }

        trace.push_back(nextBB);
        unvisitedBBs->erase(nextBB);
        currBB = nextBB;
      }

      // start creating backwards from current BB again
      currBB = *cur_b;
      
      // grow trace backwards
      while (1) {
        nextBB = bestPredecessor(currBB, cum_prob, unvisitedBBs, backEdges);
        if (nextBB == NULL) {
          break;
        }

        trace.push_front(nextBB);
        unvisitedBBs->erase(nextBB);
        currBB = nextBB;
      }
      
      // record our newly found superblock
      // we don't record superblocks of size 1
      if (trace.front() != trace.back()) {
        BasicBlock* head = trace.front();
        trace.pop_front();
        superBlocks[head] = trace;

        // populate partOfSuperBlock map
        partOfSuperBlock[head] = head;
        for (traceIter = trace.begin(); traceIter != trace.end(); ++traceIter) {        
          partOfSuperBlock[*traceIter] = head;
        }
      }
    }
  }

  delete execBBs;
  delete unvisitedBBs;
  delete backEdges;
}

void SuperBlock::fixSideEntrances() {
  // due to merging of BBs, some superblocks may have 1 BB remaining
  list<map<BasicBlock*, list<BasicBlock*> >::iterator > delSuperBlocks;

  for (map<BasicBlock*, list<BasicBlock*> >::iterator sp = superBlocks.begin(), sp_e = superBlocks.end();
       sp != sp_e; ++sp) {
    // we need to keep track of the predecessor of the current basic block being checked
    BasicBlock* prev = sp->first;

    // don't clone basic blocks if the code size threshold is achieved
    if (currCodeSize/originalCodeSize > CODE_EXPANSION_THRESHOLD) {
      break;
    }

    // the first basic block for a superblock need not be duplicated
    for (list<BasicBlock*>::iterator bb = sp->second.begin(), bb_e = sp->second.end();
         bb != bb_e; ++bb) {
      // first, collect all predecessors for this BB
      // (note: we could not just iterate through as the predecessor set may change
      list<BasicBlock*> predBBs;
      for (pred_iterator pred = pred_begin(*bb), pred_e = pred_end(*bb);
           pred != pred_e; ++pred) {
        predBBs.push_back(*pred);
      }

      // now, walk through all predecessors of this current basic block
      BasicBlock* clonedBB = NULL;
      for (list<BasicBlock*>::iterator pred = predBBs.begin(), pred_e = predBBs.end();
           pred != pred_e; ++pred) {
        // if it is not the predecessor of this current basic block present in the superblock,
        // duplicate!
        if (*pred != prev) {
          // there is no need to clone this BB multiple times
          if (clonedBB == NULL) {
            ValueToValueMapTy vmap;

            // clone this basic block, and place the corresponding code after the last BB of this superblock
            clonedBB = CloneBasicBlock(*bb, vmap, ".cloned", (*bb)->getParent());
            vmap[*bb] = clonedBB;

            /*
            errs() << "@@ BEFORE: " << *clonedBB << "\n";
            // fix phi nodes in the cloned BB
            for (BasicBlock::iterator I = clonedBB->begin(); isa<PHINode>(I); ++I) {
              PHINode* PN = dyn_cast<PHINode>(I);
              int bbIdx = PN->getBasicBlockIndex(prev);
              if (bbIdx != -1) {
                PN->removeIncomingValue(bbIdx, false);
              }
            }
            */

            // add size of duplicated BBs to total code size count
            currCodeSize += clonedBB->size();

            // modify operands in this basic block
            for (BasicBlock::iterator instr = clonedBB->begin(), instr_e = clonedBB->end();
                 instr != instr_e; ++instr) {
              for (unsigned idx = 0, num_ops = instr->getNumOperands(); idx < num_ops; ++idx) {
                Value* op = instr->getOperand(idx);
                ValueToValueMapTy::iterator op_it = vmap.find(op);
                if (op_it != vmap.end()) {
                  instr->setOperand(idx, op_it->second);
                }
              }
            }
          }

          // remove phi nodes into this BB in the trace
          /*
          for (BasicBlock::iterator I = (*bb)->begin(); isa<PHINode>(I); ++I) {
            PHINode* PN = dyn_cast<PHINode>(I);
            int bbIdx = PN->getBasicBlockIndex(*pred);
            if (bbIdx != -1) {
              PN->removeIncomingValue(bbIdx, false);
            }
          }
          */

          // modify the branch instruction of the predecessor not in the superblock to
          // branch to the cloned basic block
          Instruction* br_instr = (*pred)->getTerminator();
          br_instr->replaceUsesOfWith((Value*)*bb, (Value*)clonedBB);
        }
      }

      // determine if we can merge the BB (definitely can be merged), and its clone
      // with theirpredecessors
      if (clonedBB != NULL) {
        if (MergeBlockIntoPredecessor(*bb, this)) {
          // since we have merged this BB, delete from our superblock mappings
          partOfSuperBlock.erase(*bb);
          bb = sp->second.erase(bb);
          --bb;

          if (sp->second.empty()) {
            delSuperBlocks.push_back(sp);
          }
        }

        MergeBlockIntoPredecessor(clonedBB, this);
      }

      prev = *bb;
    }
  }

  // erase some superblocks (which only have 1 BB remaining)
  for (list<map<BasicBlock*, list<BasicBlock*> >::iterator >::iterator del = delSuperBlocks.begin(),
         del_e = delSuperBlocks.end(); del != del_e; ++del) {
    superBlocks.erase(*del);
  }
}

void SuperBlock::rearrangeCodeLayout(BasicBlock* curBB,
                                     set<BasicBlock*>* visitedBBs,
                                     set<BasicBlock*>* placedBBs,
                                     map<BasicBlock*, BasicBlock*>* revSuperBlocks) {
  // don't pursue a Basic Block that has already been visited
  if (visitedBBs->find(curBB) != visitedBBs->end()) {
    return;
  }

  visitedBBs->insert(curBB);

  // key: execution count, value: list of basic blocks
  map<double, list<BasicBlock*> >* execBBs =
    new map<double, list<BasicBlock*> >();

  // sort the execution counts of the successors in descending order
  for (succ_iterator succ = succ_begin(curBB), succ_e = succ_end(curBB);
       succ != succ_e; ++succ) {
    double execCount = PI->getExecutionCount(*succ);
    map<double, list<BasicBlock*> >::iterator it = execBBs->find(execCount);
    if (it == execBBs->end()) {
      list<BasicBlock*> arg(1, *succ);
      execBBs->insert(pair<double, list<BasicBlock*> >(execCount, arg));
    } else {
      it->second.push_back(*succ);
    }
  }

  // iterate through the successors in descending order of execution counts
  for (map<double, list<BasicBlock*> >::reverse_iterator arg = execBBs->rbegin(), arg_e = execBBs->rend();
       arg != arg_e; ++arg) {
    for (list<BasicBlock*>::iterator BB = arg->second.begin(), BB_e = arg->second.end();
         BB != BB_e; ++BB) {
      // place this BB after curBB (if it has not already been placed before)
      if (placedBBs->find(*BB) == placedBBs->end()) {
        // checks to see if curBB belongs to a superblock: we don't want to shove
        // BB in the middle of this superblock
        BasicBlock* placeholder;
        map<BasicBlock*, BasicBlock*>::iterator it = revSuperBlocks->find(curBB);
        if (it == revSuperBlocks->end()) {
          placeholder = curBB;
        } else {
          placeholder = it->second;
        }

        (*BB)->moveAfter(placeholder);
        placedBBs->insert(*BB);
      }

      // if this BB is the head of a superblock, place the tail of the superblock together
      map<BasicBlock*, list<BasicBlock*> >::iterator it = superBlocks.find(*BB);
      if (it != superBlocks.end()) {
        BasicBlock* cur = *BB;
        for (list<BasicBlock*>::iterator spBB = it->second.begin(), spBB_e = it->second.end();
             spBB != spBB_e; ++spBB) {
          (*spBB)->moveAfter(cur);
          placedBBs->insert(*spBB);

          cur = *spBB;
        }
      }

      // recursively place the successor BB
      rearrangeCodeLayout(*BB, visitedBBs, placedBBs, revSuperBlocks);
    }
  }

  delete execBBs;
}

void SuperBlock::printBB(BasicBlock* BB) {
  errs() << "Contents of BB: " << BB->getName() << "\n" << *BB << "\n";
}

void SuperBlock::printsuperBlocks() {
  unsigned cnt = 0;
  for (map<BasicBlock*, list<BasicBlock*> >::iterator it_b = superBlocks.begin(), it_e = superBlocks.end();
       it_b != it_e; ++it_b) {
    errs() << "SuperBlock " << cnt++ << " contains: ";

    errs() << ((it_b->first))->getName() << " ";
    for (list<BasicBlock*>::iterator sp_b = (it_b->second).begin(), sp_e = (it_b->second).end();
         sp_b != sp_e; ++sp_b) {
      errs() << (*sp_b)->getName() << " ";
    }
    errs() << "\n";
  }
}
