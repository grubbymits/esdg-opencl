//===-- LE1TargetMachine.h - Define TargetMachine for LE1 -00--*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the LE1 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LE1TARGETMACHINE_H
#define LE1TARGETMACHINE_H

#include "LE1Subtarget.h"
#include "LE1InstrInfo.h"
#include "LE1ISelLowering.h"
#include "LE1FrameLowering.h"
#include "LE1SelectionDAGInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetFrameLowering.h"
//#include "LE1JITInfo.h"
#include <iostream>

namespace llvm {
  class formatted_raw_ostream;

  class LE1TargetMachine : public LLVMTargetMachine {
    const DataLayout DL; // Calculates type size & alignment
    LE1Subtarget       Subtarget;
    LE1InstrInfo       InstrInfo;
    LE1TargetLowering  TLInfo;
    LE1SelectionDAGInfo TSInfo;
    LE1FrameLowering   FrameLowering;
    const InstrItineraryData* InstrItins;
    //LE1JITInfo JITInfo;

  public:
    LE1TargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);

    virtual void addAnalysisPasses(PassManagerBase &PM);

    virtual bool addPassesForOptimizations(PassManagerBase &PM);

    virtual const LE1InstrInfo   *getInstrInfo()     const
    { return &InstrInfo; }
    virtual const TargetFrameLowering *getFrameLowering()     const
    { return &FrameLowering; }
    virtual const LE1Subtarget   *getSubtargetImpl() const
    { return &Subtarget; }
    virtual const DataLayout *getDataLayout()    const
    { return &DL ;}
    //virtual LE1JITInfo *getJITInfo()
    //{ return &JITInfo; }
    virtual const InstrItineraryData *getInstrItineraryData() const
    { return InstrItins; }

    virtual const LE1RegisterInfo *getRegisterInfo()  const {
      return &InstrInfo.getRegisterInfo();
    }

    virtual const LE1TargetLowering *getTargetLowering() const {
      return &TLInfo;
    }

    virtual const LE1SelectionDAGInfo* getSelectionDAGInfo() const {
      return &TSInfo;
    }

    virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);

  };
} // End llvm namespace

#endif
