CXX := clang++
RTTIFLAG := -fno-rtti
CXXFLAGS := $(shell llvm-config --cxxflags) $(RTTIFLAG)
LLVMLDFLAGS := $(shell llvm-config --cppflags --ldflags --libs core)
DDD := $(shell echo $(LLVMLDFLAGS))
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXES = $(OBJECTS:.o=)
CLANGLIBS = \
    -L /usr/local/lib \
    -lclangFrontend \
    -lclangParse \
    -lclangSema \
    -lclangAnalysis \
    -lclangAST \
    -lclangLex \
    -lclangBasic \
    -lclangDriver \
    -lclangSerialization \
    -lLLVMMC \
    -lLLVMSupport \

all: $(OBJECTS) $(EXES)
%.o : $(CXX) -o $@ $(CLANGLIBS) $< $(LLVMLDFLAGS)
