CC = clang
SRC = test_multi_level_loops_with_barriers.cpp
LIBPATH = /usr/lib:/opt/esdg-opencl/lib
LIBS = -lm `xml2-config --libs` -lpcre -L/opt/esdg-opencl/lib -lInsizzleAPI -lOpenCL -lclangFrontendTool \
-lclangFrontend -lclangDriver -lclangEdit -L/usr/lib/x86_64/linux-gnu -lstdc++ -lpthread
INC = -I/opt/esdg-opencl/include/ -I/usr/include/x86_64-linux-gnu/c++/4.7/

multi_lvl: $(SRC)
	$(CC) $(SRC) $(INC) $(LIBS) -o $@

clean:
	rm -r binaries microblaze main.c *.o *.txt *.s *.bc *.ll multi_lvl

.PHONY: clean
