//=====-- LE1MCAsmInfo.h - LE1 asm properties ---------------*- C++ -*--====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the LE1MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LE1TARGETASMINFO_H
#define LE1TARGETASMINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {

  class LE1MCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit LE1MCAsmInfo(const MCRegisterInfo &MRI, StringRef TT);
  };

} // namespace llvm

#endif
