/**
 *
 * Author: Zachary Sherman (zajush), Flint Mu (flintmu), Joshua Lim (joshlzh)
 * EECS 583 Final Project
 *
 * SuperBlock.h
 * Superblock Formation
 *
 */

#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <list>
#include <set>
#include <string>
#include <vector>

using namespace std;
using namespace llvm;

// forward declaration
namespace llvm {
  //  void initializeSuperBlockPass(PassRegistry&);
} // end namespace llvm

namespace llvm {   
  class SuperBlock : public FunctionPass {
    
  public:
    static char ID;
    SuperBlock();

    ProfileInfo* PI;
    LoopInfo* LI;

    virtual void getAnalysisUsage(AnalysisUsage& AU) const;
    
    virtual bool runOnFunction(Function& F);

    // each element describes a superblock, key: head of superblock, value: subsequent basic blocks
    map<BasicBlock*, list<BasicBlock*> > superBlocks;

    // maps each BB (including head) inside a superblock to the head of that superblock
    map<BasicBlock*, BasicBlock*> partOfSuperBlock;

  private:
    // keeps track of the current code size, increases as we do BB duplication
    double currCodeSize;

    // keeps track of the original code size
    double originalCodeSize;

    // runs trace selection to identify superblocks
    void constructSuperBlocks(Function& F);
    
    // finds the "best" successor of currBB if available
    BasicBlock* bestSuccessor(BasicBlock* currBB,
                              double& cum_prob,
                              set<BasicBlock*>* unvisitedBBs,
			      set<pair<const BasicBlock*, const BasicBlock*> >* backEdges);

    // finds the "best" predecessor of currBB if available
    BasicBlock* bestPredecessor(BasicBlock* currBB,
                                double& cum_prob,
                                set<BasicBlock*>* unvisitedBBs,
			        set<pair<const BasicBlock*, const BasicBlock*> >* backEdges);

    // performs basic block duplication and fixup to remove any side entrances to a superblock
    // also, rearrange the code so that all basic blocks in a superblock are together
    void fixSideEntrances();

    // rearranges basic blocks to improve I-cache performance
    void rearrangeCodeLayout(BasicBlock* BB,
                             set<BasicBlock*>* visitedBBs,
                             set<BasicBlock*>* placedBBs,
                             map<BasicBlock*, BasicBlock*>* revSuperBlocks);
    
    // DEBUG: print the members of each superblocks
    void printsuperBlocks();

    // DEBUG: print the contents of a basic block
    void printBB(BasicBlock* BB);
  };
} // anonymous namespace

#endif
