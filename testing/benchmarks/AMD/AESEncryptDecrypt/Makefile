CXX = clang++
CXXFLAGS = -g
INC = -I/opt/esdg-opencl/include -I/usr/include/x86_64-linux-gnu/c++/4.8 -I/usr/include/x86_64-linux-gnu/c++/4.8 -I../SDKUtil/include
LIBS = -L/opt/esdg-opencl/lib -lOpenCL -lSDKUtil

aes-encrypt-decrypt: AESEncryptDecrypt.cpp AESEncryptDecrypt.hpp
	$(CXX) $(CXXFLAGS) $(INC) $< $(LIBS) -o $@

clean: MatrixTranspose.cpp
	rm -rf aes-encrypt-decrypt *.s *.bc binaries microblaze *.dat *.csv *.tmp* init.*
