#!/bin/sh
pocl_lib=/usr/local/lib/pocl/llvmopencl.so
linked_bc=$1
size_x=3
size_y=1
size_z=1
output_file=final.bc

/usr/local/bin/opt -load ${pocl_lib} -flatten -always-inline \
    -globaldce -simplifycfg -loop-simplify -isolate-regions -loop-barriers \
    -barriertails -barriers -isolate-regions -add-wi-metadata -wi-aa -workitem \
    -kernel=${kernel} -local-size=${size_x} ${size_y} ${size_z} \
    -o ${output_file} ${linked_bc}

