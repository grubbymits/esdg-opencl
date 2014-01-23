CXX = clang++
CXXFLAGS = -g
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.7 -I/usr/include/x86_64-linux-gnu/c++/4.8 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

reduction: Reduction.cpp Reduction.hpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: Reduction.cpp
	rm -rf reduction *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
