add_llvm_library(LLVMLE1Desc
  LE1MCAsmInfo.cpp
  LE1MCCodeEmitter.cpp
  LE1MCTargetDesc.cpp
  )

add_dependencies(LLVMLE1Desc LE1CommonTableGen)
