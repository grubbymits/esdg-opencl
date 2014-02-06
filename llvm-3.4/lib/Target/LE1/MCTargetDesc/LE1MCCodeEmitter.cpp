//===-- LE1MCCodeEmitter.cpp - Convert LE1 code to machine code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LE1MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//
#define DEBUG_TYPE "mccodeemitter"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"
#include "MCTargetDesc/LE1MCTargetDesc.h"

using namespace llvm;

namespace {
class LE1MCCodeEmitter : public MCCodeEmitter {
  LE1MCCodeEmitter(const LE1MCCodeEmitter &); // DO NOT IMPLEMENT
  void operator=(const LE1MCCodeEmitter &); // DO NOT IMPLEMENT
  const MCInstrInfo &MCII;
  const MCSubtargetInfo &STI;

public:
  LE1MCCodeEmitter(const MCInstrInfo &mcii, const MCSubtargetInfo &sti,
                   MCContext &ctx)
    : MCII(mcii), STI(sti) {}

  ~LE1MCCodeEmitter() {}

  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const {
  }
}; // class LE1MCCodeEmitter
}  // namespace

MCCodeEmitter *llvm::createLE1MCCodeEmitter(const MCInstrInfo &MCII,
                                            const MCRegisterInfo &MRI,
                                            const MCSubtargetInfo &STI,
                                            MCContext &Ctx) {
  return new LE1MCCodeEmitter(MCII, STI, Ctx);
}
