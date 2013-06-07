#!/bin/sh

clang -cc1 -x cl -I/opt/esdg-opencl/include -I/usr/local/lib/clang/3.2/include \
-Dcl_clang_storage_class_specifiers -ast-dump-xml $1 -o ${1}.xml
