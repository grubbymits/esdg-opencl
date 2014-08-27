CXX = clang++
CXXFLAGS = -g -O2
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.8 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -L../SDKUtil -lSDKUtil

radix-sort: RadixSort.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: RadixSort.cpp
	rm -rf radix-sort *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
