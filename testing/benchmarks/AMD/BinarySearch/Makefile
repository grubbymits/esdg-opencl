CXX = clang++
CXXFLAGS = -g -O0
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.7 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

binary-search: BinarySearch.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: BinarySearch.cpp
	rm -rf binary-search *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
