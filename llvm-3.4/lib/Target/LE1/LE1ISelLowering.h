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
      MAXS,
      MAXU,
      MINS,
      MINU,

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
      NANDL,
      NORL,
      ORL,
      ANDC,
      ANDL,
      ORC,

      // Group of multiplications
      MULL,
      MULLU,
      MULH,
      MULHU,
      MULHS,
      MULLL,
      MULLLU,
      MULLH,
      MULLHU,
      MULHH,
      MULHHU,

      // Compare Ops
      CMPEQ,
      CMPGE,
      CMPGEU,
      CMPGT,
      CMPGTU,
      CMPLE,
      CMPLEU,
      CMPLT,
      CMPLTU,
      CMPNE,

      // Shift Add ops
      SH1ADD,
      SH2ADD,
      SH3ADD,
      SH4ADD,

      // Extend in reg ops
      SXTB,
      SXTH,
      ZXTB,
      ZXTH,

      ADDCG,
      DIVS,

      // Nodes for OpenCL
      CPUID,
      NUM_CORES,
      GlobalId,
      READ_GROUP_ID,
      GROUP_ID_ADDR,
      LOAD_GROUP_ID,
      READ_ATTR,
      SET_ATTR
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

    MVT::SimpleValueType getCmpLibcallReturnType() const;

    //virtual SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const;
  private:
    // Subtarget Info
    const LE1Subtarget *Subtarget;

    SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const;
    SDValue LowerMULHS(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerMULHU(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerMUL(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerUDIV(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerUREM(SDValue Op, SelectionDAG &DAG) const;
    SDValue DivStep(SDValue DivArg1, SDValue DivArg2, SDValue DivCin,
                    SDValue AddArg, SDValue AddCin, SelectionDAG &DAG) const;
    SDValue RemStep(SDValue DivArg1, SDValue DivArg2, SDValue DivCin,
                    SDValue AddArg, SDValue AddCin, SelectionDAG &DAG) const;
    SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSELECT(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerIntrinsicWChain(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerConstant(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

    // Lower Operand helpers
    SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                            CallingConv::ID CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
                            SDLoc dl, SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;

    // Lower Operand specifics
    //SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;

    SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv, bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           SDLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                      SmallVectorImpl<SDValue> &InVals) const;

    SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  SDLoc dl, SelectionDAG &DAG) const;
  };
}

#endif // LE1ISELLOWERING_H
