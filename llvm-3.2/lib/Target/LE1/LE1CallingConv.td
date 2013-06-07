//===- LE1CallingConv.td - Calling Conventions LE1 -----*- tablegen -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the LE1 architectures.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

def RetCC_LE1 : CallingConv<[
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,
  CCIfType<[i32], CCAssignToReg<[AR0, AR1, AR2, AR3, AR4, AR5, AR6, AR7]>>,
  CCIfType<[i32], CCAssignToStack<4, 32>>
]>;

def CC_LE1 : CallingConv<[
  //Promote i8/i16 arguments to i32
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,

  //The same registers are used for both arguments and return values.
  CCIfType<[i32], CCAssignToReg<[AR0, AR1, AR2, AR3, AR4, AR5, AR6, AR7]>>,
  
  //Stack is assigned that in 4 byte slot, 32 byte aligned
  CCIfType<[i32], CCAssignToStack<4, 32>>
]>; 
