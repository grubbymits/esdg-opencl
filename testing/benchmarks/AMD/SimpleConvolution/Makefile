CXX = clang++
CXXFLAGS = -g
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.8 -I/usr/include/x86_64-linux-gnu/c++/4.8 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

simple-convolution: SimpleConvolution.cpp SimpleConvolution.hpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: SimpleConvolution.cpp
	rm -rf simple-convolution *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
