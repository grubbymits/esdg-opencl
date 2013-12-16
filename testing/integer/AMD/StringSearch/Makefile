CXX = clang++
CXXFLAGS = -g
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.7 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

string-search : StringSearch.cpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: StringSearch.cpp
	rm -rf string-search *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
