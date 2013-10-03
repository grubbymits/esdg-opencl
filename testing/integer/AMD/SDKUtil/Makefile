CXX = clang++
CXXFLAGS = -O2 -g -fPIC
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
INC = -I/opt/esdg-opencl/include/ -I/usr/include/x86_64-linux-gnu/c++/4.7/
LIBS = -L/opt/esdg-opencl/lib/ -lOpenCL

target = SDKUtil.so

lib : $(OBJ)
	ld $(OBJ) $(LIBS) -o $(target)

%.o : %.cpp
	$(CXX) $(INC) $(CXX_FLAGS) $(LIBS) $< -o $@
