CXX=clang++
SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, %.o, $(SRC))
LIBS = -L/opt/esdg-opencl/lib -lOpenCL
INC = -I/usr/include/x86_64-linux-gnu/c++/4.7 -I/opt/esdg-opencl/include

all: libShocCommon.so

libShocCommon.so: $(OBJ)
	ld -shared $(LIBS) -o $@ $^

%.o: $(SRC)
	$(CXX) -fPIC -o $@ $^ $(INC)
