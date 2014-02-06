include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMLE1AsmPrinter
  LE1InstPrinter.cpp
  )

add_dependencies(LLVMLE1AsmPrinter LE1CommonTableGen)
