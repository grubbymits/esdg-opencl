#!/bin/sh
kernel=array_mult
output_file=output.bc
linked_bc=linked.bc

/usr/local/bin/opt -load=/opt/esdg-opencl/lib/llvmopencl.so -domtree -workitem-handler-chooser -break-constgeps -generate-header -flatten -always-inline \
    -globaldce -simplifycfg -loop-simplify -phistoallocas -isolate-regions -loop-barriers \
    -barriertails -barriers -isolate-regions -add-wi-metadata -wi-aa -workitemrepl -workitemloops \
    -workgroup -kernel=${kernel} -local-size=4 1 1 \
    ${EXTRA_OPTS} -instcombine -header=/dev/null -o ${output_file} ${linked_bc}
