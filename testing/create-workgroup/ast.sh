
clang -ccc-host-triple le1-llvm-none -ast-print -c \
      -I/home/sam/src/le1-opencl-compiler/libclc/le1/include \
      -I/home/sam/src/le1-opencl-compiler/libclc/generic/include \
      -Xclang -mlink-bitcode-file \
      -Xclang /home/sam/src/le1-opencl-compiler/libclc/le1-llvm-none/lib/builtins.bc \
      -include clc/clc.h -Dcl_clang_storage_class_specifiers $1 -o ast.bc
