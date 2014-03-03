#include "llvm/CodeGen/DFAPacketizer.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/ScheduleDAG.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"
#include "llvm/CodeGen/LatencyPriorityQueue.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "LE1.h"
#include "LE1TargetMachine.h"
#include "LE1RegisterInfo.h"
#include "LE1Subtarget.h"
#include "LE1MachineFunction.h"

#include <map>
//#include <iostream>

using namespace llvm;

//class DefaultVLIWScheduler;
/*
namespace llvm {
  class LE1VLIWScheduler : public ScheduleDAGInstrs {
  public:
    LE1VLIWScheduler(MachineFunction &MF, MachineLoopInfo &MLI,
                     MachineDominatorTree &MDT, bool isPostRA);
    void schedule();
  };
}*/

namespace {
  class LE1Packetizer : public MachineFunctionPass {
  public:
    static char ID;
    LE1Packetizer() : MachineFunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      AU.addRequired<MachineDominatorTree>();
      AU.addPreserved<MachineDominatorTree>();
      AU.addRequired<MachineLoopInfo>();
      AU.addPreserved<MachineLoopInfo>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    const char *getPassName() const {
      return "LE1 Packetizer";
    }

    bool runOnMachineFunction(MachineFunction &MF);
  };

  char LE1Packetizer::ID = 0;


  class LE1PacketizerList : public VLIWPacketizerList {
    unsigned MaxLatency;
    public:
      LE1PacketizerList(MachineFunction &MF, MachineLoopInfo &MLI,
                        MachineDominatorTree &MDT);
      //~LE1PacketizerList();

      //virtual void initPacketizerState(void);
      //virtual void PacketizeMIs(MachineBasicBlock *MBB,
        //                        MachineBasicBlock::iterator BeginItr,
          //                      MachineBasicBlock::iterator EndItr);
      virtual bool ignorePseudoInstruction(MachineInstr *I,
                                           MachineBasicBlock *MBB);
      virtual bool isSoloInstruction(MachineInstr *MI);
      virtual bool isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ);
      //virtual bool needsNops();
      //virtual void checkLatencies(MachineBasicBlock *MBB);
      /*MachineBasicBlock::instr_iterator insertNops(MachineBasicBlock *MBB,
                                   MachineBasicBlock::instr_iterator I,
                                    unsigned NumNops);*/
      //void checkDeps(MachineInstr *MI, MachineInstr *MJ);
      //virtual bool isLegalToPruneDependencies(SUnit *SUI, SUnit *SUJ);

  };

}
/*
bool LE1PacketizerList::needsNops() {
  return MF.getTarget().getSubtarget<LE1Subtarget>().needsNops();
}*/

static bool canReserveResources(const llvm::MCInstrDesc *MID) {
  unsigned InsnClass = MID->getSchedClass();
  const llvm::InstrStage *IS = InstrItins->beginStage(InsnClass);
  unsigned FuncUnits = IS->getUnits();
}
static bool canReserveResources(llvm::MachineInstr *MI) {
  const llvm::MCInstrDesc &MID = MI->getDesc();
  return canReserveResources(&MID);
}

void LE1Packetizer::PacketizeMIs(MachineBasicBlock *MBB,
                                      MachineBasicBlock::iterator BeginItr,
                                      MachineBasicBlock::iterator EndItr) {
  assert(VLIWScheduler && "VLIW Scheduler is not initialized!");
  VLIWScheduler->startBlock(MBB);
  VLIWScheduler->enterRegion(MBB, BeginItr, EndItr,
                             std::distance(BeginItr, EndItr));
  VLIWScheduler->schedule();

  // Generate MI -> SU map.
  MIToSUnit.clear();
  for (unsigned i = 0, e = VLIWScheduler->SUnits.size(); i != e; ++i) {
    SUnit *SU = &VLIWScheduler->SUnits[i];
    MIToSUnit[SU->getInstr()] = SU;
  }

  // The main packetizer loop.
  for (; BeginItr != EndItr; ++BeginItr) {
    MachineInstr *MI = BeginItr;

    //this->initPacketizerState();

    // End the current packet if needed.
    if (this->isSoloInstruction(MI)) {
      endPacket(MBB, MI);
      continue;
    }

    // Ignore pseudo instructions.
    if (this->ignorePseudoInstruction(MI, MBB))
      continue;

    SUnit *SUI = MIToSUnit[MI];
    assert(SUI && "Missing SUnit Info!");

    // Ask DFA if machine resource is available for MI.
    bool ResourceAvail = ResourceTracker->canReserveResources(MI);
    if (ResourceAvail) {
      // Dependency check for MI with instructions in CurrentPacketMIs.
      for (std::vector<MachineInstr*>::iterator VI = CurrentPacketMIs.begin(),
           VE = CurrentPacketMIs.end(); VI != VE; ++VI) {
        MachineInstr *MJ = *VI;
        SUnit *SUJ = MIToSUnit[MJ];
        assert(SUJ && "Missing SUnit Info!");

        // Is it legal to packetize SUI and SUJ together.
        if (!this->isLegalToPacketizeTogether(SUI, SUJ)) {
          // Allow packetization if dependency can be pruned.
          if (!this->isLegalToPruneDependencies(SUI, SUJ)) {
            // End the packet if dependency cannot be pruned.
            endPacket(MBB, MI);
            break;
          } // !isLegalToPruneDependencies.
        } // !isLegalToPacketizeTogether.
      } // For all instructions in CurrentPacketMIs.
    } else {
      // End the packet if resource is not available.
      endPacket(MBB, MI);
    }

    // Add MI to the current packet.
    BeginItr = this->addToPacket(MI);
  } // For all instructions in BB.

  // End any packet left behind.
  endPacket(MBB, EndItr);
  VLIWScheduler->exitRegion();
  VLIWScheduler->finishBlock();
}

bool LE1Packetizer::runOnMachineFunction(MachineFunction &MF) {
  const TargetInstrInfo *TII = MF.getTarget().getInstrInfo();
  MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  MachineDominatorTree &MDT = getAnalysis<MachineDominatorTree>();

  // Instantiate the packetizer
  LE1PacketizerList Packetizer(MF, MLI, MDT);
  assert(Packetizer.getResourceTracker() && "Empty DFA Table");

  // Loop over all basic blocks and remove KILL pseudo-instructions
  for(MachineFunction::iterator MBBb = MF.begin(), MBBe = MF.end();
      MBBb != MBBe; ++MBBb) {
    MachineBasicBlock::iterator End = MBBb->end();
    MachineBasicBlock::iterator MI = MBBb->begin();
    while (MI != End) {
      if(MI->isKill()) {
        MachineBasicBlock::iterator DeleteMI = MI;
        ++MI;
        MBBb->erase(DeleteMI);
        End = MBBb->end();
        continue;
      }
      ++MI;
    }
  }

  for(MachineFunction::iterator MBBb = MF.begin(), MBBe = MF.end();
      MBBb != MBBe; ++MBBb) {

    // Find scheduling regions and packetize each of them
    unsigned RemainingCount = MBBb->size();
    for(MachineBasicBlock::iterator RegionEnd = MBBb->end();
        RegionEnd != MBBb->begin();) {
      MachineBasicBlock::iterator I = RegionEnd;
      for(;I != MBBb->begin(); --I, --RemainingCount) {
        if(TII->isSchedulingBoundary(llvm::prior(I), MBBb, MF))
          break;
      }
      I = MBBb->begin();

      // Skip empty scheduling regions
      if(I == RegionEnd) {
        RegionEnd = llvm::prior(RegionEnd);
        --RemainingCount;
        continue;
      }
      // Skip regions with one instruction
      if(I == llvm::prior(RegionEnd)) {
        RegionEnd = llvm::prior(RegionEnd);
        continue;
      }

      // if(CurrentPacketMIs.size() <
      //    ResourceTracker->getInstrItins()->Props.IssueWidth)
      Packetizer.PacketizeMIs(MBBb, I, RegionEnd);
      RegionEnd = I;
    }
  }

  return true;
}

/*
static bool doesModifyCalleeSavedRegs(MachineInstr *MI,
                                      const TargetRegisterInfo *TRI) {
  for(const uint16_t *CSR = TRI->getCalleeSavedRegs(); *CSR; ++CSR) {
    unsigned CalleeSavedReg = *CSR;
    if(MI->modifiesRegister(CalleeSavedReg, TRI))
      return true;
  }
  return false;
}

static bool doesCallModifySavedRegs(MachineInstr *MI,
                                    const TargetRegisterInfo *TRI) {
  //if(MI->isCall()) {
    // get the destination function of the call
    //MachineOperand *MO = MI->getOperand(2);
  //}

  return false;
}*/

static bool isControlFlow(MachineInstr *MI) {
  //if((MI->isCall()) || (MI->isBranch()) || (MI->isReturn()))
  return (MI->getDesc().isTerminator() || MI->getDesc().isCall());
    //return true;
  //else
    //return false;
}

static bool isDirectJump(MachineInstr *MI) {
  if(MI->getOpcode() == LE1::GOTO)
    return true;
  else
    return false;
}
/*
LE1VLIWScheduler::LE1VLIWScheduler(MachineFunction &MF, MachineLoopInfo &MLI,
                                   MachineDominatorTree &MDT, bool isPostRA) :
  ScheduleDAGInstrs(MF, MLI, MDT, IsPostRA) {
    std::cout << "LE1VLIW Constr\n";
    CanHandleTerminators = true;
}

void LE1VLIWScheduler::schedule() {
  // Build the scheduling graph
  std::cout << "VLIW schedule\n";
  buildSchedGraph(0);
}
*/
LE1PacketizerList::LE1PacketizerList(MachineFunction &MF,
                                     MachineLoopInfo &MLI,
                                     MachineDominatorTree &MDT)
  : VLIWPacketizerList(MF, MLI, MDT, true) {
  //VLIWScheduler = new LE1VLIWScheduler(MF, MLI, MDT, true);
}
/*
LE1PacketizerList::~LE1PacketizerList() {
  if(VLIWScheduler)
    delete VLIWScheduler;
  if(ResourceTracker)
    delete ResourceTracker;
}*/

bool LE1PacketizerList::
ignorePseudoInstruction(MachineInstr *I, MachineBasicBlock *MBB) {
  DEBUG(dbgs() << "ignorePseduoInstruction? " << I->getOpcode() << "\n");
  //if(I->isPseudo()) {
    //DEBUG(dbgs() << "\nisPseudo " << I->getOpcode());
    //return true;
  //}
  if(I->isDebugValue()) {
    //DEBUG(dbgs() << "\nisDebugValue " << I->getOpcode());
    return true;
  }
  //if(I->isLabel()) {
    //DEBUG(dbgs() << "\nisLabel " << I->getOpcode());
    //return true;
  //}

  const MCInstrDesc& TID = I->getDesc();
  unsigned SchedClass = TID.getSchedClass();
  const InstrStage* IS = 
                    ResourceTracker->getInstrItins()->beginStage(SchedClass);
  unsigned FuncUnits = IS->getUnits();
  if(!FuncUnits)
    DEBUG(dbgs() << "\nhas no func units " << I->getOpcode());
  return !FuncUnits;
}

bool LE1PacketizerList::
isSoloInstruction(MachineInstr *MI) {
  /*if(MI->getOpcode() == LE1::SUBi)
    if(MI->getOperand(1).getReg() == LE1::SP) {
      DEBUG(dbgs() << "\nAdjusting stack pointer so solo");
      return true;
    }
  if(MI->getOpcode() == LE1::STW)
   if(MI->getOperand(0).isReg())
     if(MI->getOperand(0).getReg() == LE1::L0) {
       DEBUG(dbgs() << "\nLink register being stored so solo");
       return true;
      }
  if(MI->getOpcode() == LE1::STW)
    return true;*/
  if(MI->isEHLabel()) {
    //DEBUG(dbgs() << "\nMI is a EHLabel, so solo"); 
    return true;
  }
  if(MI->getOpcode() == LE1::CLK)
    return true;

  return false;
}

// SUJ is the instruction currently in the packet, SUI is
// the next instruction to be packetized. SUnits are nodes
// of ScheduleDAG.
bool LE1PacketizerList::
isLegalToPacketizeTogether(SUnit *SUI, SUnit *SUJ) {

  // First check whether the maximum issue width has been met.
  if(CurrentPacketMIs.size() >= TM.getSubtarget<LE1Subtarget>().getIssueWidth())
    return false;

  //DEBUG(dbgs() << "\nisLegalToPacketize?");
  MachineInstr *I = SUI->getInstr();
  MachineInstr *J = SUJ->getInstr();
  assert(I && J && "Unable to packetize null instructions");

  const MCInstrDesc &MCIDJ = J->getDesc();
  bool FoundSequentialDependence = false;

  // Labels are not bundled
  if(I->isLabel())
    return false;

  // Effectively endPacket once a call has been inserted
  if(J->getOpcode() == LE1::CALL ||
     J->getOpcode() == LE1::LNKCALL)
    return false;

  // Only allowed one control-flow instruction. There's only one functional
  // unit to handle control-flow so this shouldn't be ever be true
  if(isControlFlow(I) && isControlFlow(J)) {
    //Dependence = true;
    DEBUG(dbgs() << "\nDependence = true, already a control-flow instr in packet");
    return false;
  }

  // Check for dependences: SDep::Kind Data, Anti, Output and Order
  if (SUJ->isSucc(SUI)) {
    for(unsigned i=0;
        i < SUJ->Succs.size() && !FoundSequentialDependence; ++i) {

      if(SUJ->Succs[i].getSUnit() != SUI)
        continue;

      SDep::Kind DepType = SUJ->Succs[i].getKind();

      // Data (true dependences) are not allowed
      if(DepType == SDep::Data) {
        // Check if the immediates are used, then the dependence can be removed
        // TODO make this more general to handle other instances
        /*if(J->getOpcode() == LE1::SUBi) {
          if(I->getOpcode() == LE1::STW) {
            if(J->getOperand(0).getReg() == LE1::SP) {
              int64_t stackDecrement = J->getOperand(2).getImm();
              int64_t offset = I->getOperand(2).getImm();
              I->getOperand(2).setImm(offset-stackDecrement);
            }
          }
        }
        else {*/
          //Dependence = true;
          DEBUG(dbgs() << "\nDependence is true. " << I->getOpcode() << " and "
                << J->getOpcode() << " have data dependences");
          return false;
        }
      //}

      // Ignore the order of unconditional jumps/calls and non control-flow
      // instructions
      else if(isDirectJump(I) &&
              !MCIDJ.isBranch() &&
              !MCIDJ.isCall() &&
              (DepType == SDep::Order)) {
         DEBUG(dbgs() << "\nOrder dependence between " << I->getOpcode()
               << " and " << J->getOpcode());
     }

      else if(DepType == SDep::Order)
        DEBUG(dbgs() << "\nOrder dependence between " << I->getOpcode()
              << " and " << J->getOpcode());

      // Anti (WAR) can be ignored since operations within a
      // bundle are treated as atomic.
      else if(DepType == SDep::Anti) {
        DEBUG(dbgs() << "\nAnti-dependence between " << I->getOpcode()
              << " and " << J->getOpcode());
      }

      // Output (WAW) select register allocator to rename..?
      else if(DepType == SDep::Output)
        DEBUG(dbgs() << "\nOutput dependence between " << I->getOpcode()
              << " and " << J->getOpcode());
    }
  }

  return true;
}


static const char* getName(MachineInstr *MI) {
  /*
  switch(MI->getOpcode()) {
  default:                        return "PSEUDO";
  case TargetOpcode::BUNDLE:    return "BUNDLE";
  case llvm::LE1::ADD:             return "ADD";
  case llvm::LE1::ADDCG:           return "ADDCG";
  case llvm::LE1::ADDi:            return "ADDi";
  case llvm::LE1::AND:             return "AND";
  case llvm::LE1::ANDC:            return "ANDC";
  case llvm::LE1::ANDCi:           return "ANDCi";
  case llvm::LE1::ANDL:            return "ANDL";
  case llvm::LE1::ANDi:            return "ANDi";
  case llvm::LE1::BR:              return "BR";
  case llvm::LE1::BRF:             return "BRF";
  case llvm::LE1::CALL:            return "CALL";
  case llvm::LE1::CALLPOINTER:     return "CALLPOINTER";
  case llvm::LE1::CLK:             return "CLK";
  case llvm::LE1::CMPEQ:
  case llvm::LE1::CMPEQi:
  case llvm::LE1::CMPEQr:
  case llvm::LE1::CMPEQri:         return "CMPEQ";
  case llvm::LE1::CMPGEU:
  case llvm::LE1::CMPGEUi:
  case llvm::LE1::CMPGEUr:
  case llvm::LE1::CMPGEUri:        return "CMPGEU";
  case llvm::LE1::CMPGE:
  case llvm::LE1::CMPGEi:
  case llvm::LE1::CMPGEr:
  case llvm::LE1::CMPGEri:         return "CMPGE";
  case llvm::LE1::CMPGTU:
  case llvm::LE1::CMPGTUi:
  case llvm::LE1::CMPGTUr:
  case llvm::LE1::CMPGTUri:        return "CMPGTU";
  case llvm::LE1::CMPGT:
  case llvm::LE1::CMPGTi:
  case llvm::LE1::CMPGTr:
  case llvm::LE1::CMPGTri:         return "CMPGT";
  case llvm::LE1::CMPLEU:
  case llvm::LE1::CMPLEUi:
  case llvm::LE1::CMPLEUr:
  case llvm::LE1::CMPLEUri:        return "CMPLEU";
  case llvm::LE1::CMPLE: 
  case llvm::LE1::CMPLEi:
  case llvm::LE1::CMPLEr:
  case llvm::LE1::CMPLEri:         return "CMPLE";
  case llvm::LE1::CMPLTU:
  case llvm::LE1::CMPLTUi:
  case llvm::LE1::CMPLTUr:
  case llvm::LE1::CMPLTUri:        return "CMPLTU";
  case llvm::LE1::CMPLT:
  case llvm::LE1::CMPLTi:
  case llvm::LE1::CMPLTr:
  case llvm::LE1::CMPLTri:         return "CMPLT";
  case llvm::LE1::CMPNE:
  case llvm::LE1::CMPNEi:
  case llvm::LE1::CMPNEr:
  case llvm::LE1::CMPNEri:         return "CMPNE";
  case llvm::LE1::DIVS:            return "DIVS";
  case llvm::LE1::Exit:            return "EXIT";
  case llvm::LE1::GOTO:            return "GOTO";
  case llvm::LE1::LDB:             return "LDB";
  case llvm::LE1::LDB_G:           return "LDB_G";
  case llvm::LE1::LDBu:            return "LDBu";
  case llvm::LE1::LDBu_G:          return "LDBu_G";
  case llvm::LE1::LDH:             return "LDH";
  case llvm::LE1::LDH_G:           return "LDH_G";
  case llvm::LE1::LDHu:            return "LDHu";
  case llvm::LE1::LDHu_G:          return "LDHu_G";
  case llvm::LE1::LDL:             return "LDL";
  case llvm::LE1::LDW:             return "LDW";
  case llvm::LE1::LDW_G:           return "LDW_G";
  case llvm::LE1::MAX:             return "MAX";
  case llvm::LE1::MAXU:
  case llvm::LE1::MAXUi:           return "MAXU";
  case llvm::LE1::MAXi:            return "MAX";
  case llvm::LE1::MFB:             return "MFB";
  case llvm::LE1::MFL:             return "MFL";
  case llvm::LE1::MIN:             return "MIN";
  case llvm::LE1::MINU:
  case llvm::LE1::MINUi:           return "MINU";
  case llvm::LE1::MINi:            return "MIN";
  case llvm::LE1::MOVg:            return "MOVg";
  case llvm::LE1::MOVi:
  case llvm::LE1::MOVr:            return "MOVr";
  case llvm::LE1::MTB:             return "MTB";
  case llvm::LE1::MTL:             return "MTL";
  case llvm::LE1::MTLg:            return "MTLg";
  */
  /*
  case llvm::LE1::MULHi:
  case llvm::LE1::MULH:            return "MULH";
  case llvm::LE1::MULHH:
  case llvm::LE1::MULHHi:          return "MULHH";
  case llvm::LE1::MULLHi:
  case llvm::LE1::MULLH:           return "MULLH";
  case llvm::LE1::MULHHU:
  case llvm::LE1::MULHHUi:         return "MULHHU";
  case llvm::LE1::MULHS:
  case llvm::LE1::MULHSi:         return "MULHS";
  case llvm::LE1::MULHU:
  case llvm::LE1::MULHUi:          return "MULHU";
  case llvm::LE1::MULL:
  case llvm::LE1::MULLi:           return "MULL";
  case llvm::LE1::MULLHU:
  case llvm::LE1::MULLHUi:         return "MULLHU";
  case llvm::LE1::MULLL:
  case llvm::LE1::MULLLi:          return "MULLL";
  case llvm::LE1::MULLLU:
  case llvm::LE1::MULLLUi:         return "MULLLU";
  case llvm::LE1::MULLU:
  case llvm::LE1::MULLUi:          return "MULLU";*/
    /*
  case llvm::LE1::NANDL:           return "NANDL";
  case llvm::LE1::NORL:            return "NORL";
  case llvm::LE1::ORC:
  case llvm::LE1::ORCi:            return "ORC";
  case llvm::LE1::ORL:             return "ORL";
  case llvm::LE1::OR:
  case llvm::LE1::ORi:             return "OR";
  case llvm::LE1::Ret:             return "RET";
  case llvm::LE1::RetFlag:         return "RETFLAG";
  case llvm::LE1::SBIT:            return "SBIT";
  case llvm::LE1::SBITF:           return "SBITF";
  case llvm::LE1::SH1ADD:
  case llvm::LE1::SH1ADDi:         return "SH1ADD";
  case llvm::LE1::SH2ADD:
  case llvm::LE1::SH2ADDi:         return "SH2ADD";
  case llvm::LE1::SH3ADD:
  case llvm::LE1::SH3ADDi:         return "SH3ADD";
  case llvm::LE1::SH4ADD:
  case llvm::LE1::SH4ADDi:         return "SH4ADD";
  case llvm::LE1::SHL:
  case llvm::LE1::SHLi:            return "SHL";
  case llvm::LE1::SHRU:
  case llvm::LE1::SHRUi:           return "SHRU";
  case llvm::LE1::SHR:
  case llvm::LE1::SHRi:            return "SHR";
  case llvm::LE1::SLCTFri:
  case llvm::LE1::SLCTFrr:         return "SLCTF";
  case llvm::LE1::SLCTri:
  case llvm::LE1::SLCTrr:          return "SLCT";
  case llvm::LE1::STB:             return "STB";
  case llvm::LE1::STB_G:           return "STB_G";
  case llvm::LE1::STH:             return "STH";
  case llvm::LE1::STH_G:           return "STH_G";
  case llvm::LE1::STL:             return "STL";
  case llvm::LE1::STW:             return "STW";
  case llvm::LE1::STW_G:           return "STW_G";
  case llvm::LE1::SUB:
  case llvm::LE1::SUBi:            return "SUB";
  case llvm::LE1::SXTB:            return "SXTB";
  case llvm::LE1::SXTH:            return "SXTH";
  case llvm::LE1::TBIT:            return "TBIT";
  case llvm::LE1::TBITF:           return "TBITF";
  case llvm::LE1::XORi:
  case llvm::LE1::XOR:             return "XOR";
  case llvm::LE1::ZXTB:            return "ZXTB";
  case llvm::LE1::ZXTH:            return "ZXTH";
  }
  */
}
/*
MachineBasicBlock::instr_iterator
LE1PacketizerList::insertNops(MachineBasicBlock *MBB,
                                   MachineBasicBlock::instr_iterator I,
                                    unsigned NumNops) {
  DebugLoc dl;
  ++I;
  for(unsigned i = 0; i < NumNops; ++i) {
    // Create a clock instruction, ensure that its 'bundled'.
    DEBUG(dbgs() << "\nInserting clock");
    DEBUG(dbgs() << "\nIterator pointing at " << getName(I));
    BuildMI(*MBB, I, dl, TII->get(LE1::CLK));
    //++I;
  }
  DEBUG(dbgs() << "\n----BasicBlock " << MBB->getFullName()
        << " currently looks like this:");
  DEBUG(dbgs() << "\nStarting at " << getName(MBB->instr_begin()));
  DEBUG(dbgs() << " , and ending at " << getName(MBB->instr_end()));
  for(MachineBasicBlock::instr_iterator i = MBB->instr_begin(),
      e = MBB->instr_end(); i != e; ++i) {
    DEBUG(dbgs() << "\n   " << i->getOpcode() << " - " << getName(i));
    if(i->isBundled())
      DEBUG(dbgs() << " - is bundled");
    else
      DEBUG(dbgs() << " - is not bundled");
    }

  return I;
}

void LE1PacketizerList::checkDeps(MachineInstr* MI, MachineInstr* MJ) {
  DEBUG(dbgs() << "\nChecking for dependencies between " << getName(MI)
        << " and " << getName(MJ));
  SUnit *SUI = MIToSUnit[MI];
  SUnit *SUJ = MIToSUnit[MJ];
  if(SUI->isSucc(SUJ)) {
    DEBUG(dbgs() << ", it is a successor");
    for(unsigned i = 0; i < SUI->Succs.size(); ++i) {

      if(SUI->Succs[i].getSUnit() != SUJ)
        continue;

      SDep::Kind DepType = SUI->Succs[i].getKind();

      if(DepType == SDep::Data)
        DEBUG(dbgs() << "\nThere is a data dependency");
      //else if(DepType == SDep::Order)
        //DEBUG(dbgs() << "\nThere is an order dependency");
      else
        continue;
      // TODO must be a better way to do this
      unsigned Latency = (MJ->getOpcode() == LE1::Ret) ? 1 : 2;
      DEBUG(dbgs() << "\nLatency = " << Latency);
      DEBUG(dbgs() << "\nMaxLatency = " << MaxLatency);
      if(MaxLatency < Latency) {
        MaxLatency = Latency;
        DEBUG(dbgs() << "\nSetting new MaxLatency to " << MaxLatency);
      }
    }
  }
}

void LE1PacketizerList::checkLatencies(MachineBasicBlock *MBB) {

  MachineBasicBlock::instr_iterator II = MBB->instr_begin();
  MachineBasicBlock::instr_iterator End = MBB->instr_end();
  MachineInstr &Back = MBB->back();

  DEBUG(dbgs() << "\n\nChecking Latencies in " << MBB->getFullName());
  //for(MachineBasicBlock::iterator BI = MBB->begin(), BE = MBB->end();
    //  BI != BE; ++BI) {
  DEBUG(dbgs() << "\nBeginning with " << getName(II));
  DEBUG(dbgs() << " and ending with " << getName(End));
  DEBUG(dbgs() << "\nBack = " << getName(&Back));

  unsigned BundleSize = 0;
  while(II != Back &&  II != MBB->end()) {
    DEBUG(dbgs() << "\nNow checking from " << getName(II));
    MaxLatency = 0;
    if(II->getOpcode() == TargetOpcode::BUNDLE) {
      BundleSize = II->getBundleSize();
      DEBUG(dbgs() << " , it is the bundle header, the size of which is "
            << II->getBundleSize());
    }

    if(!ignorePseudoInstruction(II, MBB)) {

      // Check from a solo instruction
      if(!II->isBundled()) {
        DEBUG(dbgs() << " , it is not bundled");
        MachineBasicBlock::instr_iterator NextII = II;
        ++NextII;
        // Skip 'bundle and clock' ops
        while(ignorePseudoInstruction(NextII, MBB)) {
          if(NextII != Back)
            ++NextII;
          else break;
        }

          if(!NextII->isBundled()) {
            DEBUG(dbgs() << "\nNeither is " << getName(NextII));
            // Check dependencies between the two solo ops
            checkDeps(II, NextII);
          }
          else {
            // Check against the top level MI of the next bundle
            DEBUG(dbgs() << "\nInstructions to check against are bundled");
            checkDeps(II, NextII);
            ++NextII;
            // Then iterate through the rest of the bundle
            while(NextII->isInsideBundle()) {
              checkDeps(II, NextII);
              ++NextII;
            }
          }
        //++II;
        this->insertNops(MBB, II, MaxLatency);
        BundleSize = 0;
      }
      // Check from bundled instructions
      else {
        DEBUG(dbgs() << " , it is bundled");
        MachineBasicBlock::instr_iterator OrigII = II;
        MachineBasicBlock::instr_iterator NextII = II;
        MachineBasicBlock::instr_iterator TopII;
        MachineBasicBlock::instr_iterator BundleHeader = NULL;
        MachineBasicBlock::instr_iterator FinalInst = NULL;
        bool IsBundled = false;
        bool LastBundle = false;

        // First, look ahead to find the first instruction that isn't part of
        // this bundle
        while(NextII->isInsideBundle()) {
          DEBUG(dbgs() << "\nLooking forward to " << getName(NextII));
          if(NextII != Back && NextII != MBB->end())
            ++NextII;
          else {
            DEBUG(dbgs() << "\nBreaking out from looking for next bundle");
            LastBundle = true;
            break;
          }
        }

        if(!LastBundle) {
          // Then skip past clock and bundle ops
          while(ignorePseudoInstruction(NextII, MBB)) {
            DEBUG(dbgs() << "\nSkipping past clock and bundle ops");
            // Check until we reach the end of the block
            if(NextII != Back && NextII != MBB->end()) {
              DEBUG(dbgs() << "\nLooking forward to " << getName(NextII));
              if(NextII->getOpcode() == TargetOpcode::BUNDLE) {
                BundleHeader = NextII;
                IsBundled = true;
                DEBUG(dbgs() << "\nIsBundle set");
              }
              ++NextII;
            }
            else {
              DEBUG(dbgs() << "\nReached the end of the bb");
              break;
            }
          }
          if( NextII != MBB->end()) {
            // Keep a pointer to the first proper inst of the bundle
            TopII = NextII;
            FinalInst = II;
            while(II->isInsideBundle()) {
              DEBUG(dbgs() << "\nStill inside bundle, op = " << getName(II));
              // check against the first op in the bundle
              checkDeps(II, NextII);

              if(NextII->isBundled() &&
                NextII != MBB->end()) {
                DEBUG(dbgs() << "\nNext is bundled");
                // Need to check against all the insts in the next bundle
                ++NextII;
                while(NextII->isInsideBundle()) {
                  checkDeps(II, NextII);
                  ++NextII;
                }
              }
              FinalInst = II;
              NextII = TopII;
              ++II;
            }
            insertNops(MBB, FinalInst, MaxLatency);
            II = FinalInst;
          }
        }
      }
    }
    DEBUG(dbgs() << "\nAt the end of the loop, II = " << getName(II));
    ++II;
  }
}
*/
/*
            // Check against the first inst, it may or may not be bundled
            checkDeps(II, TopII);

            // If it is part of a bundle, iterate through it
            if(TopII->isBundled()) {
              IsBundled = true;
              ++NextII;

              while(NextII->isInsideBundle()) {
                checkDeps(II, NextII);
                ++NextII;
              }
              // Reset the pointer
              NextII = TopII;
            }

            DEBUG(dbgs() << "\nIncrementing II in the inner loop");
            FinalInst = II;
            ++II;
            ++InstCounter;
          }
          DEBUG(dbgs() << "\nAfter looping, top is pointing at "
                << getName(TopII));
          // Once we've checked all the insts against the next ones, insert
          // NOPs if needed
          //if(IsBundled) {
            //DEBUG(dbgs() << "\nBundleHeader pointing at " << getName(BundleHeader));
            //insertNops(MBB, BundleHeader, MaxLatency);
          //}
          //else
            insertNops(MBB, FinalInst, MaxLatency);
          BundleSize = 0;
      }
    }   // ignorePseudo(II, MBB);
      else
        DEBUG(dbgs() << " , it is a pseudo");
      DEBUG(dbgs() << "\nIncrementing II at end of main loop");
      DEBUG(dbgs() << " it is currently pointing at " << getName(II));
    ++II;
  }     // while(II != Back)
}*/


// Public Constructor Function
FunctionPass *llvm::createLE1Packetizer(LE1TargetMachine &TM) {
  return new LE1Packetizer();
}
