//===- LE1Subtarget.cpp - LE1 Subtarget Information -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LE1 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "LE1Subtarget.h"
#include "LE1.h"
#include "llvm/Support/TargetRegistry.h"

#include <iostream>
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "LE1GenSubtargetInfo.inc"

using namespace llvm;

LE1Subtarget::LE1Subtarget(const std::string &TT, const std::string &CPU,
                             const std::string &FS) :// bool little) :
  LE1GenSubtargetInfo(TT, CPU, FS),
  HasExpandDiv(false),
  NeedsNops(false)
{
  //std::cout << "Subtarget being created\n";
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "scalar";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);
  //std::cout << "SubtargetFeatures parsed\n";
  //InstrItins = getInstrItineraryForCPU(CPUName);
  //SchedModel = getSchedModelForCPU(CPUName);

  // Initialize scheduling itinerary for the specified CPU.
  switch(CPU[0]) {
    case '1':
      issueWidth = 1;
      break;
    case '2':
      issueWidth = 2;
      break;
    case '3':
      issueWidth = 3;
      break;
    case '4':
      issueWidth = 4;
      break;
    case '5':
      issueWidth = 5;
      break;
    default:
      issueWidth = 1;
      break;
    }
}
