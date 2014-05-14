// TODO Analyse the kernels and the input data to figure out what WBITS should
// be. I think the only proper options are 24, 16, or 8, but I can't see why I
// couldn't use other formats. This parameter could get passed to the subtarget
// on the command line at final compile time.
#define FIXEDPT_BITS	32
#define FIXEDPT_WBITS	24
#define FIXEDPT_FBITS	(FIXEDPT_BITS - FIXEDPT_WBITS)
#define FIXEDPT_FMASK	(((fixedpt)1 << FIXEDPT_FBITS) - 1)
#define FIXEDPT_ONE	((fixedpt)((fixedpt)1 << FIXEDPT_FBITS))
#define FIXEDPT_ONE_HALF (FIXEDPT_ONE >> 1)
#define FIXEDPT_TWO	(FIXEDPT_ONE + FIXEDPT_ONE)
#define FIXEDPT_PI	fixedpt_rconst(3.14159265358979323846)
#define FIXEDPT_TWO_PI	fixedpt_rconst(2 * 3.14159265358979323846)
#define FIXEDPT_HALF_PI	fixedpt_rconst(3.14159265358979323846 / 2)
#define FIXEDPT_E	fixedpt_rconst(2.7182818284590452354)

typedef __int32_t fixedpt;
typedef	__int64_t	fixedptd;
typedef	__uint32_t fixedptu;
typedef	__uint64_t fixedptud;

//#define fixedpt_rconst(R) \
  ((fixedpt)((R) * FIXEDPT_ONE + ((R) >= 0 ? 0.5 : -0.5)))
SDValue LE1FixedPoint::RConst(ConstantSDNode *CSN, SelectionDAG &DAG) const {

}

//#define fixedpt_fromint(I) ((fixedptd)(I) << FIXEDPT_FBITS)
SDValue LE1FixedPoint::FromInt(SDValue Op) const {

}

//#define fixedpt_toint(F) ((F) >> FIXEDPT_FBITS)
SDValue LE1FixedPoint::ToInt(SDValue Op) const {
}

//#define fixedpt_add(A,B) ((A) + (B))
SDValue LE1FixedPoint::Add(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1):
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(RHS))
    RHS = RConst(CSN);

  return DAG.getNode(ISD::ADD, dl, MVT::i32, LHS, RHS);
}

//#define fixedpt_sub(A,B) ((A) - (B))
SDValue LE1FixedPoint::Sub(SDValue Op, SelectionDAG) const {
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1):
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(RHS))
    RHS = RConst(CSN);
  else if (RHS.getValueType() == MVT::f32)
    RHS = From

  return DAG.getNode(ISD::SUB, dl, MVT::i32, LHS, RHS);
}

//#define fixedpt_xmul(A,B)						\
	((fixedpt)(((fixedptd)(A) * (fixedptd)(B)) >> FIXEDPT_FBITS))
SDValue LE1FixedPoint::Mul(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1):
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(RHS))
    RHS = RConst(CSN);
}

//#define fixedpt_xdiv(A,B)						\
	((fixedpt)(((fixedptd)(A) << FIXEDPT_FBITS) / (fixedptd)(B)))
SDValue LE1FixedPoint::Div(SDValue Op0, SDValue Op1) const {
  SDLoc dl(Op.getNode());
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1):
  if (ConstantSDNode *CSN = dyn_cast<ConstantSDNode>(RHS))
    RHS = RConst(CSN);
}
