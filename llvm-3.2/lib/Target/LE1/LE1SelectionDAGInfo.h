//===-- LE1SelectionDAGInfo.h - LE1 SelectionDAG Info ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the LE1 subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LE1SELECTIONDAGINFO_H
#define LE1SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class LE1TargetMachine;

class LE1SelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit LE1SelectionDAGInfo(const LE1TargetMachine &TM);
  ~LE1SelectionDAGInfo();
};

}

#endif
