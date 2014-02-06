//===-- LE1TargetInfo.cpp - LE1 Target Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LE1.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheLE1Target; 

extern "C" void LLVMInitializeLE1TargetInfo() {
  RegisterTarget<Triple::le1> X(TheLE1Target, "le1", "LE1");

}
