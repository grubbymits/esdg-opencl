CXX = clang++
CXXFLAGS = -g
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.7 -I../SDKUtil/include
LIBS = -lglut -lGLU -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

nbody: NBody.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: NBody.cpp
	rm -rf nbody *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
