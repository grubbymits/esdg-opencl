//===-- LE1BaseInfo.h - Top level definitions for ARM ------- --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the LE1 target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LE1BASEINFO_H
#define LE1BASEINFO_H

#include "LE1MCTargetDesc.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {
/// getLE1RegisterNumbering - Given the enum value for some register,
/// return the number that it corresponds to.
inline static unsigned getLE1RegisterNumbering(unsigned RegEnum)
{
  switch (RegEnum) {
  default: llvm_unreachable("Unknown register number!");
  }
  return 0; // Not reached
}
}

#endif
