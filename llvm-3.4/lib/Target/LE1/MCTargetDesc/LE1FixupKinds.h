#ifndef LLVM_LE1_LE1FIXUPKINDS_H
#define LLVM_LE1_LE1FIXUPKINDS_H

//===-- LE1/LE1FixupKinds.h - LE1 Specific Fixup Entries --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace LE1 {
    enum Fixups {
        // fixup_LE1_xxx - R_LE1_NONE
        fixup_LE1_NONE = FirstTargetFixupKind,

        // fixup_LE1_xxx - R_LE1_16.
        fixup_LE1_16,

        // fixup_LE1_xxx - R_LE1_32.
        fixup_LE1_32,

        // fixup_LE1_xxx - R_LE1_REL32.
        fixup_LE1_REL32,

        // fixup_LE1_xxx - R_LE1_26.
        fixup_LE1_26,

        // fixup_LE1_xxx - R_LE1_HI16.
        fixup_LE1_HI16,

        // fixup_LE1_xxx - R_LE1_LO16.
        fixup_LE1_LO16,

        // fixup_LE1_xxx - R_LE1_GPREL16.
        fixup_LE1_GPREL16,

        // fixup_LE1_xxx - R_LE1_LITERAL.
        fixup_LE1_LITERAL,

        // fixup_LE1_xxx - R_LE1_GOT16.
        fixup_LE1_GOT16,

        // fixup_LE1_xxx - R_LE1_PC16.
        fixup_LE1_PC16,

        // fixup_LE1_xxx - R_LE1_CALL16.
        fixup_LE1_CALL16,

        // fixup_LE1_xxx - R_LE1_GPREL32.
        fixup_LE1_GPREL32,

        // fixup_LE1_xxx - R_LE1_SHIFT5.
        fixup_LE1_SHIFT5,

        // fixup_LE1_xxx - R_LE1_SHIFT6.
        fixup_LE1_SHIFT6,

        // fixup_LE1_xxx - R_LE1_64.
        fixup_LE1_64,

        // fixup_LE1_xxx - R_LE1_TLS_GD.
        fixup_LE1_TLSGD,

        // fixup_LE1_xxx - R_LE1_TLS_GOTTPREL.
        fixup_LE1_GOTTPREL,

        // fixup_LE1_xxx - R_LE1_TLS_TPREL_HI16.
        fixup_LE1_TPREL_HI,

        // fixup_LE1_xxx - R_LE1_TLS_TPREL_LO16.
        fixup_LE1_TPREL_LO,

        // fixup_LE1_xxx - yyy. // This should become R_LE1_PC16
        fixup_LE1_Branch_PCRel,

        // Marker
        LastTargetFixupKind,
        NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
    };
} // namespace llvm
} // namespace LE1


#endif /* LLVM_LE1_LE1FIXUPKINDS_H */
