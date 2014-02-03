CXX = clang++
CXXFLAGS = -c -g -fPIC
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
INC = -I/opt/esdg-opencl/include/ -I/usr/include/x86_64-linux-gnu/c++/4.7/
LIBS = -L/opt/esdg-opencl/lib/ -lOpenCL

target = libSDKUtil.so

$(target) : $(OBJ)
	$(CXX) -o $@ -shared $(OBJ) $(LIBS)
%.o : %.cpp
	$(CXX) $(INC) $(CXXFLAGS) $< -o $@
