#C compiler
CC = clang

SRC = nw.c
LIBS = -L/opt/esdg-opencl/lib -lOpenCL
INC = -I/opt/esdg-opencl/include/
CC_FLAGS = -g 

EXE = nw

release:$(SRC)
	$(CC) $(CC_FLAGS) $(SRC) $(INC) $(LIBS) -o $(EXE)

run:
	./$(EXE)

clean: $(SRC)
	rm -rf $(EXE) $(EXE).linkinfo result* *.txt *.s *.bc binaries microblaze *.ll *.dat *.csv *.tmp* init.*
