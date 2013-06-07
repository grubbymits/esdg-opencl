CXX = clang++
CFLAGS = -fno-rtti -fPIC

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
TARGET = libWorkitemCoarsen.so
CFLAGS += -shared
LLVM_SRC_PATH = /home/sam/src/le1-clc/llvm-3.2
LLVM_BUILD_PATH = /home/sam/src/build-llvm
endif

ifeq ($(UNAME), Darwin)
TARGET = libWorkitemCoarsen.dylib
CFLAGS += -dynamiclib
LLVM_SRC_PATH = /Users/sam/src/le1-clc/llvm-3.2
LLVM_BUILD_PATH = /User/sam/src/build-llvm
endif


LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin
LLVM_LIBS=core mc
LLVM_CONFIG_COMMAND = llvm-config --cxxflags --ldflags \
                                        --libs $(LLVM_LIBS)
CLANG_BUILD_FLAGS = -I$(LLVM_SRC_PATH)/tools/clang/include \
                                      -I$(LLVM_BUILD_PATH)/tools/clang/include \
				      -I/usr/include/x86_64-linux-gnu/c++/4.7

CLANGLIBS = \
  -L/usr/local/lib -lclangFrontendTool -lclangFrontend -lclangDriver \
  -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangSema -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore \
  -lclangAnalysis -lclangARCMigrate -lclangRewriteCore \
  -lclangEdit -lclangAST -lclangLex -lclangBasic

LLVMLIBS = \
  -L/usr/local/lib -lLLVMAsmParser -lLLVMMCParser -lLLVMInstrumentation \
  -lLLVMInterpreter -lLLVMCodeGen -lLLVMipo -lLLVMVectorize -lLLVMScalarOpts \
  -lLLVMInstCombine -lLLVMLinker -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis \
  -lLLVMArchive -lLLVMBitReader -lLLVMBitWriter -lLLVMMCJIT -lLLVMRuntimeDyld \
  -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore \
  -lLLVMSupport

all: $(TARGET)

$(TARGET): WorkitemCoarsen.cpp
	$(CXX) $< $(CFLAGS) -o $@ \
	$(CLANG_BUILD_FLAGS) $(LLVMLIBS) $(CLANGLIBS) `$(LLVM_CONFIG_COMMAND)`

clean:
	rm -rf *.o *.ll $(TARGET)
