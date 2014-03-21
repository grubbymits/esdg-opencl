CXX = clang++
CXXFLAGS = -g -O2
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.7 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

floyd-warshall: FloydWarshall.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: FloydWarshall.cpp
	rm -rf floyd-warshall *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
