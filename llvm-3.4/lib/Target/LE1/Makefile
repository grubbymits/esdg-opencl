##===- lib/Target/LE1/Makefile ----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMLE1CodeGen
TARGET = LE1

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = LE1GenRegisterInfo.inc LE1GenInstrInfo.inc \
                LE1GenAsmWriter.inc LE1GenCodeEmitter.inc \
                LE1GenDAGISel.inc LE1GenCallingConv.inc \
                LE1GenSubtargetInfo.inc LE1GenDFAPacketizer.inc

DIRS = InstPrinter TargetInfo MCTargetDesc MachineModels

include $(LEVEL)/Makefile.common

