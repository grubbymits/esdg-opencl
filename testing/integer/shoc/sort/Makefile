CXX=clang++
COMMONSRC=$(wildcard ../common/*.cpp)
OBJ=$(patsubst %.cpp, %.o, $(COMMONSRC))

LIBPATH=/usr/lib:/usr/local/lib:/opt/esdg-opencl/lib

LIBS = -lm `xml2-config --libs` -lpcre -L/opt/esdg-opencl/lib -lInsizzleAPI \
       -lOpenCL -lclangFrontendTool -lclangFrontend -lclangDriver -lclangEdit \
       -L/usr/lib/x86_64/linux-gnu -lstdc++ -lpthread

INC = -I/usr/include/x86_64-linux-gnu/c++/4.7/ -I/opt/esdg-opencl/include \
      -I../common/

all: sort

sort: $(OBJ) sort.o
	ld $(LIBS) sort.o $(OBJ) sort.o -o $@

sort.o : Sort.cpp
	$(CXX) -o $@ $^ $(INC)

%.o: $(COMMONSRC)
	$(CXX) -o $@ $^ $(INC)

clean:
	rm -r binaries microblaze *.bc *.s sort

.PHONY: clean
