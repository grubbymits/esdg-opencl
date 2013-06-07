//===-- LE1SelectionDAGInfo.cpp - LE1 SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LE1SelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "le1-selectiondag-info"
#include "LE1TargetMachine.h"
using namespace llvm;

LE1SelectionDAGInfo::LE1SelectionDAGInfo(const LE1TargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

LE1SelectionDAGInfo::~LE1SelectionDAGInfo() {
}
