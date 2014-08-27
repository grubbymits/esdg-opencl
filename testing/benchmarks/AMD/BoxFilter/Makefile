CXX = clang++
CXXFLAGS = -c -g -O2
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
INC = -I/opt/esdg-opencl/include/ -I/usr/include/x86_64-linux-gnu/c++/4.8/ -I../SDKUtil/include/
LIBS = -L/opt/esdg-opencl/lib/ -lOpenCL -lSDKUtil

target = box-filter

$(target) : $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LIBS)
%.o : %.cpp
	$(CXX) $(INC) $(CXXFLAGS) $< -o $@
