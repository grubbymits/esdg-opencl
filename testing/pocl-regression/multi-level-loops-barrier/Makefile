#C compiler
CC = clang++

SRC = test_multi_level_loops_with_barriers.cpp
LIBS = -L/opt/esdg-opencl/lib -lOpenCL
INC = -I/opt/esdg-opencl/include/ -I/usr/include/x86_64-linux-gnu/c++/4.7/
CC_FLAGS = -g

EXE = test

release:$(SRC)
	$(CC) $(CC_FLAGS) $(SRC) $(INC) $(LIBS) -o $(EXE)

run:
	./$(EXE)

clean: $(SRC)
	rm -rf $(EXE) $(EXE).linkinfo result* *.txt *.s *.bc binaries microblaze *.ll *.dat *.csv *.tmp* init.* *.cl
