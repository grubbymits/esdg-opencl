CXX = clang++
CFLAGS = -fno-rtti

LLVM_SRC_PATH = /Users/sam/src/le1-clc/llvm-3.2
LLVM_BUILD_PATH = /User/sam/src/build-llvm

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
  -lclangAnalysis -lclangARCMigrate -lLLVMMCParser -lclangRewriteCore \
  -lclangEdit -lclangAST -lclangLex -lclangBasic

all: rewritersample

rewritersample: rewritersample.cpp
	$(CXX) rewritersample.cpp $(CFLAGS) -o rewritersample \
	$(CLANG_BUILD_FLAGS) $(CLANGLIBS) `$(LLVM_CONFIG_COMMAND)`

clean:
	rm -rf *.o *.ll rewritersample
