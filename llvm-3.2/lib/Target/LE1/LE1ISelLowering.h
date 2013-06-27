//===-- LE1ISelLowering.h - LE1 DAG Lowering Interface --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that LE1 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LE1ISELLOWERING_H
#define LE1ISELLOWERING_H

#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"
#include "LE1.h"
#include "LE1Subtarget.h"

namespace llvm {
  namespace LE1ISD {
    enum NodeType {
      // Start the numbering from where ISD NodeType finishes.
      FIRST_NUMBER = ISD::BUILTIN_OP_END,

      // Jump and link (call)
      Call,
      LinkCall,
      Goto,
      TargetGlobal,
      TargetGlobalConst,
      BR,
      BRF,

      // Max and Min operations
      Max,
      Min,

      // Return
      Ret,

      // Pseudo return
      RetFlag,

      Exit,

      // Mov instructions
      Mov,
      MTB,
      MTBF,
      MFB,
      MTBINV,
      MFL,
      MTL,

      LoadLink,

      // Select
      SLCTF,

      // Extra Boolean Operators
      Nandl,
      Norl,
      Orl,
      Andc,
      Andl,
      Orc,

      // Group of multiplications
      Mull,
      Mullu,
      Mulh,
      Mulhu,
      Mulhs,
      Mulll,
      Mulllu,
      Mullh,
      Mullhu,
      Mulhh,
      Mulhhu,
      MUL_CHAIN,
      ADD_CHAIN,

      // Compare Ops
      Cmpeq,
      Cmpge,
      Cmpgeu,
      Cmpgt,
      Cmpgtu,
      Cmple,
      Cmpleu,
      Cmplt,
      Cmpltu,
      Cmpne,

      // MAdd/Sub nodes
      //MAdd,
      //MAddu,
      //MSub,
      //MSubu,

      // Shift ops
      Shl,
      Shr,
      Shru,

      // Shift Add ops
      Sh1Add,
      Sh2Add,
      Sh3Add,
      Sh4Add,

      // Extend in reg ops
      SXTB,
      SXTH,
      ZXTB,
      ZXTH,

      // Bit Set and Test
      Tbit,
      Tbitf,
      Sbit,
      Sbitf,

      // Extending instructions
      SextB,
      ZextB,
      SextH,
      ZextH,

      Addcg,
      Divs,

      // Load from global
      LoadGlobalU8,
      LoadGlobalU16,
      LoadGlobalS8,
      LoadGlobalS16,

      ResetLocalID,
      IncrLocalID,
      IncLocalID,
      CPUID,
      GlobalId,
      LocalSize,
      READ_GROUP_ID,
      GROUP_ID_ADDR,
      LOAD_GROUP_ID,
      READ_ATTR
      //GlobalLoadU8,
      //GlobalLoadU16,
      //GlobalLoadS8,
      //GlobalLoadS16

    };
  }

  //===--------------------------------------------------------------------===//
  // TargetLowering Implementation
  //===--------------------------------------------------------------------===//

  class LE1TargetLowering : public TargetLowering  {
  public:
    explicit LE1TargetLowering(LE1TargetMachine &TM);

    /// LowerOperation - Provide custom lowering hooks for some operations.
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    /// getTargetNodeName - This method returns the name of a target specific
    //  DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

    /// getSetCCResultType - get the ISD::SETCC result ValueType
    EVT getSetCCResultType(EVT VT) const;

    //virtual SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const;
  private:
    // Subtarget Info
    const LE1Subtarget *Subtarget;
   //FIXME can't be 64-bit 
    bool HasLE164, IsN64;

    SDValue LowerMULHS(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerMULHU(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerUDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerUREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue DivStep(SDValue DivArg1, SDValue DivArg2, SDValue DivCin,
                    SDValue AddArg, SDValue AddCin, SelectionDAG &DAG) const;
    SDValue RemStep(SDValue DivArg1, SDValue DivArg2, SDValue DivCin,
                    SDValue AddArg, SDValue AddCin, SelectionDAG &DAG) const;
    SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerIntrinsicWChain(SDValue Op, SelectionDAG &DAG) const;

    // Lower Operand helpers
    SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                            CallingConv::ID CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
                            DebugLoc dl, SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;

    // Lower Operand specifics
    //SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerDYNAMIC_STACKALLOC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerLoad(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerSELECT(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerFCOPYSIGN(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
    //SDValue LowerMEMBARRIER(SDValue Op, SelectionDAG& DAG) const;
    //SDValue LowerATOMIC_FENCE(SDValue Op, SelectionDAG& DAG) const;

    SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv, bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                      SmallVectorImpl<SDValue> &InVals) const;

    SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  DebugLoc dl, SelectionDAG &DAG) const;

    //virtual MachineBasicBlock *
      //EmitInstrWithCustomInserter(MachineInstr *MI,
        //                          MachineBasicBlock *MBB) const;

    // Inline asm support
    ConstraintType getConstraintType(const std::string &Constraint) const;

    /// Examine constraint string and operand type and determine a weight value.
    /// The operand object must already have been set up with the operand type.
    ConstraintWeight getSingleConstraintMatchWeight(
      AsmOperandInfo &info, const char *constraint) const;

    std::pair<unsigned, const TargetRegisterClass*>
              getRegForInlineAsmConstraint(const std::string &Constraint,
              EVT VT) const;

    virtual bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const;

    /// isFPImmLegal - Returns true if the target can instruction select the
    /// specified FP immediate natively. If false, the legalizer will
    /// materialize the FP immediate as a load from a constant pool.
    //virtual bool isFPImmLegal(const APFloat &Imm, EVT VT) const;

   //MachineBasicBlock *EmitAtomicBinary(MachineInstr *MI, MachineBasicBlock *BB,
     //               unsigned Size, unsigned BinOpcode, bool Nand = false) const;
    //MachineBasicBlock *EmitAtomicBinaryPartword(MachineInstr *MI,
      //              MachineBasicBlock *BB, unsigned Size, unsigned BinOpcode,
        //            bool Nand = false) const;
    //MachineBasicBlock *EmitAtomicCmpSwap(MachineInstr *MI,
      //                            MachineBasicBlock *BB, unsigned Size) const;
    //MachineBasicBlock *EmitAtomicCmpSwapPartword(MachineInstr *MI,
      //                            MachineBasicBlock *BB, unsigned Size) const;
  };
}

#endif // LE1ISELLOWERING_H
