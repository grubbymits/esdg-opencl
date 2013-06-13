#C compiler
CC = clang++

SRC = bfs.cpp timer.cc
LIBS = -lm `xml2-config --libs` -lpcre -L/opt/esdg-opencl/lib -lInsizzleAPI \
-lOpenCL -lclangFrontendTool -lclangFrontend -lclangDriver -lclangEdit -lpthread
INC = -I/opt/esdg-opencl/include/ -I/usr/include/x86_64-linux-gnu/c++/4.7/
CC_FLAGS =

EXE = bfs

release:$(SRC)
	$(CC) $(CC_FLAGS) $(SRC) $(INC) $(LIBS) -o $(EXE)

run:
	./$(EXE)

clean: $(SRC)
	rm -rf $(EXE) $(EXE).linkinfo result* *.txt *.s *.bc binaries microblaze *.ll
