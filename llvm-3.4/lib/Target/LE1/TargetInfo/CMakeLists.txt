include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMLE1Info
  LE1TargetInfo.cpp
  )

add_dependencies(LLVMLE1Info LE1CommonTableGen)
