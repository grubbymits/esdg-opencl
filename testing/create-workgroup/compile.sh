#!/bin/sh
# Script to take an OpenCL kernel, compile it into LLVM bytecode and then
# replicate several work-items into a single instance and produce assembly
# output for the LE1.

# Compile the .cl kernel file
#clang -ccc-host-triple le1 -c -emit-llvm \
# -I/home/sam/src/le1-opencl-compiler/pocl-0.6 \
# -include /home/sam/src/le1-opencl-compiler/pocl-0.6/include/le1/types.h \
# -include /home/sam/src/le1-opencl-compiler/pocl-0.6/include/_kernel.h \
# -o kernel.bc -x cl $1
clang -ccc-host-triple le1-llvm-none -emit-llvm -c \
      -I/home/sam/src/le1-opencl-compiler/libclc/le1/include \
      -I/home/sam/src/le1-opencl-compiler/libclc/generic/include \
      -Xclang -mlink-bitcode-file \
      -Xclang /home/sam/src/le1-opencl-compiler/libclc/le1-llvm-none/lib/builtins.bc \
      -include clc/clc.h -Dcl_clang_storage_class_specifiers $1 -o kernel.bc

# Link the compiled kernel with the OpenCL library
#llvm-ld -o linked.bc.out -b linked.bc kernel.bc ~/src/le1-opencl-compiler/libclc/le1-llvm-none/lib/workitem/workitem.bc

#llvm-ld -o linked.bc.out -b linked.bc kernel.bc -L/usr/local/lib/pocl/le1/ \
 #       -lkernel

rm linked.bc.out



