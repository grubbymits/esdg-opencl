/*
 * File: SuperBlockHoist.cpp
 *
 * Description:
 *  Hoists invariant instructions within a superblock out
 */

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Support/CFG.h"
#include "llvm/Analysis/LoopPass.h"
#include "LAMP/LAMPLoadProfile.h"
#include "llvm/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h" 
#include <list>

#include "../../include/SuperBlock.h"

using namespace llvm;

namespace 
{
    struct SuperBlockHoist : public LoopPass 
    {
        static char ID;

        SuperBlockHoist() : LoopPass(ID) {}

        virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
          AU.addRequired<SuperBlock>();
        }

        private:
          SuperBlock* SP;

          bool Changed;            // Set to true when we change anything.
          bool SPPreheaderSet;     // Set to true when we set the SP preheader
          BasicBlock *Preheader;   // The preheader block of the current loop...
          BasicBlock *SPPreheader; // preheader of the superblock, right below the 
                                   // loop preheader

          Loop *CurLoop;           // The current loop we are working on...

          void hoistSPInvariantInst(Instruction &I);

          bool invariantInSuperBlock(Instruction &I);

    };
}

char SuperBlockHoist::ID = 0;

static RegisterPass<SuperBlockHoist> Z("SuperBlockHoist", "hoist invariant instructions out of superblocks", false, false);

bool SuperBlockHoist::runOnLoop(Loop *L, LPPassManager &LPM) {
  Changed = false;
  SPPreheaderSet = false;

  SP = &getAnalysis<SuperBlock>();

  //prints out superblocks
  unsigned cnt = 0;
  for (map<BasicBlock*, list<BasicBlock*> >::iterator it_b = (SP->superBlocks).begin(), it_e = (SP->superBlocks).end();
       it_b != it_e; ++it_b) {
    errs() << "SuperBlock " << cnt++ << " contains: ";

    errs() << ((it_b->first))->getName() << " ";
    for (list<BasicBlock*>::iterator sp_b = (it_b->second).begin(), sp_e = (it_b->second).end();
         sp_b != sp_e; ++sp_b) {
      errs() << (*sp_b)->getName() << " ";
    }
    errs() << "\n";
  }

  CurLoop = L;
  Preheader = L->getLoopPreheader();

  //iterate through body of current loop
  for (Loop::block_iterator loopIter = L->block_begin(), E = L->block_end(); loopIter != E; ++loopIter) 
  {
    //TODO do everything inside here

    for (BasicBlock::iterator bbIter = (*loopIter)->begin(), E = (*loopIter)->end(); bbIter != E; bbIter++ )
	  {	
      Instruction &I = *bbIter;
	    if (invariantInSuperBlock(I)) 
      {
        //if instruction is part of a 
        //superblock and has superblock invariant operands, then hoist to a new
        //BB that's beneath the preheader

        hoistSPInvariantInst(I);

      }
	  }	

  }

  if(SPPreheaderSet) //if there is a superblock preheader
  {
      //fix backedges of side exits to branch to SPPreheader   
      for (pred_iterator pred_b = pred_begin(CurLoop->getHeader()), pred_e = pred_end(CurLoop->getHeader());
           pred_b != pred_e; ++pred_b) {
          //iterates through all predecessorBB of loop preheader
            
          //FIXME check part of CURRENT superblock
          //if pred BB is not part of current superblock, connect to SPPreheader 
          
          //in our case CurLoop->getHeader() happens to be the head of the superblock we want
          if (    ((SP->partOfSuperBlock).find(*pred_b) == (SP->partOfSuperBlock).end()) 
               || ((SP->partOfSuperBlock).find(*pred_b)->second != CurLoop->getHeader())) {

              //insert branches
              (*pred_b)->getTerminator()->eraseFromParent(); //erase old uncond branch from predBB to loop header
              //BranchInst *Create(BasicBlock *IfTrue, BasicBlock *InsertAtEnd)
              
              //create new uncond branch to connect to SPPreheader
              BranchInst *BI = BranchInst::Create(SPPreheader, *pred_b);
          }
      }
  }

  CurLoop = 0;
  Preheader = 0;
  SPPreheaderSet = 0;

  return Changed;
}

bool SuperBlockHoist::invariantInSuperBlock(Instruction &I)
{
    //can hoist an instruction where all it's sources are invariant inside the 
    //main trace of the superblock but have a source that is changed in the side  
    //exit 

    //first check to make sure instruction is part of a superblock

    BasicBlock* headBB = NULL;
    BasicBlock* tailBB =  NULL;
    BasicBlock* bbOfInst = I.getParent();

    if ((SP->partOfSuperBlock).find(bbOfInst) == (SP->partOfSuperBlock).end()) {
      return false;
    }

    headBB = (SP->partOfSuperBlock).find(bbOfInst)->second;

    list<BasicBlock*> trace = SP->superBlocks[headBB];

    tailBB = trace.back();

    bool backEdge = false;
    for (succ_iterator succ_b = succ_begin(tailBB), succ_e = succ_end(tailBB); succ_b != succ_e; ++succ_b) 
    {
        if (*succ_b == headBB) 
        {
	          backEdge = true;
        }
    }

    if(!backEdge)
    {
	      return false;
    }

    //if instructrion is part of a superblock, check to see whether it's 
    //invariant within superblock

    //construct full trace of superblock

    trace.push_front(headBB); //now trace contain entire superblock
    list<BasicBlock*>::iterator traceIter;

    bool breakOut = false;//check to see whether 

    //look at loads from an alloca
    if(LoadInst *loadInst = dyn_cast<LoadInst>(&I))
    {
        //check to see if the 0th operand is an alloca, if it is then it's "safe"
        if (Instruction *load_operand = dyn_cast<Instruction>(loadInst->getOperand(0)))
        {
            //if the memory location is an instruction, then its potentially an alloca
            if(AllocaInst::classof( load_operand ))
            {
                for (traceIter = trace.begin(); traceIter != trace.end(); ++traceIter) 
                {
      
                    for (BasicBlock::iterator bbIter = (*traceIter)->begin(), 
                          E = (*traceIter)->end(); bbIter != E; bbIter++ )
                    {
                        if(&I == bbIter)
                        {
                            breakOut = true;
                            break;
                        }
                        else
                        {
                            //check to make sure no stores to this mem address
                            if(StoreInst::classof(bbIter))
                            {
                                if((*bbIter).getOperand(1) == load_operand)
                                {
                                    return false;
                                }
                            }

                            //check no instructions use the result of this instruction
                            //as an operand
                            for (unsigned i = 0, e = I.getNumOperands(); i != e; ++i)
                            {
                                if (Instruction *I_operand = dyn_cast<Instruction>(I.getOperand(i)))
                                {
                                    if(I_operand == &I )
                                    {                                
                                        return false;
                                    }
                                }
                            } 
                        }    
                    }

                    if(breakOut)
                    {
                        break;
                    }

                }

                return true;
            }  
        }
        else
        {
            return false;
        }
    }  
    else //not a load
    {
      return false;
    }

    return false;
}

void SuperBlockHoist::hoistSPInvariantInst(Instruction &I)
{
    //first check to see whether a superblock preheader has already been created
    //if not, create it and set flag to true
    if (!SPPreheaderSet)
    {
        //split loop preheader, instruction in SPPreheader contains jump to
        //loop header so i can insert instructions before that
        //preheader will now contain jump to SPPreheader
        SPPreheader = SplitBlock(Preheader, Preheader->getTerminator(), this); 
        SPPreheaderSet = true;
    }

    // Move the new node to the SPPreheader, before its terminator.
    I.moveBefore(SPPreheader->getTerminator());
    Changed = true; //FIXME not sure whether this will affect correctness
} 
