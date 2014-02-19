//===-- LE1ISelLowering.cpp - LE1 DAG Lowering Implementation -----------===//
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

#define DEBUG_TYPE "le1-lower"
#include "LE1ISelLowering.h"
#include "LE1MachineFunction.h"
#include "LE1TargetMachine.h"
//#include "LE1TargetObjectFile.h"
#include "LE1Subtarget.h"
#include "InstPrinter/LE1InstPrinter.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetLibraryInfo.h"

//#include <iostream>

using namespace llvm;

// If I is a shifted mask, set the size (Size) and the first bit of the 
// mask (Pos), and return true.
// For example, if I is 0x003ff800, (Pos, Size) = (11, 11).  
//static bool IsShiftedMask(uint64_t I, uint64_t &Pos, uint64_t &Size) {
  //if (!isUInt<32>(I) || !isShiftedMask_32(I))
    // return false;

  //Size = CountPopulation_32(I);
  //Pos = CountTrailingZeros_32(I);
  //return true;
//}

const char *LE1TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case LE1ISD::Call:          return "LE1ISD::Call";
  case LE1ISD::Goto:          return "LE1ISD::Goto";
  case LE1ISD::BRF:           return "LE1ISD::BRF";
  case LE1ISD::BR:            return "LE1ISD::BR";
  case LE1ISD::Ret:           return "LE1ISD::Ret";
  case LE1ISD::RetFlag:       return "LE1ISD::RetFlag";
  case LE1ISD::Exit:          return "LE1ISD::Exit";

  case LE1ISD::Mov:           return "LE1ISD::Mov";
  case LE1ISD::MTB:           return "LE1ISD::MTB";
  case LE1ISD::MFB:           return "LE1ISD::MFB";
  case LE1ISD::MTBINV:        return "LE1ISD::MTBINV";

  case LE1ISD::NANDL:         return "LE1ISD::NANDL";
  case LE1ISD::NORL:          return "LE1ISD::NORL";
  case LE1ISD::ORL:           return "LE1ISD::ORL";
  case LE1ISD::ANDC:          return "LE1ISD::ANDC";
  case LE1ISD::ANDL:          return "LE1ISD::ANDL";
  case LE1ISD::ORC:           return "LE1ISD::ORC";

  case LE1ISD::MULL:          return "LE1ISD::MULL";
  case LE1ISD::MULH:          return "LE1ISD::MULH";
  case LE1ISD::MULHS:         return "LE1ISD::MULHS";
  case LE1ISD::MULLL:         return "LE1ISD::MULLL";
  case LE1ISD::MULLH:         return "LE1ISD::MULLH";
  case LE1ISD::MULHH:         return "LE1ISD::MULHH";
  case LE1ISD::Addcg:         return "LE1ISD::Addcg";
  case LE1ISD::Divs:          return "LE1ISD::Divs";

  case LE1ISD::CMPEQ:         return "LE1ISD::CMPEQ";
  case LE1ISD::CMPGE:         return "LE1ISD::CMPGE";
  case LE1ISD::CMPGEU:        return "LE1ISD::CMPGEU";
  case LE1ISD::CMPGT:         return "LE1ISD::CMPGT";
  case LE1ISD::CMPGTU:        return "LE1ISD::CMPGTU";
  case LE1ISD::CMPLE:         return "LE1ISD::CMPLE";
  case LE1ISD::CMPLEU:        return "LE1ISD::CMPLEU";
  case LE1ISD::CMPLT:         return "LE1ISD::CMPLT";
  case LE1ISD::CMPLTU:        return "LE1ISD::CMPLTU";
  case LE1ISD::CMPNE:         return "LE1ISD::CMPNE";

  case LE1ISD::MAXS:           return "LE1ISD::MAXS";
  case LE1ISD::MAXU:           return "LE1ISD::MAXU";
  case LE1ISD::MINS:           return "LE1ISD::MINS";
  case LE1ISD::MINU:           return "LE1ISD::MINU";

  case LE1ISD::SXTB:          return "LE1ISD::SXTB";
  case LE1ISD::SXTH:          return "LE1ISD::SXTH";
  case LE1ISD::ZXTB:          return "LE1ISD::ZXTB";
  case LE1ISD::ZXTH:          return "LE1ISD::ZXTH";

  case LE1ISD::SH1ADD:        return "LE1ISD::SH1ADD";
  case LE1ISD::SH2ADD:        return "LE1ISD::SH2ADD";
  case LE1ISD::SH3ADD:        return "LE1ISD::SH3ADD";
  case LE1ISD::SH4ADD:        return "LE1ISD::SH4ADD";

  case LE1ISD::TBIT:          return "LE1ISD::TBIT";
  case LE1ISD::TBITF:         return "LE1ISD::TBITF";
  case LE1ISD::SBIT:          return "LE1ISD::SBIT";
  case LE1ISD::SBITF:         return "LE1ISD::SBITF";

  case LE1ISD::CPUID:         return "LE1ISD::CPUID";
  case LE1ISD::LocalSize:     return "LE1ISD::LocalSize";
  case LE1ISD::GlobalId:      return "LE1ISD::GlobalId";
  case LE1ISD::READ_GROUP_ID: return "LE1ISD::READ_GROUP_ID";
  case LE1ISD::GROUP_ID_ADDR: return "LE1ISD::GROUP_ID_ADDR";
  case LE1ISD::LOAD_GROUP_ID: return "LE1ISD::LOAD_GROUP_ID";
  case LE1ISD::READ_ATTR:     return "LE1ISD::READ_ATTR";
  case LE1ISD::SET_ATTR:      return "LE1ISD::SET_ATTR";

  default:                         return NULL;
  }
}

LE1TargetLowering::
LE1TargetLowering(LE1TargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileELF()),
    Subtarget(&TM.getSubtarget<LE1Subtarget>()) {

  // Set up the register classes
  addRegisterClass(MVT::i32, &LE1::CPURegsRegClass);
  addRegisterClass(MVT::i1, &LE1::BRegsRegClass);

  computeRegisterProperties();
  
  setSelectIsExpensive(false);
  setIntDivIsCheap(false);
  setJumpIsExpensive(true);
  setSchedulingPreference(Sched::ILP);

  // Basically try to completely remove calls to these library functions
  MaxStoresPerMemset = 100;
  MaxStoresPerMemsetOptSize = 100;
  MaxStoresPerMemmove = 100;
  MaxStoresPerMemmoveOptSize = 100;
  MaxStoresPerMemcpy = 100;
  MaxStoresPerMemcpyOptSize = 100;

  // Load extended operations for i1 types must be promoted
  setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);

  /*
  setCondCodeAction(ISD::SETUGT,  MVT::i1,  Expand);
  setCondCodeAction(ISD::SETUGE,  MVT::i1,  Expand);
  setCondCodeAction(ISD::SETULT,  MVT::i1,  Expand);
  setCondCodeAction(ISD::SETULE,  MVT::i1,  Expand);
  setCondCodeAction(ISD::SETGT,   MVT::i1,  Expand);
  setCondCodeAction(ISD::SETGE,   MVT::i1,  Expand);
  setCondCodeAction(ISD::SETLT,   MVT::i1,  Expand);
  setCondCodeAction(ISD::SETLE,   MVT::i1,  Expand);
  setCondCodeAction(ISD::SETEQ,   MVT::i1,  Expand);
  setCondCodeAction(ISD::SETNE,   MVT::i1,  Legal);*/

  // Handle Vectors Comparisons
  // FIXME This screws with legalisation of boolean SetCC? Bug?
  for (unsigned i = (unsigned)MVT::FIRST_VECTOR_VALUETYPE;
       i <= (unsigned)MVT::LAST_VECTOR_VALUETYPE; ++i) {
    MVT VT((MVT::SimpleValueType)i);
    setCondCodeAction(ISD::SETUGT,  VT, Expand);
    setCondCodeAction(ISD::SETUGE,  VT, Expand);
    setCondCodeAction(ISD::SETULT,  VT, Expand);
    setCondCodeAction(ISD::SETULE,  VT, Expand);
    setCondCodeAction(ISD::SETGT,   VT, Expand);
    setCondCodeAction(ISD::SETGE,   VT, Expand);
    setCondCodeAction(ISD::SETLT,   VT, Expand);
    setCondCodeAction(ISD::SETLE,   VT, Expand);
    setCondCodeAction(ISD::SETEQ,   VT, Expand);
    setCondCodeAction(ISD::SETNE,   VT, Expand);
    setCondCodeAction(ISD::SETOEQ,  VT, Expand);
    setCondCodeAction(ISD::SETUEQ,  VT, Expand);
    setCondCodeAction(ISD::SETOGE,  VT, Expand);
    setCondCodeAction(ISD::SETOGT,  VT, Expand);
    setCondCodeAction(ISD::SETOLT,  VT, Expand);
    setCondCodeAction(ISD::SETOLE,  VT, Expand);
    setOperationAction(ISD::SETCC,   VT, Expand);
  }

  // LE1 doesn't have extending float->double load/store
  //setLoadExtAction(ISD::EXTLOAD, MVT::f32, Expand);
  //setTruncStoreAction(MVT::f64, MVT::f32, Expand);

  // LE1 Custom Operations
  setOperationAction(ISD::SDIV,               MVT::i32,   Custom);
  setOperationAction(ISD::UDIV,               MVT::i32,   Custom);
  setOperationAction(ISD::SREM,               MVT::i32,   Custom);
  setOperationAction(ISD::UREM,               MVT::i32,   Custom);
  setOperationAction(ISD::MULHS,              MVT::i32,   Custom);
  setOperationAction(ISD::MULHU,              MVT::i32,   Custom);
  setOperationAction(ISD::MUL,                MVT::i32,   Custom);
  setOperationAction(ISD::SETCC,              MVT::i32,   Custom);
  setOperationAction(ISD::SELECT_CC,          MVT::i1,    Custom);
  setOperationAction(ISD::SELECT_CC,          MVT::i32,   Custom);
  setOperationAction(ISD::BRCOND,             MVT::Other, Custom);
  setOperationAction(ISD::INTRINSIC_VOID,     MVT::Other, Custom);
  setOperationAction(ISD::INTRINSIC_W_CHAIN,  MVT::Other, Custom);
  setOperationAction(ISD::INTRINSIC_WO_CHAIN, MVT::Other, Custom);

    /*
  // Softfloat Floating Point Library Calls
  // Integer to Float conversions
  setLibcallName(RTLIB::SINTTOFP_I32_F32, "int32_to_float32");
  //setLibcallName(RTLIB::SINTTOFP_I32_F32, "_r_ilfloat");
  setOperationAction(ISD::SINT_TO_FP, MVT::i32, Expand);

  // FIXME - Is this ok? Is sign detected..?
  setLibcallName(RTLIB::UINTTOFP_I32_F32, "int32_to_float32");
  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);

  setLibcallName(RTLIB::SINTTOFP_I32_F64, "int32_to_float64");
  //setLibcallName(RTLIB::SINTTOFP_I32_F64, "_d_ilfloat");
  setOperationAction(ISD::SINT_TO_FP, MVT::i32, Expand);

  //setLibcallName(RTLIB::UINTTOFP_I32_F64, "_d_ufloat");
  //setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);

  //Software IEC/IEEE single-precision conversion routines.
  setLibcallName(RTLIB::FPTOSINT_F32_I32, "float32_to_int32");
  setOperationAction(ISD::FP_TO_SINT, MVT::f32, Expand);

  //FIXME
  //float32_to_int32_round_to_zero

  setLibcallName(RTLIB::FPEXT_F32_F64, "float32_to_float64");
  //setLibcallName(RTLIB::FPEXT_F32_F64, "_d_r");
  setOperationAction(ISD::FP_EXTEND, MVT::f32, Expand);

  //Software IEC/IEEE single-precision operations.
  // FIXME are these roundings correct? There is NEARBYINT_F too..
  setLibcallName(RTLIB::RINT_F32, "float32_round_to_int");
  setOperationAction(ISD::FRINT , MVT::f32, Expand);

  setLibcallName(RTLIB::ADD_F32, "float32_add");
  //setLibcallName(RTLIB::ADD_F32, "_r_add");
  setOperationAction(ISD::FADD, MVT::f32, Expand);

  setLibcallName(RTLIB::SUB_F32, "float32_sub");
  //setLibcallName(RTLIB::SUB_F32, "_r_sub");
  setOperationAction(ISD::FSUB, MVT::f32, Expand);




  setLibcallName(RTLIB::REM_F32, "float32_rem");
  setOperationAction(ISD::SREM, MVT::f32, Expand);
  //setLibcallName(RTLIB::UREM_F32, "float32_rem");
  setOperationAction(ISD::UREM, MVT::f32, Expand);

  setLibcallName(RTLIB::SQRT_F32, "float32_sqrt");

  setLibcallName(RTLIB::OEQ_F32, "float32_eq");
  //setLibcallName(RTLIB::OEQ_F32, "_r_eq");
  setOperationAction(ISD::SETOEQ, MVT::f32, Expand);

  setLibcallName(RTLIB::OLE_F32, "float32_le");
  //setLibcallName(RTLIB::OLE_F32, "_r_le");
  setOperationAction(ISD::SETOLE, MVT::f32, Expand);

  setLibcallName(RTLIB::OLT_F32, "float32_lt");
  //setLibcallName(RTLIB::OLT_F32, "_r_lt");
  setOperationAction(ISD::SETOLT, MVT::f32, Expand);

  // TODO I think that the functions for gt and lt are the same accept for
  // their NaN handling.
  setLibcallName(RTLIB::OGT_F32, "float32_lt");
  setOperationAction(ISD::SETOGT, MVT::f32, Expand);

  setLibcallName(RTLIB::UO_F32,  "float32_is_signaling_nan");

  // FIXME
  //float32_eq_signaling
  //float32_le_quiet
  //float32_lt_quiet
  //float32_is_signaling_nan

  //Software IEC/IEEE double-precision conversion routines.
  setLibcallName(RTLIB::FPTOSINT_F64_I32, "float64_to_int32");
  setOperationAction(ISD::FP_TO_SINT, MVT::f64, Expand);

  // FIXME
  //float64_to_int32_round_to_zero
 
  setLibcallName(RTLIB::FPROUND_F64_F32, "float64_to_float32");
  setOperationAction(ISD::FP_ROUND, MVT::f64, Expand);

  //Software IEC/IEEE double-precision operations.
  // FIXME are these roundings correct? There is NEARBYINT_F too..
  setLibcallName(RTLIB::RINT_F64, "float64_round_to_int");
  setOperationAction(ISD::FRINT, MVT::f64, Expand);

  setLibcallName(RTLIB::ADD_F64, "float64_add");
  //setLibcallName(RTLIB::ADD_F64, "_d_add");
  setOperationAction(ISD::FADD, MVT::f64, Expand);

  setLibcallName(RTLIB::SUB_F64, "float64_sub");
  //setLibcallName(RTLIB::SUB_F64, "_d_sub");
  setOperationAction(ISD::FSUB, MVT::f64, Expand);

  setLibcallName(RTLIB::MUL_F64, "float64_mul");
  //setLibcallName(RTLIB::MUL_F64, "_d_mul");
  setOperationAction(ISD::FMUL, MVT::f64, Expand);

  setLibcallName(RTLIB::DIV_F64, "float64_div");
  //setLibcallName(RTLIB::DIV_F64, "_d_div");
  setOperationAction(ISD::FDIV, MVT::f64, Expand);

  setLibcallName(RTLIB::REM_F64, "float64_rem");
  setOperationAction(ISD::SREM, MVT::f64, Expand);
  //setLibcallName(RTLIB::UREM_F64, "float64_rem");
  setOperationAction(ISD::UREM, MVT::f64, Expand);

  setLibcallName(RTLIB::SQRT_F64, "float64_sqrt");
  setOperationAction(ISD::FSQRT  , MVT::f64, Expand);

  setLibcallName(RTLIB::OEQ_F64, "float64_eq");
  //setLibcallName(RTLIB::OEQ_F64, "_d_eq");
  setOperationAction(ISD::SETOEQ, MVT::f64, Expand);

  setLibcallName(RTLIB::OLE_F64, "float64_le");
  //setLibcallName(RTLIB::OLE_F64, "_d_le");
  setOperationAction(ISD::SETOLE, MVT::f32, Expand);

  setLibcallName(RTLIB::OLT_F64, "float64_lt");
  //setLibcallName(RTLIB::OLT_F64, "_d_lt");
  setOperationAction(ISD::SETOLT, MVT::f64, Expand);

  setOperationAction(LibFunc::exp2,   MVT::f32, Expand);
  setOperationAction(LibFunc::exp2f,  MVT::f32, Expand);
  setOperationAction(LibFunc::exp2,   MVT::f64, Expand);
  setOperationAction(LibFunc::exp2f,  MVT::f64, Expand);
  */
  setLibcallName(RTLIB::ADD_F32, "float32_add");
  setOperationAction(ISD::FADD, MVT::f32, Expand);

  setLibcallName(RTLIB::MUL_F32, "float32_mul");
  setOperationAction(ISD::FMUL, MVT::f32, Expand);
  setLibcallName(RTLIB::DIV_F32, "float32_div");
  setOperationAction(ISD::FDIV, MVT::f32, Expand);

  // FIXME
  //float64_eq_signaling
  //float64_le_quiet
  //float64_lt_quiet
  //float64_is_signaling_nan

  setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
  setOperationAction(ISD::GlobalAddress,      MVT::i16,   Custom);
  setOperationAction(ISD::GlobalAddress,      MVT::i8,    Custom);

  setOperationAction(ISD::VASTART,            MVT::Other, Custom);

  // FIXME This should use Addcg and be custom
  setOperationAction(ISD::ADDE,             MVT::i32, Expand);
  setOperationAction(ISD::SUBE,             MVT::i32, Expand);

  // Operations not directly supported by LE1.
  setOperationAction(ISD::SDIV,             MVT::i64, Expand);
  setOperationAction(ISD::UDIV,             MVT::i64, Expand);
  //setOperationAction(ISD::SREM,             MVT::i32, Expand);
  //setOperationAction(ISD::UREM,             MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM,          MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM,          MVT::i32, Expand);
  setOperationAction(ISD::SREM,             MVT::i64, Expand);
  setOperationAction(ISD::UREM,             MVT::i64, Expand);

  setOperationAction(ISD::ROTL,             MVT::i32, Expand);
  setOperationAction(ISD::ROTR,             MVT::i32, Expand);
  setOperationAction(ISD::ROTL,             MVT::i64, Expand);
  setOperationAction(ISD::ROTR,             MVT::i64, Expand);

  setOperationAction(ISD::SMUL_LOHI,        MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI,        MVT::i32, Expand);

  setOperationAction(ISD::BR_JT,            MVT::Other, Expand);
  setOperationAction(ISD::BR_CC,            MVT::Other, Expand);
  setOperationAction(ISD::BR_CC,            MVT::i32,  Expand);
  setOperationAction(ISD::BRIND,            MVT::Other, Expand);
  //setOperationAction(ISD::SELECT_CC,         MVT::Other, Expand);
  setOperationAction(ISD::BSWAP,            MVT::i32, Expand);
  setOperationAction(ISD::CTPOP,            MVT::i32, Expand);
  setOperationAction(ISD::CTTZ,             MVT::i32, Expand);
  setOperationAction(ISD::CTLZ,             MVT::i32, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF,  MVT::i32, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF,  MVT::i32, Expand);

  setOperationAction(ISD::SHL_PARTS,        MVT::i32,   Expand);
  setOperationAction(ISD::SRA_PARTS,        MVT::i32,   Expand);
  setOperationAction(ISD::SRL_PARTS,        MVT::i32,   Expand);
  setOperationAction(ISD::SIGN_EXTEND,      MVT::i32,   Expand);
  setOperationAction(ISD::ZERO_EXTEND,      MVT::i32,   Expand);

  //setOperationAction(ISD::EXCEPTIONADDR,     MVT::i32, Expand);
  //setOperationAction(ISD::EHSELECTION,       MVT::i32, Expand);

  setOperationAction(ISD::VAARG,             MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,            MVT::Other, Expand);
  setOperationAction(ISD::VAEND,             MVT::Other, Expand);

  // Use the default for now
  setOperationAction(ISD::STACKSAVE,         MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE,      MVT::Other, Expand);

  //setOperationAction(ISD::MEMBARRIER,        MVT::Other, Custom);
  //setOperationAction(ISD::Constant,            MVT::i1, Promote); 
  setOperationAction(ISD::SIGN_EXTEND_INREG,  MVT::i1, Expand); 
  setOperationAction(ISD::ANY_EXTEND,         MVT::i1, Expand);
  //setOperationAction(ISD::TRUNCATE,           MVT::i1, Promote);

  setOperationAction(ISD::AND,                MVT::i1, Promote);
  setOperationAction(ISD::OR,                 MVT::i1, Promote);
  setOperationAction(ISD::ADD,                MVT::i1, Promote);
  setOperationAction(ISD::SUB,                MVT::i1, Promote);
  setOperationAction(ISD::XOR,                MVT::i1, Promote);
  setOperationAction(ISD::SHL,                MVT::i1, Promote);
  setOperationAction(ISD::SRA,                MVT::i1, Promote);
  setOperationAction(ISD::SRL,                MVT::i1, Promote);
  setOperationAction(ISD::SELECT_CC,          MVT::i1, Promote);

  setTargetDAGCombine(ISD::ADD);
  setTargetDAGCombine(ISD::AND);
  setTargetDAGCombine(ISD::OR);
  setTargetDAGCombine(ISD::MUL);
  setTargetDAGCombine(ISD::SHL);
  setTargetDAGCombine(ISD::SELECT_CC);


  setOperationAction(ISD::FSQRT, MVT::f32, Expand);

  // FIXME don't know what this should be
  setMinFunctionAlignment(2);

  setStackPointerRegisterToSaveRestore(LE1::SP);
  computeRegisterProperties();

  //setExceptionPointerRegister(LE1::A0);
  //setExceptionSelectorRegister(LE1::A1);
}

static inline bool isNeg1(SDValue Op) {
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Op))
    return (CSN->getSExtValue() == -1);
  return false;
}

static inline bool isPos1(SDValue Op) {
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Op))
    return (CSN->isOne());
  return false;
}

static inline bool isZero(SDValue Op) {
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Op))
    return (CSN->getZExtValue() == 0);
  return false;
}

static inline bool isImm16(SDValue Imm) {
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Imm)) {
    unsigned value = CSN->getZExtValue();
    if (value == (unsigned short)value)
      return (value == 16);
  }
  return false;
}

static inline bool isSHL_16(SDValue Op) {
  if (Op.getOpcode() == ISD::SHL)
    return (isImm16(Op.getOperand(1)));
  return false;
}

static inline bool isSRL_16(SDValue Op) {
  if (Op.getOpcode() == ISD::SRL)
    return (isImm16(Op.getOperand(1)));
  return false;
}

static inline bool isSRA_16(SDValue Op) {
  if (Op.getOpcode() == ISD::SRA)
    return (isImm16(Op.getOperand(1)));
  return false;
}

static inline bool isSEXT_16(SDValue Op) {
  if (Op.getOpcode() == ISD::SIGN_EXTEND_INREG)
    return (Op.getOperand(1).getValueType() == MVT::i16);
  return false;
}

static inline bool is16BitSImm(SDValue Op) {
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Op)) {
    int value = CSN->getSExtValue();
    return (value == (value & 0xFF));
  }
  return false;
}

static inline bool is16BitUImm(SDValue Op) {
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Op)) {
    unsigned value = CSN->getZExtValue();
    return (value == (value & 0xFF));
  }
  return false;
}

static bool is16BitMask(SDValue Op) {
  if (Op.getOpcode() == ISD::AND) {
    if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(Op.getOperand(1)))
      return (CSN->getZExtValue() == 0xFF);
  }
  return false;
}

SDValue static PerformADDCombine(SDNode *N, SelectionDAG &DAG) {
  DEBUG(dbgs() << "PerformADDCombine\n");
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  SDLoc dl(N);

  if ((LHS.getOpcode() != ISD::SHL) && (RHS.getOpcode() != ISD::SHL))
    return SDValue(N, 0);

  uint64_t ShiftVal = 0;
  if (LHS.getOpcode() == ISD::SHL) {
    if (ConstantSDNode *CN = cast<ConstantSDNode>(LHS.getOperand(1)))
      ShiftVal = CN->getConstantOperandVal(0);
  }
  else if (RHS.getOpcode() == ISD::SHL) {
    if (ConstantSDNode *CN = cast<ConstantSDNode>(RHS.getOperand(1)))
      ShiftVal = CN->getConstantOperandVal(0);
  }
  unsigned Opcode = 0;
  switch (ShiftVal) {
  default:
    return SDValue(N, 0);
  case 1:
    Opcode = LE1ISD::SH1ADD;
    break;
  case 2:
    Opcode = LE1ISD::SH2ADD;
    break;
  case 3:
    Opcode = LE1ISD::SH3ADD;
    break;
  case 4:
    Opcode = LE1ISD::SH4ADD;
    break;
  }
  return DAG.getNode(Opcode, dl, MVT::i32, LHS, RHS);
}

SDValue static PerformANDCombine(SDNode *N, SelectionDAG &DAG) {
  DEBUG(dbgs() << "PerformANDCombine\n");
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  // ANDC
  if (RHS.getOpcode() == ISD::XOR)
    if (isNeg1(RHS.getOperand(1)))
      return DAG.getNode(LE1ISD::ANDC, dl, VT, LHS, RHS.getOperand(0));

  // NANDL = and (or (any_extend (select_cc a, 0, -1, 0, b, seteq)),
  //                 (any_extend (select_cc c, 0, -1, 0, d, seteq))),
  //              1
  if ((LHS.getOpcode() == ISD::OR) && (isPos1(RHS))) {
    if ((LHS.getOperand(0).getOpcode() == ISD::ANY_EXTEND) &&
        (LHS.getOperand(1).getOpcode() == ISD::ANY_EXTEND)) {

      DEBUG(dbgs() << "AND (OR ((ANY_EXTEND), (ANY_EXTEND)), 1)\n");

      SDValue ext0 = LHS.getOperand(0);
      SDValue ext1 = LHS.getOperand(1);

      if ((ext0.getOperand(0).getOpcode() == ISD::SELECT_CC) &&
          (ext1.getOperand(0).getOpcode() == ISD::SELECT_CC)) {

        DEBUG(dbgs() << "Extends are from SELECT_CC\n");

        SDValue select0 = ext0.getOperand(0);
        SDValue select1 = ext0.getOperand(0);
        ISD::CondCode CC0 = cast<CondCodeSDNode>(select0->getOperand(4))->get();
        ISD::CondCode CC1 = cast<CondCodeSDNode>(select1->getOperand(4))->get();

        if ((CC0 == ISD::SETEQ) && (CC1 == ISD::SETEQ))
          DEBUG(dbgs() << "Nodes both use SETEQ\n");

        if ((isZero(select0.getOperand(1))) &&
            (isZero(select1.getOperand(1))) &&
            (isNeg1(select0.getOperand(2))) &&
            (isNeg1(select1.getOperand(2))) &&
            (isZero(select0.getOperand(3))) &&
            (isZero(select1.getOperand(3))) &&
            (CC0 == ISD::SETEQ) && (CC1 == ISD::SETEQ)) {

          DEBUG(dbgs() << "Combining into NANDL\n");
          return DAG.getNode(LE1ISD::NANDL, dl, VT, select0.getOperand(0),
                             select1.getOperand(0));
        }
      }
    }
  }
  // ANDL
  if ((LHS.getOpcode() == ISD::XOR) && (isPos1(RHS))) {
    if ((LHS.getOperand(0).getOpcode() == ISD::OR) &&
        (isNeg1(LHS.getOperand(1)))) {

      SDValue Or = LHS.getOperand(0);

      if ((Or.getOperand(0).getOpcode() == ISD::ANY_EXTEND) &&
          (Or.getOperand(1).getOpcode() == ISD::ANY_EXTEND)) {

        SDValue ext0 = Or.getOperand(0);
        SDValue ext1 = Or.getOperand(1);

        if ((ext0.getOperand(0).getOpcode() == ISD::SELECT_CC) &&
            (ext1.getOperand(0).getOpcode() == ISD::SELECT_CC)) {

          SDValue select0 = ext0.getOperand(0);
          SDValue select1 = ext0.getOperand(0);
          ISD::CondCode CC0 =
            cast<CondCodeSDNode>(select0->getOperand(4))->get();
          ISD::CondCode CC1 =
            cast<CondCodeSDNode>(select1->getOperand(4))->get();

          if ((isZero(select0.getOperand(1))) &&
              (isZero(select1.getOperand(1))) &&
              (isNeg1(select0.getOperand(2))) &&
              (isNeg1(select1.getOperand(2))) &&
              (isZero(select0.getOperand(3))) &&
              (isZero(select1.getOperand(3))) &&
              (CC0 == ISD::SETEQ) && (CC1 == ISD::SETEQ)) {

            return DAG.getNode(LE1ISD::ANDL, dl, VT, select0.getOperand(0),
                               select1.getOperand(0));
          }
        }
      }
    }
  }
  return SDValue(N, 0);
}

// ORC = (~(s1)) | (s2)
// ORL = (((s1) == 0) & ((s2) == 0)) ? 0 : 1
SDValue static PerformORCombine(SDNode *N, SelectionDAG &DAG) {
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  if (N->getOperand(1).getOpcode() == ISD::XOR) {
    SDValue Xor = N->getOperand(1);
    if (isNeg1(Xor.getOperand(1)))
      return DAG.getNode(LE1ISD::ORC, dl, VT, N->getOperand(0),
                         Xor.getOperand(0));
  }
  return SDValue(N, 0);
}

SDValue static PerformMULCombine(SDNode *N, SelectionDAG &DAG) {
  DEBUG(dbgs() << "PerformMULCombine\n");
  SDValue LHS = N->getOperand(0);
  SDValue RHS = N->getOperand(1);
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  // def MULLH   // i16(s1) * i16(s2 >> 16)
  // def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                (sra CPURegs:$rhs, (i32 16))),
  //           (MULLH CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                (sra imm:$rhs, (i32 16))),
  //           (MULLH CPURegs:$lhs, imm:$rhs)>;

  // def MULHH   // i16(s1 >> 16) * i16(s2 >> 16)
  // def : Pat<(mul (sra CPURegs:$lhs, (i32 16)),
  //                (sra CPURegs:$rhs, (i32 16))),
  //           (MULHH CPURegs:$lhs, CPURegs:$rhs)>;
  // def MULLL   // i16(s1) * i16(s2)
  // def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                (sra (shl CPURegs:$rhs, (i32 16)), (i32 16))),
  //           (MULLL CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul (sext_inreg CPURegs:$lhs, i16),
  //                (sext_inreg CPURegs:$rhs, i16)),
  //           (MULLL CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul (sra (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                 immSExt16:$rhs),
  //           (MULLLi CPURegs:$lhs, immSExt16:$rhs)>;
  if ((isSRA_16(LHS)) && (isSRA_16(RHS))) {
    DEBUG(dbgs() << "LHS and RHS = SRA_16\n");

    if (isSHL_16(LHS.getOperand(0))) {
      DEBUG(dbgs() << "LHS.getOperand(0) = SHL_16\n");

      if (isSHL_16(RHS.getOperand(0))) {
        return DAG.getNode(LE1ISD::MULLL, dl, VT,
                           LHS.getOperand(0).getOperand(0),
                           RHS.getOperand(0).getOperand(0));
      }
      else
        return DAG.getNode(LE1ISD::MULLH, dl, VT,
                           LHS.getOperand(0).getOperand(0), RHS.getOperand(0));
    }
    else
      return DAG.getNode(LE1ISD::MULHH, dl, VT, LHS.getOperand(0),
                         RHS.getOperand(0));
  }
  else if ((isSEXT_16(LHS)) && (isSEXT_16(RHS))) {
    DEBUG(dbgs() << "LHS and RHS = SEXT_16\n");
    return DAG.getNode(LE1ISD::MULLL, dl, VT, LHS.getOperand(0),
                       RHS.getOperand(0));
  }
  else if ((isSRA_16(LHS)) && (is16BitUImm(RHS))) {
    DEBUG(dbgs() << "LHS = SRA_16 and RHS = 16BitUImm\n");
    if (isSHL_16(LHS.getOperand(0)))
      return DAG.getNode(LE1ISD::MULLL, dl, VT, LHS.getOperand(0).getOperand(0),
                         RHS);
  }
  // def MULLLU  // ui16(s1) * ui16(s2)
  // def : Pat<(mul (and CPURegs:$lhs, (i32 0xffff)),
  //                (and CPURegs:$rhs, (i32 0xffff))),
  //           (MULLLU CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                (srl (shl CPURegs:$rhs, (i32 16)), (i32 16))),
  //           (MULLLU CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                 immZExt16:$rhs),
  //           (MULLLUi CPURegs:$lhs, immZExt16:$rhs)>;
  if (isSRL_16(LHS)) {
    if (isSHL_16(LHS.getOperand(0))) {
      DEBUG(dbgs() << "LHS = SRL_16, LHS.getOperand(0) = SHL_16\n");
      if ((isSRL_16(RHS)) && (isSHL_16(RHS.getOperand(0)))) {
        DEBUG(dbgs() << "RHS = SRL_16, RHS.getOperand(0) = SHL_16\n");
        return DAG.getNode(LE1ISD::MULLLU, dl, VT,
                           LHS.getOperand(0).getOperand(0),
                           RHS.getOperand(0).getOperand(0));
      }
      else if (is16BitUImm(RHS))
        return DAG.getNode(LE1ISD::MULLLU, dl, VT,
                           LHS.getOperand(0).getOperand(0), RHS);
    }
  }
  else if ((is16BitMask(LHS)) && (is16BitMask(RHS)))
    return DAG.getNode(LE1ISD::MULLLU, dl, VT, LHS.getOperand(0).getOperand(0),
                       RHS.getOperand(1).getOperand(0));
  else if ((LHS.getOpcode() == ISD::ZERO_EXTEND) &&
           (RHS.getOpcode() == ISD::ZERO_EXTEND))
    return DAG.getNode(LE1ISD::MULLU, dl, VT, LHS.getOperand(0),
                       RHS.getOperand(0));

  // def MULHHU  // ui16(s1 >> 16) * ui16(s2 >> 16)
  // def : Pat<(mul (srl CPURegs:$lhs, (i32 16)),
  //                (srl CPURegs:$rhs, (i32 16))),
  //           (MULHHU CPURegs:$lhs, CPURegs:$rhs)>;
  // def MULLHU  // ui16(s1) * ui16(s2 >> 16)
  // def : Pat<(mul (srl (shl CPURegs:$lhs, (i32 16)), (i32 16)),
  //                (srl CPURegs:$rhs, (i32 16))),
  //           (MULLHU CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul (and CPURegs:$lhs, (i32 0xffff)),
  //                (srl CPURegs:$rhs, (i32 16))),
  //        (MULLHU CPURegs:$lhs, CPURegs:$rhs)>;
  if ((isSRL_16(LHS)) && (isSRL_16(RHS))) {
    if (isSHL_16(LHS.getOperand(0)))
      return DAG.getNode(LE1ISD::MULLHU, dl, VT,
                         LHS.getOperand(0).getOperand(0),
                         RHS.getOperand(0));
    else
      return DAG.getNode(LE1ISD::MULHHU, dl, VT, LHS.getOperand(0),
                         RHS.getOperand(0));
  }
  else if ((is16BitMask(LHS)) && (isSRA_16(RHS)))
    return DAG.getNode(LE1ISD::MULLHU, dl, VT, LHS.getOperand(0),
                       RHS.getOperand(0));

  // def MULL    // s1 * i16(s2)
  // def : Pat<(mul CPURegs:$lhs, (sra (shl CPURegs:$rhs, (i32 16)), (i32 16))),
  //           (MULL CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul CPURegs:$lhs, (sext_inreg CPURegs:$rhs, i16)),
  //           (MULL CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul CPURegs:$lhs, immSExt16:$rhs),
  //           (MULLi CPURegs:$lhs, immSExt16:$rhs)>;
  if ((isSRA_16(RHS)) && (isSHL_16(RHS.getOperand(0))))
    return DAG.getNode(LE1ISD::MULL, dl, VT, LHS,
                       RHS.getOperand(0).getOperand(0));
  else if (isSEXT_16(RHS))
    return DAG.getNode(LE1ISD::MULL, dl, VT, LHS, RHS.getOperand(0));
  else if (is16BitSImm(RHS))
    return DAG.getNode(LE1ISD::MULL, dl, VT, LHS, RHS);

  // def MULLU   // s1 * ui16(s2)
  // def : Pat<(mul CPURegs:$lhs, (and CPURegs:$rhs, (i32 0xFFFF))),
  //           (MULLU CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul CPURegs:$lhs, (srl (shl CPURegs:$rhs, (i32 16)), (i32 16))),
  //           (MULLU CPURegs:$lhs, CPURegs:$rhs)>;
  // def : Pat<(mul CPURegs:$lhs, immZExt16:$rhs),
  //           (MULLUi CPURegs:$lhs, immZExt16:$rhs)>;
  // def MULHU   // s1 * ui16(s2 >> 16)
  // def : Pat<(mul CPURegs:$lhs, (srl CPURegs:$rhs, (i32 16))),
  //           (MULH CPURegs:$lhs, CPURegs:$rhs)>;
  if (isSRL_16(RHS)) {
    if (isSHL_16(RHS.getOperand(0)))
      return DAG.getNode(LE1ISD::MULLU, dl, VT, LHS, RHS.getOperand(0));
    else
      return DAG.getNode(LE1ISD::MULHU, dl, VT, LHS, RHS.getOperand(0));
  }
  else if (is16BitUImm(RHS))
    return DAG.getNode(LE1ISD::MULLU, dl, VT, LHS, RHS);
  else if ((is16BitMask(RHS)))
    return DAG.getNode(LE1ISD::MULLU, dl, VT, LHS, RHS.getOperand(0));

  //def MULH    // s1 * i16(s2 >> 16)
  //def : Pat<(mul CPURegs:$lhs, (sra CPURegs:$rhs, (i32 16))),
  //          (MULH CPURegs:$lhs, CPURegs:$rhs)>;
  if ((isSRA_16(RHS)))
    return DAG.getNode(LE1ISD::MULH, dl, VT, LHS, RHS.getOperand(0));

  return SDValue(N, 0);
}

SDValue static PerformSHLCombine(SDNode *N, SelectionDAG &DAG) {
  DEBUG(dbgs() << "PerformSHLCombine\n");
  SDValue Op0 = N->getOperand(0);
  SDValue Op1 = N->getOperand(1);
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  // def MULHS   // s1 * ui16(s2 >> 16) << 16
  // def : Pat<(shl (mul CPURegs:$lhs,
  //                     (srl CPURegs:$rhs, (i32 16))),
  //                (i32 16)),
  //           (MULHS CPURegs:$lhs, CPURegs:$rhs)>;
  if ((Op0.getOpcode() == ISD::MUL) && (isImm16(Op1))) {
    DEBUG(dbgs() << "Op0 = MUL, Op1 = Imm16\n");
    if (isSRL_16(Op0.getOperand(1)))
      return DAG.getNode(LE1ISD::MULHS, dl, VT, Op0.getOperand(0),
                         Op0.getOperand(1).getOperand(0));
  }
  return SDValue(N, 0);
}

SDValue static PerformSELECT_CCCombine(SDNode *N, SelectionDAG &DAG) {
  SDValue Op0 = N->getOperand(0);
  SDValue Op1 = N->getOperand(1);
  SDValue Op2 = N->getOperand(2);
  SDValue Op3 = N->getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(N->getOperand(4))->get();
  SDLoc dl(N);
  EVT VT = N->getValueType(0);

  // NORL, ORL
  if ((Op0.getOpcode() == ISD::OR) && (isZero(Op1)) && (isNeg1(Op2)) &&
      (isZero(Op3))) {
    if (CC == ISD::SETEQ)
      return DAG.getNode(LE1ISD::NORL, dl, VT, Op0.getOperand(0),
                         Op0.getOperand(1));
    else if (CC == ISD::SETNE)
      return DAG.getNode(LE1ISD::ORL, dl, VT, Op0.getOperand(0),
                         Op0.getOperand(1));
  }
  return SDValue(N, 0);
}

SDValue LE1TargetLowering::PerformDAGCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  DEBUG(dbgs() << "PeformDAGCombine\n");
  SelectionDAG &DAG = DCI.DAG;
  switch(N->getOpcode()) {
  default:
    break;
  case ISD::ADD:        return PerformADDCombine(N, DAG);
  case ISD::AND:        return PerformANDCombine(N, DAG);
  case ISD::OR:         return PerformORCombine(N, DAG);
  case ISD::MUL:        return PerformMULCombine(N, DAG);
  case ISD::SHL:        return PerformSHLCombine(N, DAG);
  case ISD::SELECT_CC:  return PerformSELECT_CCCombine(N, DAG);
  }
  return SDValue();
}

EVT LE1TargetLowering::getSetCCResultType(EVT VT) const {
  if (!VT.isVector())
    return MVT::i1;
  else
    return VT;
}

SDValue LE1TargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode())
  {
    case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
    case ISD::MULHS:              return LowerMULHS(Op, DAG);
    case ISD::MULHU:              return LowerMULHU(Op, DAG);
    case ISD::MUL:                return LowerMUL(Op, DAG);
    case ISD::SDIV:               return LowerSDIV(Op, DAG);
    case ISD::UDIV:               return LowerUDIV(Op, DAG);
    case ISD::SREM:               return LowerSREM(Op, DAG);
    case ISD::UREM:               return LowerUREM(Op, DAG);
    case ISD::VASTART:            return LowerVASTART(Op, DAG);
    case ISD::SELECT_CC:          return LowerSELECT_CC(Op, DAG);
    case ISD::SETCC:              return LowerSETCC(Op, DAG);
    case ISD::BRCOND:             return LowerBRCOND(Op, DAG);
    case ISD::FRAMEADDR:          return LowerFRAMEADDR(Op, DAG);
    case ISD::INTRINSIC_VOID:
    case ISD::INTRINSIC_W_CHAIN:  return LowerIntrinsicWChain(Op, DAG);
    case ISD::INTRINSIC_WO_CHAIN: return LowerINTRINSIC_WO_CHAIN(Op, DAG);
  }
  return SDValue();
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

// AddLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned
AddLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
  assert(RC->contains(PReg) && "Not the correct regclass!");
  unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
  MF.getRegInfo().addLiveIn(PReg, VReg);
  return VReg;
}

SDValue LE1TargetLowering::LowerMUL(SDValue Op, SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDLoc dl(Op.getNode());

  SDValue Mullu = DAG.getNode(LE1ISD::MULLU, dl, Op.getValueType(),
                              LHS, RHS);
  SDValue Mulhs = DAG.getNode(LE1ISD::MULHS, dl, Op.getValueType(),
                              LHS, RHS);
  return DAG.getNode(ISD::ADD, dl, Op.getValueType(), Mullu, Mulhs);
}

SDValue LE1TargetLowering::
LowerMULHS(SDValue Op, SelectionDAG &DAG) const
{
  // Taken from 'A Hacker's Delight'
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue ShiftImm = DAG.getTargetConstant(16, MVT::i32);
  SDValue MaskImm = DAG.getTargetConstant(0xFFFF, MVT::i32);

  SDValue v0, v1;

  // Check whether the second operand is an immediate value
  if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(RHS)) {
    // If it's large, move it to a register and then modify
    if(C->getConstantIntValue()->getBitWidth() > 16) {
      //SDValue v = DAG.getNode(LE1ISD::Mov, dl, MVT::i32, RHS);
      v0 = DAG.getNode(ISD::AND, dl, MVT::i32, RHS, MaskImm);
      v1 = DAG.getNode(ISD::SRA, dl, MVT::i32, RHS, ShiftImm);
    }
  }
  else {
    v0 = DAG.getNode(ISD::AND, dl, MVT::i32, RHS, MaskImm);
    v1 = DAG.getNode(ISD::SRA, dl, MVT::i32, RHS, ShiftImm);
  }

  SDValue u0 = DAG.getNode(ISD::AND, dl, MVT::i32, LHS, MaskImm);
  SDValue u1 = DAG.getNode(ISD::SRA, dl, MVT::i32, LHS, ShiftImm);

  SDValue w0 = DAG.getNode(ISD::MUL, dl, MVT::i32, u0, v0);
  SDValue t1 = DAG.getNode(ISD::MUL, dl, MVT::i32, u1, v0);
  SDValue t2 = DAG.getNode(ISD::SRL, dl, MVT::i32, w0, ShiftImm);
  SDValue t = DAG.getNode(ISD::ADD, dl, MVT::i32, t1, t2);

  SDValue w1 = DAG.getNode(ISD::AND, dl, MVT::i32, t, MaskImm);
  SDValue w2 = DAG.getNode(ISD::SRA, dl, MVT::i32, t, ShiftImm);

  SDValue w3 = DAG.getNode(ISD::MUL, dl, MVT::i32, u0, v1);
  SDValue w4 = DAG.getNode(ISD::ADD, dl, MVT::i32, w3, w1);

  SDValue u1v1 = DAG.getNode(ISD::MUL, dl, MVT::i32, u1, v1);
  SDValue w4Shift = DAG.getNode(ISD::SRA, dl, MVT::i32, w4, ShiftImm);
  SDValue resA = DAG.getNode(ISD::ADD, dl, MVT::i32, u1v1, w4Shift);
  return DAG.getNode(ISD::ADD, dl, MVT::i32, resA, w2);
}

SDValue LE1TargetLowering::
LowerMULHU(SDValue Op, SelectionDAG &DAG) const
{
  // Taken from 'A Hacker's Delight'
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue ShiftImm = DAG.getTargetConstant(16, MVT::i32);
  SDValue MaskImm = DAG.getTargetConstant(0xFFFF, MVT::i32);

  SDValue v0, v1;

  // Check whether the second operand is an immediate value
  if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(RHS)) {
    // If it's large, move it to a register and then modify
    if(C->getConstantIntValue()->getBitWidth() > 16) {
      SDValue v = DAG.getNode(LE1ISD::Mov, dl, MVT::i32, RHS);
      v0 = DAG.getNode(ISD::AND, dl, MVT::i32, RHS, MaskImm);
      v1 = DAG.getNode(ISD::SRL, dl, MVT::i32, RHS, ShiftImm);
    }
  }
  else {
    v0 = DAG.getNode(ISD::AND, dl, MVT::i32, RHS, MaskImm);
    v1 = DAG.getNode(ISD::SRA, dl, MVT::i32, RHS, ShiftImm);
  }

  SDValue u0 = DAG.getNode(ISD::AND, dl, MVT::i32, LHS, MaskImm);
  SDValue u1 = DAG.getNode(ISD::SRL, dl, MVT::i32, LHS, ShiftImm);

  SDValue w0 = DAG.getNode(ISD::MUL, dl, MVT::i32, u0, v0);
  SDValue t1 = DAG.getNode(ISD::MUL, dl, MVT::i32, u1, v0);
  SDValue t2 = DAG.getNode(ISD::SRL, dl, MVT::i32, w0, ShiftImm);
  SDValue t = DAG.getNode(ISD::ADD, dl, MVT::i32, t1, t2);

  SDValue w1 = DAG.getNode(ISD::AND, dl, MVT::i32, t, MaskImm);
  SDValue w2 = DAG.getNode(ISD::SRL, dl, MVT::i32, t, ShiftImm);

  SDValue w3 = DAG.getNode(ISD::MUL, dl, MVT::i32, u0, v1);
  SDValue w4 = DAG.getNode(ISD::ADD, dl, MVT::i32, w3, w1);

  SDValue u1v1 = DAG.getNode(ISD::MUL, dl, MVT::i32, u1, v1);
  SDValue w4Shift = DAG.getNode(ISD::SRL, dl, MVT::i32, w4, ShiftImm);
  SDValue resA = DAG.getNode(ISD::ADD, dl, MVT::i32, u1v1, w4Shift);
  return DAG.getNode(ISD::ADD, dl, MVT::i32, resA, w2);

  /*
  DebugLoc dl = Op.getDebugLoc();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue ShiftImm = DAG.getTargetConstant(16, MVT::i32);
  SDValue Zero = DAG.getRegister(LE1::ZERO, MVT::i32);

  SDValue Mullhu0 = DAG.getNode(LE1ISD::Mullhu, dl, MVT::i32, LHS, RHS);
  SDValue Mullhu1 = DAG.getNode(LE1ISD::Mullhu, dl, MVT::i32, RHS, LHS);
  SDValue Mulllu0 = DAG.getNode(LE1ISD::Mulllu, dl, MVT::i32, LHS, RHS);
  SDValue Mulhhu0 = DAG.getNode(LE1ISD::Mulhhu, dl, MVT::i32, LHS, RHS);

  SDValue Shru0 = DAG.getNode(ISD::SRL, dl, MVT::i32, Mullhu0, ShiftImm);
  SDValue Shru1 = DAG.getNode(ISD::SRL, dl, MVT::i32, Mullhu1, ShiftImm);

  SDValue Add0 = DAG.getNode(ISD::ADD, dl, MVT::i32, Shru0, Shru1);
  SDValue Add1 = DAG.getNode(ISD::ADD, dl, MVT::i32, Mulhhu0, Add0);
  // Add1 now holds the current top 32 bits, now just have to calculate any
  // bits carried from the additions of the lower 32 bits

  SDValue MTB0 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Zero);
  SDValue Addcg0 = DAG.getNode(LE1ISD::Addcg, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               Mulllu0, Mullhu0, MTB0);
  SDValue Cout0(Addcg0.getNode(), 1);

  SDValue Addcg1 = DAG.getNode(LE1ISD::Addcg, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               Addcg0, Mullhu1, Cout0);
  SDValue Cout1(Addcg1.getNode(), 1);

  SDValue Add2 = DAG.getNode(ISD::ADD, dl, MVT::i32, Add1,
                             DAG.getTargetConstant(1, MVT::i32));

  return DAG.getNode(ISD::SELECT, dl, MVT::i32, Cout1,
                                Add2, Add1);*/
}

SDValue LE1TargetLowering::
LowerSDIV(SDValue Op, SelectionDAG &DAG) const
{
  //std::cout << "LowerSDIV\n";
  assert((Op.getValueType() == MVT::i32) && "32-bit division only!");
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue Zero = DAG.getRegister(LE1::ZERO, MVT::i32);

  // Check whether the polarity of the values. If the argument is negative,
  // use its absolute value.
  //SDValue Cmplt0 = DAG.getNode(LE1ISD::Cmplt, dl, MVT::i32, LHS, Zero);
  SDValue Cmplt0 = DAG.getSetCC(dl, MVT::i32, LHS, Zero, ISD::SETLT);
  SDValue Sub0 = DAG.getNode(ISD::SUB, dl, MVT::i32, Zero, LHS);
  //SDValue MTB1 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Cmplt0);
  SDValue Slct0 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmplt0, Sub0, LHS);

  //SDValue Cmplt1 = DAG.getNode(LE1ISD::Cmplt, dl, MVT::i32, RHS, Zero);
  SDValue Cmplt1 = DAG.getSetCC(dl, MVT::i32, RHS, Zero, ISD::SETLT);
  SDValue Sub1 = DAG.getNode(ISD::SUB, dl, MVT::i32, Zero, RHS);
  //SDValue MTB2 = DAG.getNode(LE1ISD::MFB, dl, MVT::i1, Cmplt1);
  SDValue Slct1 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmplt1, Sub1, RHS);

  // Check whether the polarities are the same
  //SDValue Cmpeq0 = DAG.getNode(LE1ISD::Cmpeq, dl, MVT::i1, Cmplt1, Cmplt0);
  SDValue Cmpeq0 = DAG.getSetCC(dl, MVT::i1, Cmplt1, Cmplt0, ISD::SETEQ);

  // Begin the division process
  SDValue MTB0 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Zero);
  SDValue Addcg0 = DAG.getNode(LE1ISD::Addcg, dl, 
                               DAG.getVTList(MVT::i32, MVT::i1),
                               Slct0, Slct0, MTB0);
  SDValue AddCout(Addcg0.getNode(), 1);
  SDValue Res = DivStep(Zero, Slct1, AddCout, Addcg0, MTB0, DAG);

  // If the polarities were the same, return result otherwise return the
  // corrected value using Sub2
  SDValue Sub2 = DAG.getNode(ISD::SUB, dl, MVT::i32, Zero, Res);
  return DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmpeq0, Res, Sub2);
}

SDValue LE1TargetLowering::
LowerSREM(SDValue Op, SelectionDAG &DAG) const
{
  //std::cout << "LowerSREM\n";
  assert((Op.getValueType() == MVT::i32) && "32-bit remainder only!");
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue Zero = DAG.getRegister(LE1::ZERO, MVT::i32);

  //SDValue Cmplt0 = DAG.getNode(LE1ISD::Cmplt, dl, MVT::i1, LHS, Zero);
  SDValue Cmplt0 = DAG.getSetCC(dl, MVT::i1, LHS, Zero, ISD::SETLT);
  SDValue Sub0 = DAG.getNode(ISD::SUB, dl, MVT::i32, Zero, LHS);
  SDValue Slct0 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmplt0, Sub0, LHS);

  //SDValue Cmplt1 = DAG.getNode(LE1ISD::Cmplt, dl, MVT::i1, RHS, Zero);
  SDValue Cmplt1 = DAG.getSetCC(dl, MVT::i1, RHS, Zero, ISD::SETLT);
  SDValue Sub1 = DAG.getNode(ISD::SUB, dl, MVT::i32, Zero, RHS);
  SDValue Slct1 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmplt1, Sub1, RHS);

  SDValue MTB0 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Zero);
  SDValue Addcg0 = DAG.getNode(LE1ISD::Addcg, dl, 
                               DAG.getVTList(MVT::i32, MVT::i1),
                               Slct0, Slct0, MTB0);
  SDValue AddCout(Addcg0.getNode(), 1);

  // Perform the addcg/divs operations
  SDValue Res = RemStep(Zero, Slct1, AddCout, Addcg0, MTB0, DAG);
  //SDValue ResCout(Res.getNode(), 1);

  //SDValue Cmpge0 = DAG.getNode(LE1ISD::Cmpge, dl, MVT::i1, Res, Zero);
  SDValue Cmpge0 = DAG.getSetCC(dl, MVT::i1, Res, Zero, ISD::SETGE);
  SDValue Add0 = DAG.getNode(ISD::ADD, dl, MVT::i32, Res, Slct1);
  SDValue Slct2 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmpge0, Res, Add0);
  SDValue Sub2 = DAG.getNode(ISD::SUB, dl, MVT::i32, Zero, Slct2);
  return DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmplt0, Sub2, Slct2); 

}

SDValue LE1TargetLowering::LowerUDIV(SDValue Op, SelectionDAG &DAG) const
{
  //std::cout << "LowerUDIV\n";
  assert((Op.getValueType() == MVT::i32) && "32-bit division only!");
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue Zero = DAG.getRegister(LE1::ZERO, MVT::i32);

  //SDValue Cmplt = DAG.getNode(LE1ISD::Cmplt, dl, MVT::i32, RHS, Zero);
  SDValue CMPLT = DAG.getSetCC(dl, MVT::i32, RHS, Zero, ISD::SETLT);
  SDValue MTB0 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Zero);
  //SDValue ShiftBit = DAG.getNode(LE1ISD::MFB, dl, MVT::i32, Cmplt);
  SDValue Shru0 = DAG.getNode(ISD::SRA, dl, MVT::i32, RHS, CMPLT);
  SDValue Shru1 = DAG.getNode(ISD::SRA, dl, MVT::i32, LHS, CMPLT);

  //SDValue Cmpgeu = DAG.getNode(LE1ISD::Cmpgeu, dl, MVT::i1, LHS, RHS);
  SDValue CMPGEU = DAG.getSetCC(dl, MVT::i1, LHS, RHS, ISD::SETUGE);

  SDValue Addcg = DAG.getNode(LE1ISD::Addcg, dl,
                             DAG.getVTList(MVT::i32, MVT::i1),
                             Shru1, Shru1, MTB0);
  //std::cout << "ADD0\n";
  SDValue CarryA(Addcg.getNode(), 1);
  SDValue Res = DivStep(Zero, Shru0, CarryA, Addcg, MTB0, DAG);
  //SDValue MTB1 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, CMPLT);
  SDValue MFB0 = DAG.getNode(LE1ISD::MFB, dl, MVT::i32, CMPGEU);
  return DAG.getNode(ISD::SELECT, dl, MVT::i32, CMPLT, MFB0, Res);
  //return DAG.getNode(ISD::SELECT, dl, MVT::i32, MTB1, Cmpgeu, Res);

}

SDValue LE1TargetLowering::LowerUREM(SDValue Op, SelectionDAG &DAG) const
{
  //std::cout << "LowerUREM\n";
  assert((Op.getValueType() == MVT::i32) && "32-bit remainder only!");
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue Zero = DAG.getRegister(LE1::ZERO, MVT::i32);

  //SDValue Cmplt0 = DAG.getNode(LE1ISD::Cmplt, dl, MVT::i32, RHS, Zero);
  SDValue Cmplt0 = DAG.getSetCC(dl, MVT::i32, RHS, Zero, ISD::SETLT);
  SDValue MTB0 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Zero);
  //SDValue MTB1 = DAG.getNode(LE1ISD::MFB, dl, MVT::i32, Cmplt0);

  SDValue Shru0 = DAG.getNode(ISD::SRA, dl, MVT::i32, RHS, Cmplt0);
  SDValue Shru1 = DAG.getNode(ISD::SRA, dl, MVT::i32, LHS, Cmplt0);
  //SDValue Cmpgeu0 = DAG.getNode(LE1ISD::Cmpgeu, dl, MVT::i1, LHS, RHS);
  SDValue Cmpgeu0 = DAG.getSetCC(dl, MVT::i1, LHS, RHS, ISD::SETUGE);
  // FIXME unused value?!
  SDValue Sub0 = DAG.getNode(ISD::SUB, dl, MVT::i32, LHS, RHS);

  SDValue Addcg0 = DAG.getNode(LE1ISD::Addcg, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               Shru1, Shru1, MTB0);
  SDValue AddCout(Addcg0.getNode(),1);

  SDValue Slct0 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmpgeu0, LHS, RHS);
  SDValue Res = RemStep(Zero, Shru0, AddCout, Addcg0, MTB0, DAG);

  //SDValue Cmpge0 = DAG.getNode(LE1ISD::Cmpge, dl, MVT::i1, Res, Zero);
  SDValue Cmpge0 = DAG.getSetCC(dl, MVT::i1, Res, Zero, ISD::SETGE);
  SDValue Add0 = DAG.getNode(ISD::ADD, dl, MVT::i32, Res, Shru0);
  SDValue Slct1 = DAG.getNode(ISD::SELECT, dl, MVT::i32, Cmpge0, Res, Add0);
  SDValue MTB1 = DAG.getNode(LE1ISD::MTB, dl, MVT::i1, Cmplt0);
  return DAG.getNode(ISD::SELECT, dl, MVT::i32, MTB1, Slct0, Slct1);

}

SDValue LE1TargetLowering::
RemStep(SDValue DivArg1, SDValue DivArg2, SDValue DivCin, SDValue AddArg,
        SDValue AddCin, SelectionDAG &DAG) const {
  // Need to do this 31 times
  static int stepCount = 0;
  // FIXME no DebugLoc
  SDLoc dl(DivArg1.getNode());

  SDValue DivRes = DAG.getNode(LE1ISD::Divs, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               DivArg1, DivArg2, DivCin);
  SDValue DivCout(DivRes.getNode(), 1);
  
  SDValue AddRes = DAG.getNode(LE1ISD::Addcg, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               AddArg, AddArg, AddCin);
  SDValue AddCout(AddRes.getNode(), 1);

  ++stepCount;
  if(stepCount < 31)
    return RemStep(DivRes, DivArg2, AddCout, AddRes, DivCout, DAG);
  else {
    stepCount = 0;
    return DAG.getNode(LE1ISD::Divs, dl,
                       DAG.getVTList(MVT::i32, MVT::i1),
                       DivRes, DivArg2, AddCout);
  }
}

SDValue LE1TargetLowering::
DivStep(SDValue DivArg1, SDValue DivArg2, SDValue DivCin, SDValue AddArg,
        SDValue AddCin, SelectionDAG &DAG) const {
  // Need to do this 32 times
  static int stepCount = 0;
  //FIXME no DebugLoc
  SDLoc dl(DivArg1.getNode());

  SDValue DivRes = DAG.getNode(LE1ISD::Divs, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               DivArg1, DivArg2, DivCin);
  SDValue DivCout(DivRes.getNode(), 1);
  
  SDValue AddRes = DAG.getNode(LE1ISD::Addcg, dl,
                               DAG.getVTList(MVT::i32, MVT::i1),
                               AddArg, AddArg, AddCin);
  SDValue AddCout(AddRes.getNode(), 1);

  ++stepCount;
  if(stepCount < 32)
    return DivStep(DivRes, DivArg2, AddCout, AddRes, DivCout, DAG);
  else {
    stepCount = 0;
    SDValue Zero = DAG.getRegister(LE1::ZERO, MVT::i32);
    SDValue Final = DAG.getNode(LE1ISD::Addcg, dl,
                                DAG.getVTList(MVT::i32, MVT::i1),
                                AddRes, AddRes, DivCout);
    //SDValue Cmpge = DAG.getNode(LE1ISD::Cmpge, dl, MVT::i1, DivRes, Zero);
    SDValue Cmpge = DAG.getSetCC(dl, MVT::i1, DivRes, Zero, ISD::SETGE);
    //SDValue Cmpge = DAG.getNode(ISD::SETCC, dl, MVT::i1, DivRes, Zero,
      //                          DAG.getConstant(ISD::SETGE, MVT::i32));
    SDValue ORC = DAG.getNode(LE1ISD::ORC, dl, MVT::i32, Final, Zero);
    SDValue MFB = DAG.getNode(LE1ISD::MFB, dl, MVT::i32, Cmpge);
    return DAG.getNode(LE1ISD::SH1ADD, dl, MVT::i32, ORC, MFB);
    // return AddRes;
  }
}

static SDValue CreateCMP(SDValue Op, SelectionDAG &DAG) {
  if (Op.getOpcode() != ISD::SETCC)
    return Op;

  SDLoc dl(Op.getNode());
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  unsigned Opcode = 0;

  switch(CC) {
  default:
    return Op;
  case ISD::SETEQ:
    Opcode = LE1ISD::CMPEQ;
    break;
  case ISD::SETNE:
    Opcode = LE1ISD::CMPNE;
    break;
  case ISD::SETLT:
    Opcode = LE1ISD::CMPLT;
    break;
  case ISD::SETULT:
    Opcode = LE1ISD::CMPLTU;
    break;
  case ISD::SETLE:
    Opcode = LE1ISD::CMPLE;
    break;
  case ISD::SETULE:
    Opcode = LE1ISD::CMPLEU;
    break;
  case ISD::SETGT:
    Opcode = LE1ISD::CMPGT;
    break;
  case ISD::SETUGT:
    Opcode = LE1ISD::CMPGTU;
    break;
  case ISD::SETGE:
    Opcode = LE1ISD::CMPGE;
    break;
  case ISD::SETUGE:
    Opcode = LE1ISD::CMPGEU;
    break;
  }

  return DAG.getNode(Opcode, dl, Op.getValueType(), LHS, RHS);
}

SDValue LE1TargetLowering::LowerSETCC(SDValue Op,
                                      SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(1);
  SDValue RHS = Op.getOperand(2);
  SDLoc dl(Op.getNode());
  SDNode* Node = Op.getNode();
  ISD::CondCode CC = cast<CondCodeSDNode>(Node->getOperand(2))->get();
  unsigned Opcode = 0;

  // ORL and NORL
  if (LHS.getOpcode() == ISD::OR) {
    if (ConstantSDNode* CSN = cast<ConstantSDNode>(RHS.getNode())) {
      if (CSN->getConstantOperandVal(0) == 0) {

        if (CC == ISD::SETEQ)
          Opcode = LE1ISD::NORL;
        else if (CC == ISD::SETNE)
          Opcode = LE1ISD::ORL;
        else
          return SDValue();

        return DAG.getNode(Opcode, dl, Op.getValueType(), LHS, RHS);
      }
    }
  }
  // TBIT and TBITF
  else if (LHS.getOpcode() == ISD::AND) {
    if (ConstantSDNode *CSN = cast<ConstantSDNode>(RHS.getNode())) {
      if ((CSN->getConstantOperandVal(0) == 0) &&
          (LHS.getOperand(0).getOpcode() == ISD::SHL)) {

        if (ConstantSDNode *CSN =
            cast<ConstantSDNode>(LHS.getOperand(0).getOperand(0).getNode())) {
          if (CSN->getConstantOperandVal(0) == 1) {

            if (CC == ISD::SETEQ)
              Opcode = LE1ISD::TBITF;
            else if (CC == ISD::SETNE)
              Opcode = LE1ISD::TBIT;
            else
              return SDValue();

            return DAG.getNode(Opcode, dl, Op.getValueType(), LHS, RHS);
          }
        }
      }
    }
  }
  return SDValue();
}

SDValue LE1TargetLowering::LowerBRCOND(SDValue Op, SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  SDValue Dest = Op.getOperand(2);
  SDLoc dl(Op.getNode());
  SDValue Cond = CreateCMP(Op.getOperand(1), DAG);

  return DAG.getNode(LE1ISD::BR, dl, Op.getValueType(), Chain, Cond, Dest);
}

SDValue LE1TargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue Op0 = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  SDValue Op2 = Op.getOperand(2);
  SDValue Op3 = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc dl(Op.getNode());
  EVT VT = Op.getValueType();

  unsigned Opcode = 0;

  // Max and Min
  if ((Op0 == Op2) && (Op1 == Op3)) {
    switch(CC) {
    default:
      return SDValue();
    case ISD::SETGT:
    case ISD::SETGE:
      Opcode = LE1ISD::MAXS;
      break;
    case ISD::SETLT:
    case ISD::SETLE:
      Opcode = LE1ISD::MINS;
      break;
    case ISD::SETUGT:
    case ISD::SETUGE:
      Opcode = LE1ISD::MAXU;
      break;
    case ISD::SETULT:
    case ISD::SETULE:
      Opcode = LE1ISD::MINU;
      break;
    }
    return DAG.getNode(Opcode, dl, Op.getValueType(), Op0, Op1);
  }
  return SDValue();
}

SDValue LE1TargetLowering::LowerINTRINSIC_WO_CHAIN(SDValue Op,
                                                   SelectionDAG &DAG) const {
  //DEBUG(dbgs() << "LowerINTRINISIC_WO_CHAIN\n");
  SDLoc dl(Op.getNode());
  unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();

  // Create a GlobalAddressSDNode with a GlobalValue
  // Then get the address:
  // DAG.getTargetGlobalAddress(GV, dl, getPointerTy(), G->getOffset(), Flags);
  // Then create a load from that address.

  // Most of the intrinsics rely on CPU Id
  SDValue CPUId = DAG.getNode(LE1ISD::CPUID, dl, MVT::i32);
  CPUId = DAG.getNode(ISD::SRL, dl, MVT::i32, CPUId,
                      DAG.getConstant(8, MVT::i32));

  switch(IntNo) {
  case Intrinsic::le1_read_cpuid: {
    return CPUId;
  }
  case Intrinsic::le1_read_group_id_0: {
    SDValue Index = DAG.getNode(ISD::MUL, dl, MVT::i32, CPUId,
                                DAG.getTargetConstant(4, MVT::i32));
    SDValue GroupAddr = DAG.getNode(LE1ISD::GROUP_ID_ADDR, dl, MVT::i32);
    GroupAddr = DAG.getNode(ISD::ADD, dl, MVT::i32, Index, GroupAddr);
    return DAG.getNode(LE1ISD::READ_ATTR, dl, MVT::i32, GroupAddr);
  }
  case Intrinsic::le1_read_group_id_1: {
    SDValue Index = DAG.getNode(ISD::MUL, dl, MVT::i32, CPUId,
                                DAG.getTargetConstant(4, MVT::i32));
    Index = DAG.getNode(ISD::ADD, dl, MVT::i32, Index,
                        DAG.getConstant(1, MVT::i32));
    SDValue GroupAddr = DAG.getNode(LE1ISD::GROUP_ID_ADDR, dl, MVT::i32);
    GroupAddr = DAG.getNode(ISD::ADD, dl, MVT::i32, Index, GroupAddr);
    return DAG.getNode(LE1ISD::READ_ATTR, dl, MVT::i32, GroupAddr);
  }
  case Intrinsic::le1_read_group_id_2: {
    SDValue Index = DAG.getNode(ISD::MUL, dl, MVT::i32, CPUId,
                                DAG.getConstant(4, MVT::i32));
    Index = DAG.getNode(ISD::ADD, dl, MVT::i32, Index,
                        DAG.getConstant(2, MVT::i32));
    SDValue GroupAddr = DAG.getNode(LE1ISD::GROUP_ID_ADDR, dl, MVT::i32);
    GroupAddr = DAG.getNode(ISD::ADD, dl, MVT::i32, Index, GroupAddr);
    return DAG.getNode(LE1ISD::READ_ATTR, dl, MVT::i32, GroupAddr);
  }
  default:
    return SDValue();
    break;
  }

}

SDValue LE1TargetLowering::LowerIntrinsicWChain(SDValue Op,
                                                 SelectionDAG &DAG) const {
  SDLoc dl(Op.getNode());
  unsigned IntNo = cast<ConstantSDNode>(Op.getOperand(1))->getZExtValue();
  SDValue Chain = Op.getOperand(0);
  SDValue CPUId = DAG.getNode(LE1ISD::CPUID, dl, MVT::i32);
  CPUId = DAG.getNode(ISD::SRL, dl, MVT::i32, CPUId,
                      DAG.getConstant(8, MVT::i32));


  // get_global_id = group_id * local_size * local_id
  // group_id = cpuid
  // local_size
  // local_id
  SDValue Result;

  switch(IntNo) {
  case Intrinsic::le1_set_group_id_0: {
    SDValue GroupId = Op.getOperand(2);
    SDValue Index = DAG.getNode(ISD::MUL, dl, MVT::i32, CPUId,
                                DAG.getTargetConstant(4, MVT::i32));
    //Index = DAG.getNode(ISD::ADD, dl, MVT::i32, Index,
      //                  DAG.getConstant(0, MVT::i32));
    SDValue GroupAddr = DAG.getNode(LE1ISD::GROUP_ID_ADDR, dl, MVT::i32);
    GroupAddr = DAG.getNode(ISD::ADD, dl, MVT::i32, Index, GroupAddr);
    //SDValue NumCores = DAG.getNode(LE1ISD::NUM_CORES, dl, MVT::i32);
    //SDValue Workgroup = DAG.getNode(ISD::MUL, dl, MVT::i32, NumCores, GroupId);
    //Workgroup = DAG.getNode(ISD::ADD, dl, MVT::i32, Workgroup, CPUId);
    Result = DAG.getNode(LE1ISD::SET_ATTR, dl, MVT::Other, Chain,
                         GroupId, GroupAddr);
    break;
  }
  case Intrinsic::le1_set_group_id_1: {
    SDValue GroupId = Op.getOperand(2);
    SDValue Index = DAG.getNode(ISD::MUL, dl, MVT::i32, CPUId,
                                DAG.getTargetConstant(4, MVT::i32));
    Index = DAG.getNode(ISD::ADD, dl, MVT::i32, Index,
                        DAG.getConstant(1, MVT::i32));
    SDValue GroupAddr = DAG.getNode(LE1ISD::GROUP_ID_ADDR, dl, MVT::i32);
    GroupAddr = DAG.getNode(ISD::ADD, dl, MVT::i32, Index, GroupAddr);

    //SDValue NumCores = DAG.getNode(LE1ISD::NUM_CORES, dl, MVT::i32);
    //SDValue Workgroup = DAG.getNode(ISD::MUL, dl, MVT::i32, NumCores, GroupId);
    //Workgroup = DAG.getNode(ISD::ADD, dl, MVT::i32, Workgroup, CPUId);
    Result = DAG.getNode(LE1ISD::SET_ATTR, dl, MVT::Other, Chain,
                         GroupId, GroupAddr);
    break;
  }
  case Intrinsic::le1_set_group_id_2: {
    SDValue GroupId = Op.getOperand(2);
    SDValue Index = DAG.getNode(ISD::MUL, dl, MVT::i32, CPUId,
                                DAG.getTargetConstant(4, MVT::i32));
    Index = DAG.getNode(ISD::ADD, dl, MVT::i32, Index,
                        DAG.getConstant(2, MVT::i32));
    SDValue GroupAddr = DAG.getNode(LE1ISD::GROUP_ID_ADDR, dl, MVT::i32);
    GroupAddr = DAG.getNode(ISD::ADD, dl, MVT::i32, Index, GroupAddr);
    //SDValue NumCores = DAG.getNode(LE1ISD::NUM_CORES, dl, MVT::i32);
    //SDValue Workgroup = DAG.getNode(ISD::MUL, dl, MVT::i32, NumCores, GroupId);
    //Workgroup = DAG.getNode(ISD::ADD, dl, MVT::i32, Workgroup, CPUId);
    Result = DAG.getNode(LE1ISD::SET_ATTR, dl, MVT::Other, Chain,
                         GroupId, GroupAddr);
    break;
  }
  default:
    llvm_unreachable("unhandled intrinsic with chain!");
    break;
  }
  //DAG.ReplaceAllUsesWith(Op, Result);
  return Result;
}


SDValue LE1TargetLowering::LowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  //std::cout << "Entering LE1TargetLowering::LowerGlobalAddress\n";
  // FIXME there isn't actually debug info here
  //std::cout << "LowerGlobal, Opcode = " << Op.getOpcode() << std::endl;
  SDLoc dl(Op.getNode());
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();
  EVT ValTy = Op.getValueType();
  SDValue GA = DAG.getTargetGlobalAddress(GV, dl, ValTy, Offset);

  //LE1TargetObjectFile &TLOF =
    //(LE1TargetObjectFile&)getObjFileLowering();
  //if(TLOF.IsGlobalInSmallSection(GV, getTargetMachine())) {
    //std::cout << "isSmallSection\n";
    return DAG.getNode(LE1ISD::TargetGlobal, dl, ValTy, GA);
  //}
  //else std::cout << "!isSmallSection\n";
  return DAG.getNode(LE1ISD::TargetGlobalConst, dl, ValTy, GA);

  return DAG.getNode(LE1ISD::Mov, dl, MVT::i32, GA);

  // Otherwise, just return the address
  return GA;

}

SDValue LE1TargetLowering::LowerLoad(SDValue Op, SelectionDAG &DAG) const {
  /*DebugLoc dl = Op.getDebugLoc();
  LoadSDNode *LoadNode = cast<LoadSDNode>(Op.getNode());
  SDValue Chain = LoadNode->getChain();
  EVT VT = Op.getValueType();
  //EVT VT = LoadNode->getMemoryVT();

  // Only lower load ops that are directly using a global address
  if(Op.getOperand(1)->getOpcode() == LE1ISD::TargetGlobal) {

    SDValue TargetAddr = Op.getOperand(1);
    SDValue TargetOffset = Op.getOperand(2);
    unsigned Opcode = 0;

    switch(Op.getOpcode()) {
    case ISD::EXTLOAD:
    case ISD::ZEXTLOAD:
      Opcode = (VT == MVT::i8) ? LE1ISD::LoadGlobalU8 : LE1ISD::LoadGlobalU16;
      break;
    case ISD::SEXTLOAD:
      Opcode = (VT == MVT::i8) ? LE1ISD::LoadGlobalS8 : LE1ISD::LoadGlobalS16;
      break;
    }
    return DAG.getNode(Opcode, dl, VT, MVT::Other, TargetAddr, TargetOffset,
                       Chain);
  }
  else*/
    return Op;
}

SDValue LE1TargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  LE1FunctionInfo *FuncInfo = MF.getInfo<LE1FunctionInfo>();

  SDLoc dl(Op.getNode());
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                 getPointerTy());

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), dl, FI, Op.getOperand(1),
                      MachinePointerInfo(SV),
                      false, false, 0);
}


SDValue LE1TargetLowering::
LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const {
  // check the depth
  assert((cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() == 0) &&
         "Frame address can only be determined for current frame.");

  MachineFrameInfo *MFI = DAG.getMachineFunction().getFrameInfo();
  MFI->setFrameAddressIsTaken(true);
  EVT VT = Op.getValueType();
  SDLoc dl(Op.getNode());
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), dl, LE1::STRP, VT);
  return FrameAddr;
}

// TODO: set SType according to the desired memory barrier behavior.
/*SDValue LE1TargetLowering::LowerMEMBARRIER(SDValue Op,
                                            SelectionDAG& DAG) const {
  unsigned SType = 0;
  DebugLoc dl = Op.getDebugLoc();
  return DAG.getNode(LE1ISD::Sync, dl, MVT::Other, Op.getOperand(0),
                     DAG.getConstant(SType, MVT::i32));
}*/

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//
#include "LE1GenCallingConv.inc"

//===----------------------------------------------------------------------===//
// TODO: Implement a generic logic using tblgen that can support this.
// LE1 O32 ABI rules:
// ---
// i32 - Passed in A0, A1, A2, A3 and stack
// f32 - Only passed in f32 registers if no int reg has been used yet to hold
//       an argument. Otherwise, passed in A1, A2, A3 and stack.
// f64 - Only passed in two aliased f32 registers if no int reg has been used
//       yet to hold an argument. Otherwise, use A2, A3 and stack. If A1 is
//       not used, it must be shadowed. If only A3 is avaiable, shadow it and
//       go to stack.
//
//  For vararg functions, all arguments are passed in A0, A1, A2, A3 and stack.
//===----------------------------------------------------------------------===//
/*static bool CC_LE1(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State) {

  static const unsigned IntRegsSize=8;

  static const unsigned IntRegs[] = {
      LE1::AR0, LE1::AR1, LE1::AR2, LE1::AR3,
      LE1::AR4, LE1::AR5, LE1::AR6, LE1::AR7
  };

  // ByVal Args
  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo,
                      1, 4, ArgFlags);
    unsigned NextReg = (State.getNextStackOffset() + 3) / 4;
    for (unsigned r = State.getFirstUnallocated(IntRegs, IntRegsSize);
         r < std::min(IntRegsSize, NextReg); ++r)
      State.AllocateReg(IntRegs[r]);
    return false;
  }

  // Promote i8 and i16
  if (LocVT == MVT::i8 || LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  unsigned Reg = State.AllocateReg(IntRegs, IntRegsSize);
  unsigned SizeInBytes = ValVT.getSizeInBits() >> 3;
  unsigned OrigAlign = ArgFlags.getOrigAlign();
  unsigned Offset = State.AllocateStack(SizeInBytes, OrigAlign);

  if (!Reg)
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
  else
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));

  return false; // CC must always match
}*/


//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

static const unsigned O32IntRegsSize = 8;
//FIXME this needs to include AR4,5,6,7
static const uint16_t O32IntRegs[] = {
  LE1::AR0, LE1::AR1, LE1::AR2, LE1::AR3, 
  LE1::AR4, LE1::AR5, LE1::AR6, LE1::AR7
};

// Return next O32 integer argument register.
//static unsigned getNextIntArgReg(unsigned Reg) {
//  assert((Reg == LE1::AR0) || (Reg == LE1::AR2));
//  return (Reg == LE1::AR0) ? LE1::AR1 : LE1::AR3;
//}

// Write ByVal Arg to arg registers and stack.
static void
WriteByValArg(SDValue& ByValChain, SDValue Chain, SDLoc dl,
              SmallVector<std::pair<unsigned, SDValue>, 8>& RegsToPass,
              SmallVector<SDValue, 8>& MemOpChains, int& LastFI,
              MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
              const CCValAssign &VA, const ISD::ArgFlagsTy& Flags,
              MVT PtrType, bool isLittle) {
  //std::cout << "Entering WriteByValArg\n";

  unsigned LocMemOffset = 0;
  if(VA.isMemLoc())
    LocMemOffset = VA.getLocMemOffset();
  unsigned Offset = 0;
  uint32_t RemainingSize = Flags.getByValSize();
  unsigned ByValAlign = Flags.getByValAlign();

  // Copy the first 4 words of byval arg to registers A0 - A3.
  // FIXME: Use a stricter alignment if it enables better optimization in passes
  //        run later.
  for (; RemainingSize >= 4 && LocMemOffset < 4 * 4;
       Offset += 4, RemainingSize -= 4, LocMemOffset += 4) {
    SDValue LoadPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
                                  DAG.getConstant(Offset, MVT::i32));
    SDValue LoadVal = DAG.getLoad(MVT::i32, dl, Chain, LoadPtr,
                                  MachinePointerInfo(),
                                  false, false, false,
                                  std::min(ByValAlign, (unsigned )4));
    MemOpChains.push_back(LoadVal.getValue(1));
    unsigned DstReg = O32IntRegs[LocMemOffset / 4];
    RegsToPass.push_back(std::make_pair(DstReg, LoadVal));
  }

  if (RemainingSize == 0) {
    //std::cout << "RemainingSize = 0\n";
    return;
  }

  // If there still is a register available for argument passing, write the
  // remaining part of the structure to it using subword loads and shifts.
  if (LocMemOffset < 4 * 4) {
    assert(RemainingSize <= 3 && RemainingSize >= 1 &&
           "There must be one to three bytes remaining.");
    unsigned LoadSize = (RemainingSize == 3 ? 2 : RemainingSize);
    SDValue LoadPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
                                  DAG.getConstant(Offset, MVT::i32));
    unsigned Alignment = std::min(ByValAlign, (unsigned )4);
    SDValue LoadVal = DAG.getExtLoad(ISD::ZEXTLOAD, dl, MVT::i32, Chain,
                                     LoadPtr, MachinePointerInfo(),
                                     MVT::getIntegerVT(LoadSize * 8), false,
                                     false, Alignment);
    MemOpChains.push_back(LoadVal.getValue(1));

    // If target is big endian, shift it to the most significant half-word or
    // byte.
    if (!isLittle)
      LoadVal = DAG.getNode(ISD::SHL, dl, MVT::i32, LoadVal,
                            DAG.getConstant(32 - LoadSize * 8, MVT::i32));

    Offset += LoadSize;
    RemainingSize -= LoadSize;

    // Read second subword if necessary.
    if (RemainingSize != 0)  {
      assert(RemainingSize == 1 && "There must be one byte remaining.");
      LoadPtr = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg, 
                            DAG.getConstant(Offset, MVT::i32));
      unsigned Alignment = std::min(ByValAlign, (unsigned )2);
      SDValue Subword = DAG.getExtLoad(ISD::ZEXTLOAD, dl, MVT::i32, Chain,
                                       LoadPtr, MachinePointerInfo(),
                                       MVT::i8, false, false, Alignment);
      MemOpChains.push_back(Subword.getValue(1));
      // Insert the loaded byte to LoadVal.
      // FIXME: Use INS if supported by target.
      unsigned ShiftAmt = isLittle ? 16 : 8;
      SDValue Shift = DAG.getNode(ISD::SHL, dl, MVT::i32, Subword,
                                  DAG.getConstant(ShiftAmt, MVT::i32));
      LoadVal = DAG.getNode(ISD::OR, dl, MVT::i32, LoadVal, Shift);
    }

    unsigned DstReg = O32IntRegs[LocMemOffset / 4];
    RegsToPass.push_back(std::make_pair(DstReg, LoadVal));
    return;
  }

  // Create a fixed object on stack at offset LocMemOffset and copy
  // remaining part of byval arg to it using memcpy.
  SDValue Src = DAG.getNode(ISD::ADD, dl, MVT::i32, Arg,
                            DAG.getConstant(Offset, MVT::i32));
  LastFI = MFI->CreateFixedObject(RemainingSize, LocMemOffset, true);
  SDValue Dst = DAG.getFrameIndex(LastFI, PtrType);
  ByValChain = DAG.getMemcpy(ByValChain, dl, Dst, Src,
                             DAG.getConstant(RemainingSize, MVT::i32),
                             std::min(ByValAlign, (unsigned)4),
                             /*isVolatile=*/false, /*AlwaysInline=*/false,
                             MachinePointerInfo(0), MachinePointerInfo(0));

  //std::cout << "Leaving WriteByValArg\n";
}

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: isTailCall.
SDValue
LE1TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                 SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  SDLoc &dl                             = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue InChain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  isTailCall = false;

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFL = MF.getTarget().getFrameLowering();

  // Position-independent code: a body of code that executes correctly
  // regardless of its absolute address.
  //bool IsPIC = getTargetMachine().getRelocationModel() == Reloc::PIC_;
  LE1FunctionInfo *LE1FI = MF.getInfo<LE1FunctionInfo>();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_LE1);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NextStackOffset = CCInfo.getNextStackOffset();

  // Chain is the output chain of the last Load/Store or CopyToReg node.
  // ByValChain is the output chain of the last Memcpy node created for copying
  // byval arguments to the stack.
  SDValue Chain, CallSeqStart, ByValChain;
  SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, true);
  Chain = CallSeqStart = DAG.getCALLSEQ_START(InChain, NextStackOffsetVal, dl);
  ByValChain = InChain;

  //std::cout << "Chain Op = " << Chain.getNode()->getOpcode() << std::endl;

  // If this is the first call, create a stack frame object that points to
  // a location to which .cprestore saves $gp.
  //if (IsPIC && !LE1FI->getGPFI())
    //LE1FI->setGPFI(MFI->CreateFixedObject(4, 0, true));

  // Get the frame index of the stack frame object that points to the location
  // of dynamically allocated area on the stack.
  int DynAllocFI = LE1FI->getDynAllocFI();

  // Update size of the maximum argument space.
  // For O32, a minimum of four words (16 bytes) of argument space is
  // allocated.
  
  unsigned MaxCallFrameSize = LE1FI->getMaxCallFrameSize();

  if (MaxCallFrameSize < NextStackOffset) {
    LE1FI->setMaxCallFrameSize(NextStackOffset);

    // Set the offsets relative to $sp of the $gp restore slot and dynamically
    // allocated stack space. These offsets must be aligned to a boundary
    // determined by the stack alignment of the ABI.
    unsigned StackAlignment = TFL->getStackAlignment();
    NextStackOffset = (NextStackOffset + StackAlignment - 1) /
                      StackAlignment * StackAlignment;

    //if (IsPIC)
      //MFI->setObjectOffset(LE1FI->getGPFI(), NextStackOffset);

    MFI->setObjectOffset(DynAllocFI, NextStackOffset);
  }

  // With EABI is it possible to have 16 args on registers.
  //SmallVector<std::pair<unsigned, SDValue>, 16> RegsToPass;
  
  // With LE1 it is possible to have 8 args on registers.
  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  int FirstFI = -MFI->getNumFixedObjects() - 1, LastFI = 0;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    SDValue Arg = OutVals[i];
    CCValAssign &VA = ArgLocs[i];

    // ByVal Arg.
    ISD::ArgFlagsTy Flags = Outs[i].Flags;
    if (Flags.isByVal()) {
      //assert(Subtarget->isABI_O32() &&
        //     "No support for ByVal args by ABIs other than O32 yet.");
      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");
      WriteByValArg(ByValChain, Chain, dl, RegsToPass, MemOpChains, LastFI, MFI,
                    DAG, Arg, VA, Flags, getPointerTy(), false);
      continue;
    }

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    }

    // Arguments that can be passed on register must be kept at
    // RegsToPass vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
      continue;
    }

    // Register can't get to this point...
    assert(VA.isMemLoc());


    // Create the frame index object for this incoming parameter
    LastFI = MFI->CreateFixedObject(VA.getValVT().getSizeInBits()/8,
                                    VA.getLocMemOffset(), true);
    SDValue PtrOff = DAG.getFrameIndex(LastFI, getPointerTy());

    // emit ISD::STORE whichs stores the
    // parameter value to a stack Location
    MemOpChains.push_back(DAG.getStore(Chain, dl, Arg, PtrOff,
                                       MachinePointerInfo(),
                                       false, false, 0));
  }

  // Extend range of indices of frame objects for outgoing arguments that were
  // created during this function call. Skip this step if no such objects were
  // created.
  if (LastFI)
    LE1FI->extendOutArgFIRange(FirstFI, LastFI);

  // If a memcpy has been created to copy a byval arg to a stack, replace the
  // chain input of CallSeqStart with ByValChain.
  if (InChain != ByValChain)
    DAG.UpdateNodeOperands(CallSeqStart.getNode(), ByValChain,
                           NextStackOffsetVal);

  // Transform all store nodes into one single node because all store
  // nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        &MemOpChains[0], MemOpChains.size());

  // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
  // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
  // node so that legalize doesn't hack it.

  //unsigned char OpFlag = IsPIC ? LE1II::MO_GOT_CALL : LE1II::MO_NO_FLAG;
  unsigned char OpFlag = 0; //LE1II::MO_NO_FLAG;

  bool LoadSymAddr = false;
  SDValue CalleeLo;

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    //if (IsPIC && G->getGlobal()->hasInternalLinkage()) {
      //Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl,
        //                                  getPointerTy(), 0, 0);
      //CalleeLo = DAG.getTargetGlobalAddress(G->getGlobal(), dl, getPointerTy(),
        //                                    0, LE1II::MO_ABS_LO);
    //} else {
      //std::cout << "GlobalAddress\n";
      Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl,
                                          getPointerTy(), 0, 0);
      //std::cout << "Callee is TargetGlobalAddress\n";
    //}

    LoadSymAddr = true;
  }
  else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(),
                                getPointerTy(), OpFlag);
    LoadSymAddr = true;
    //std::cout << "ExternalSymbol\n";
  }

  SDValue InFlag;
  if(!LoadSymAddr) {
    //std::cout << "not LoadSymAddr\n";
    //SDValue LoadValue = DAG.getLoad(MVT::i32, dl, DAG.getEntryNode(), Callee,
      //                              MachinePointerInfo::getGOT(),
        //                            false, false, false, 0);
    //Chain = DAG.getCopyToReg(Chain, dl, LE1::L0, LoadValue, SDValue(0,0));
    /*
    std::cout << "Callee NumOperands = " << Callee.getNumOperands() << std::endl;
    if(Callee.getOpcode() == ISD::CopyToReg)
      std::cout << "CopyToReg\n";
    else if(Callee.getOpcode() == ISD::LOAD &&
           (dyn_cast<GlobalAddressSDNode>(Callee.getNode()->getOperand(1)))) {
      std::cout << "Load\n";
      LoadSDNode *LoadNode = cast<LoadSDNode>(Callee.getNode());
      SDNode *Target = LoadNode->getBasePtr().getNode();
      std::cout << "got target\n";
      //SDValue Base = Target->getOperand(0);
      SDValue Base = LoadNode->getOperand(1);
      std::cout << "got base\n";
        int64_t Offset = cast<GlobalAddressSDNode>(Base)->getOffset();
        std::cout << "got offset\n";
        const GlobalValue *GV = cast<GlobalAddressSDNode>(Base)->getGlobal();
        std::cout << "got GV\n";
      SDValue TargetAddr = DAG.getTargetGlobalAddress(GV, dl,
                                                             getPointerTy(), 0);
      std::cout << "created TargetAddr\n";
      SDValue TargetOffset = DAG.getTargetConstant(Offset, getPointerTy());
      std::cout << "created TargetOffset\n";
      SDValue LoadLink = DAG.getNode(LE1ISD::LoadLink, dl,
                                     DAG.getVTList(MVT::i32, MVT::Other),
                                         TargetAddr, TargetOffset, Chain);
      std::cout << "created LoadLink\n";
      //MachineSDNode::mmo_iterator MemOp = MF.allocateMemRefsArray(1);
      //MemOp[0] = LoadNode->getMemOperand();
      //cast<MachineSDNode>(LoadLink)->setMemRefs(MemOp, MemOp+1);
      //DAG.ReplaceAllUsesOfValueWith(Callee, LoadLink);
      Callee = LoadLink;
    }*/
    //for(int i=0;i<Callee.getNumOperands();++i) {
      //if(Callee.getOperand(i).isTargetMemoryOpcode())
        //std::cout << "Operand " << i << " isTargetMemoryOpcode\n";
      //else if(Callee.getOperand(i).isTargetOpcode())
        //std::cout << "Operand " << i << " isTargetOpcode\n";
    //}
    //Callee.getNode()
    Chain = DAG.getCopyToReg(Chain, dl, LE1::L0, Callee, SDValue(0,0));
    //Callee = DAG.getNode(LE1ISD::MTL, dl, MVT::i32,
              //          DAG.getRegister(LE1::L0, MVT::i32), Callee);
    InFlag = Chain.getValue(1);
    Callee = DAG.getRegister(LE1::L0, MVT::i32);
  }

  // Create nodes that load address of callee and copy it to T9
  //if (IsPIC) {
    //std::cout << "IsPIC\n";
    //if (LoadSymAddr) {
      // Load callee address
      //std::cout << "Load callee address\n";
      //Callee = DAG.getNode(LE1ISD::WrapperPIC, dl, MVT::i32, Callee);
      //SDValue LoadValue = DAG.getLoad(MVT::i32, dl, DAG.getEntryNode(), Callee,
        //                              MachinePointerInfo::getGOT(),
          //                            false, false, 0);

      // Use GOT+LO if callee has internal linkage.
      //if (CalleeLo.getNode()) {
        //std::cout << "CalleeLo\n";
        //SDValue Lo = DAG.getNode(LE1ISD::Lo, dl, MVT::i32, CalleeLo);
        //Callee = DAG.getNode(ISD::ADD, dl, MVT::i32, LoadValue, Lo);
      //} else
        //Callee = LoadValue;
    //}

    // copy to T9
    //Chain = DAG.getCopyToReg(Chain, dl, LE1::T9, Callee, SDValue(0, 0));
    //InFlag = Chain.getValue(1);
    //Callee = DAG.getRegister(LE1::T9, MVT::i32);
  //}

  // Build a sequence of copy-to-reg nodes chained together with token
  // chain and flag operands which copy the outgoing args into registers.
  // The InFlag in necessary since all emitted instructions must be
  // stuck together.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // LE1Call = #chain, #link_register #target_address, #opt_in_flags...
  //             = Chain, LinkReg, Callee, Reg#1, Reg#2, ...
  //
  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  SDValue LinkReg = DAG.getRegister(LE1::L0, MVT::i32);
  Ops.push_back(LinkReg);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  if(LoadSymAddr)
    Chain  = DAG.getNode(LE1ISD::Call, dl, NodeTys,
                        &Ops[0], Ops.size());
  else
    Chain = DAG.getNode(LE1ISD::Call, dl, NodeTys, &Ops[0], Ops.size());

  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getIntPtrConstant(NextStackOffset, true),
                             DAG.getIntPtrConstant(0, true), InFlag, dl);
  InFlag = Chain.getValue(1);

  //std::cout << "Leaving LE1TargetLowering::LowerCall\n";
  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg,
                         Ins, dl, DAG, InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
LE1TargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                    CallingConv::ID CallConv, bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    SDLoc dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const {
  //std::cout << "Entering LE1TargetLowering::LowerCallResult\n";
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_LE1);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  //std::cout << "Leaving LE1TargetLowering::LowerCallResult\n";
  return Chain;
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//
static void ReadByValArg(MachineFunction &MF, SDValue Chain, SDLoc dl,
                         std::vector<SDValue>& OutChains,
                         SelectionDAG &DAG, unsigned NumWords, SDValue FIN,
                         const CCValAssign &VA, const ISD::ArgFlagsTy& Flags) {
  // FIXME shouldn't have to copy values into the frame
  //std::cout << "Entering ReadByValArg\n";
  unsigned LocMem = VA.getLocMemOffset();
  unsigned FirstWord = LocMem / 4;

  // copy register A0 - A3 to frame object
  // copy register AR0 - AR7 to frame object
  for (unsigned i = 0; i < NumWords; ++i) {
    unsigned CurWord = FirstWord + i;
    if (CurWord >= O32IntRegsSize)
      break;

    unsigned SrcReg = O32IntRegs[CurWord];
    unsigned Reg = AddLiveIn(MF, SrcReg, &LE1::CPURegsRegClass);
    SDValue StorePtr = DAG.getNode(ISD::ADD, dl, MVT::i32, FIN,
                                   DAG.getConstant(i * 4, MVT::i32));
    SDValue Store = DAG.getStore(Chain, dl, DAG.getRegister(Reg, MVT::i32),
                                 StorePtr, MachinePointerInfo(), false,
                                 false, 0);
    OutChains.push_back(Store);
  }
  //std::cout << "Leaving ReadByValArg\n";
}

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue
LE1TargetLowering::LowerFormalArguments(SDValue Chain,
                                        CallingConv::ID CallConv,
                                        bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                        SDLoc dl, SelectionDAG &DAG,
                                      SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  LE1FunctionInfo *LE1FI = MF.getInfo<LE1FunctionInfo>();

  LE1FI->setVarArgsFrameIndex(0);

  // Used with vargs to acumulate store chains.
  std::vector<SDValue> OutChains;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  //if (IsO32)
    //CCInfo.AnalyzeFormalArguments(Ins, CC_MipsO32);
  //else
    //CCInfo.AnalyzeFormalArguments(Ins, CC_Mips);
  CCInfo.AnalyzeFormalArguments(Ins, CC_LE1);
  int LastFI = 0;// MipsFI->LastInArgFI is 0 at the entry of this function.
  //std::cout << "ArgLocs.size = " << ArgLocs.size() << std::endl;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    EVT ValVT = VA.getValVT();
    ISD::ArgFlagsTy Flags = Ins[i].Flags;
    bool IsRegLoc = VA.isRegLoc();

    if (Flags.isByVal() && !IsRegLoc) {
      //std::cout << "isByVal\n";
      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");
      //if (IsO32) {
        unsigned NumWords = (Flags.getByValSize() + 3) / 4;
        LastFI = MFI->CreateFixedObject(NumWords * 4, VA.getLocMemOffset(),
                                        true);
        SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
        InVals.push_back(FIN);
        ReadByValArg(MF, Chain, dl, OutChains, DAG, NumWords, FIN, VA, Flags);
      //} else // N32/64
        //LastFI = CopyMips64ByValRegs(MF, Chain, dl, OutChains, DAG, VA, Flags,
          //                           MFI, IsRegLoc, InVals, MipsFI,
            //                         getPointerTy());
      continue;
    }

    // Arguments stored on registers
    if (IsRegLoc) {
      //std::cout << "isRegLoc\n";
      EVT RegVT = VA.getLocVT();
      unsigned ArgReg = VA.getLocReg();
      const TargetRegisterClass *RC = &LE1::CPURegsRegClass;

      //if (RegVT == MVT::i32)
        //RC = Mips::CPURegsRegisterClass;
      //else if (RegVT == MVT::i64)
        //RC = Mips::CPU64RegsRegisterClass;
      //else if (RegVT == MVT::f32)
        //RC = Mips::FGR32RegisterClass;
      //else if (RegVT == MVT::f64)
        //RC = HasMips64 ? Mips::FGR64RegisterClass : Mips::AFGR64RegisterClass;
      //else
        //llvm_unreachable("RegVT not supported by FormalArguments Lowering");

      // Transform the arguments stored on
      // physical registers into virtual ones
      unsigned Reg = AddLiveIn(DAG.getMachineFunction(), ArgReg, RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);

      // If this is an 8 or 16-bit value, it has been passed promoted
      // to 32 bits.  Insert an assert[sz]ext to capture this, then
      // truncate to the right size.
      if (VA.getLocInfo() != CCValAssign::Full) {
        unsigned Opcode = 0;
        if (VA.getLocInfo() == CCValAssign::SExt)
          Opcode = ISD::AssertSext;
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          Opcode = ISD::AssertZext;
        if (Opcode)
          ArgValue = DAG.getNode(Opcode, dl, RegVT, ArgValue,
                                 DAG.getValueType(ValVT));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, ValVT, ArgValue);
      }

      // Handle floating point arguments passed in integer registers.
      //if ((RegVT == MVT::i32 && ValVT == MVT::f32) ||
        //  (RegVT == MVT::i64 && ValVT == MVT::f64))
        //ArgValue = DAG.getNode(ISD::BITCAST, dl, ValVT, ArgValue);
      //else if (IsO32 && RegVT == MVT::i32 && ValVT == MVT::f64) {
        //unsigned Reg2 = AddLiveIn(DAG.getMachineFunction(),
          //                        getNextIntArgReg(ArgReg), RC);
        //SDValue ArgValue2 = DAG.getCopyFromReg(Chain, dl, Reg2, RegVT);
        //if (!Subtarget->isLittle())
          //std::swap(ArgValue, ArgValue2);
        //ArgValue = DAG.getNode(MipsISD::BuildPairF64, dl, MVT::f64,
          //                     ArgValue, ArgValue2);
      //}

      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()

      // sanity check
      assert(VA.isMemLoc());

      // The stack pointer offset is relative to the caller stack frame.
      LastFI = MFI->CreateFixedObject(ValVT.getSizeInBits()/8,
                                      VA.getLocMemOffset(), true);

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
      InVals.push_back(DAG.getLoad(ValVT, dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(LastFI),
                                   false, false, false, 0));
    }
  }

  // The mips ABIs for returning structs by value requires that we copy
  // the sret argument into $v0 for the return. Save the argument into
  // a virtual register so that we can access it from the return points.
  if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
    unsigned Reg = LE1FI->getSRetReturnReg();
    if (!Reg) {
      Reg = MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
      LE1FI->setSRetReturnReg(Reg);
    }
    SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), dl, Reg, InVals[0]);
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Copy, Chain);
  }

  if (isVarArg) {
    //unsigned NumOfRegs = IsO32 ? 4 : 8;
    unsigned NumOfRegs = 8;
    //const unsigned *ArgRegs = IsO32 ? O32IntRegs : Mips64IntRegs;
    const uint16_t *ArgRegs = O32IntRegs;
    unsigned Idx = CCInfo.getFirstUnallocated(O32IntRegs, NumOfRegs);
    //int FirstRegSlotOffset = IsO32 ? 0 : -64 ; // offset of $a0's slot.
    int FirstRegSlotOffset = 0;
    const TargetRegisterClass *RC = &LE1::CPURegsRegClass;
    //  = IsO32 ? Mips::CPURegsRegisterClass : Mips::CPU64RegsRegisterClass;
    unsigned RegSize = RC->getSize();
    int RegSlotOffset = FirstRegSlotOffset + Idx * RegSize;

    // Offset of the first variable argument from stack pointer.
    int FirstVaArgOffset;

    //if (IsO32 || (Idx == NumOfRegs)) {
    if(Idx == NumOfRegs) {
      FirstVaArgOffset =
        (CCInfo.getNextStackOffset() + RegSize - 1) / RegSize * RegSize;
    } else
      FirstVaArgOffset = RegSlotOffset;

    // Record the frame index of the first variable argument
    // which is a value necessary to VASTART.
    LastFI = MFI->CreateFixedObject(RegSize, FirstVaArgOffset, true);
    LE1FI->setVarArgsFrameIndex(LastFI);

    // Copy the integer registers that have not been used for argument passing
    // to the argument register save area. For O32, the save area is allocated
    // in the caller's stack frame, while for N32/64, it is allocated in the
    // callee's stack frame.
    for (int StackOffset = RegSlotOffset;
         Idx < NumOfRegs; ++Idx, StackOffset += RegSize) {
      unsigned Reg = AddLiveIn(DAG.getMachineFunction(), ArgRegs[Idx], RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg,
                                            MVT::getIntegerVT(RegSize * 8));
      LastFI = MFI->CreateFixedObject(RegSize, StackOffset, true);
      SDValue PtrOff = DAG.getFrameIndex(LastFI, getPointerTy());
      OutChains.push_back(DAG.getStore(Chain, dl, ArgValue, PtrOff,
                                       MachinePointerInfo(), false, false, 0));
    }
  }

  LE1FI->setLastInArgFI(LastFI);

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens when on varg functions
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        &OutChains[0], OutChains.size());
  }
  //std::cout << "Leaving LowerFormalArguments\n";
  return Chain;
}

/*
SDValue
LE1TargetLowering::LowerFormalArguments(SDValue Chain,
                                         CallingConv::ID CallConv,
                                         bool isVarArg,
                                         const SmallVectorImpl<ISD::InputArg>
                                         &Ins,
                                         DebugLoc dl, SelectionDAG &DAG,
                                         SmallVectorImpl<SDValue> &InVals)
                                          const {
  std::cout << "Entering LE1TargetLowering::LowerFormalArguments\n";
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  LE1FunctionInfo *LE1FI = MF.getInfo<LE1FunctionInfo>();

  LE1FI->setVarArgsFrameIndex(0);

  // Used with vargs to acumulate store chains.
  std::vector<SDValue> OutChains;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;

  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_LE1);

  int LastFI = 0;// LE1FI->LastInArgFI is 0 at the entry of this function.
  std::cout << "ArgLocs.size = " << ArgLocs.size() << std::endl;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    // Arguments stored on registers
    //if (VA.isRegLoc()) {
    if(i < 8) {
      EVT RegVT = VA.getLocVT();
      unsigned ArgReg = VA.getLocReg();
      if (RegVT != MVT::i32)
        llvm_unreachable("RegVT not supported by FormalArguments Lowering");

      const TargetRegisterClass *RC = LE1::CPURegsRegisterClass;

      //if (RegVT == MVT::i32)
        //RC = LE1::CPURegsRegisterClass;
      //else
        //llvm_unreachable("RegVT not supported by FormalArguments Lowering");

      // Transform the arguments stored on
      // physical registers into virtual ones
      unsigned Reg = AddLiveIn(DAG.getMachineFunction(), ArgReg, RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);

      // If this is an 8 or 16-bit value, it has been passed promoted
      // to 32 bits.  Insert an assert[sz]ext to capture this, then
      // truncate to the right size.
      if (VA.getLocInfo() != CCValAssign::Full) {
        unsigned Opcode = 0;
        if (VA.getLocInfo() == CCValAssign::SExt)
          Opcode = ISD::AssertSext;
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          Opcode = ISD::AssertZext;
        if (Opcode)
          ArgValue = DAG.getNode(Opcode, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
      }

      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()

      // sanity check
      assert(VA.isMemLoc());

      ISD::ArgFlagsTy Flags = Ins[i].Flags;

      if (Flags.isByVal()) {
        //assert(Subtarget->isABI_O32() &&
          //     "No support for ByVal args by ABIs other than O32 yet.");
        assert(Flags.getByValSize() &&
               "ByVal args of size 0 should have been ignored by front-end.");
        unsigned NumWords = (Flags.getByValSize() + 3) / 4;
        LastFI = MFI->CreateFixedObject(NumWords * 4, VA.getLocMemOffset(),
                                        true);
        SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
        InVals.push_back(FIN);
        ReadByValArg(MF, Chain, dl, OutChains, DAG, NumWords, FIN, VA, Flags);

        continue;
      }

      // The stack pointer offset is relative to the caller stack frame.
      LastFI = MFI->CreateFixedObject(VA.getValVT().getSizeInBits()/8,
                                      VA.getLocMemOffset(), true);

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
      InVals.push_back(DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(LastFI),
                                   false, false, false, 0));
    }
  }

  // The le1 ABIs for returning structs by value requires that we copy
  // the sret argument into $v0 for the return. Save the argument into
  // a virtual register so that we can access it from the return points.
  if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
    unsigned Reg = LE1FI->getSRetReturnReg();
    if (!Reg) {
      Reg = MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
      LE1FI->setSRetReturnReg(Reg);
    }
    SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), dl, Reg, InVals[0]);
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Copy, Chain);
  }

  LE1FI->setLastInArgFI(LastFI);

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens when on varg functions
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        &OutChains[0], OutChains.size());
  }

  //std::cout << "Leaving LE1TargetLowering::LowerFormalArguments\n";
  return Chain;
}
*/
//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
LE1TargetLowering::LowerReturn(SDValue Chain,
                                CallingConv::ID CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                SDLoc dl, SelectionDAG &DAG) const {
  //std::cout << "Entering LE1TargetLowering::LowerReturn\n";
  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), RVLocs, *DAG.getContext());

  // Analysis return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_LE1);

  // If this is the first return lowered for this function, add
  // the regs to the liveout set for the function.
  /*
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
    for (unsigned i = 0; i != RVLocs.size(); ++i)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }*/

  SDValue Flag;

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    //assert(VA.isRegLoc() && "Can only return in registers!");

    // FIXME - This isn't necessarily a register now!
    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
                             OutVals[i], Flag);

    // guarantee that all emitted copies are
    // stuck together, avoiding something bad
    Flag = Chain.getValue(1);
  }

  // The le1 ABIs for returning structs by value requires that we copy
  // the sret argument into $v0 for the return. We saved the argument into
  // a virtual register in the entry block, so now we copy the value out
  // and into $v0.
  if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
    MachineFunction &MF      = DAG.getMachineFunction();
    LE1FunctionInfo *LE1FI = MF.getInfo<LE1FunctionInfo>();
    unsigned Reg = LE1FI->getSRetReturnReg();

    if (!Reg)
      llvm_unreachable("sret virtual register not created in the entry block");
    SDValue Val = DAG.getCopyFromReg(Chain, dl, Reg, getPointerTy());

    Chain = DAG.getCopyToReg(Chain, dl, LE1::STRP, Val, Flag);
    Flag = Chain.getValue(1);
  }

 
  //std::cout << "Leaving LE1TargetLowering::LowerReturn\n";
  if (Flag.getNode())
    return DAG.getNode(LE1ISD::Ret, dl, MVT::Other,
                       Chain, 
                       DAG.getRegister(LE1::SP, MVT::i32), 
                       DAG.getConstant(0, MVT::i32),
                       DAG.getRegister(LE1::L0, MVT::i32), 
                       Flag);

  return DAG.getNode(LE1ISD::Ret, dl, MVT::Other, Chain,
                     DAG.getRegister(LE1::SP, MVT::i32),
                     DAG.getConstant(0, MVT::i32),
                     DAG.getRegister(LE1::L0, MVT::i32));

}
