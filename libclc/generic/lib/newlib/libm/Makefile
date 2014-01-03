CC = clang
CFLAGS = -O2 -c -emit-llvm -target le1
COMMON_SRC = $(wildcard common/*.c)
MATH_SRC = $(wildcard math/*.c)
INC = -I../libc/include/

COMMON_OBJ = $(patsubst %.c, %.o, $(COMMON_SRC))
MATH_OBJ = $(patsubst %.c, %.o, $(MATH_SRC))

newlib.bc : common.bc math.bc
	llvm-link $^ -o $@

common.bc : $(COMMON_OBJ)
	llvm-link $(COMMON_OBJ) -o $@

math.bc : $(MATH_OBJ)
	llvm-link $(MATH_OBJ) -o $@

%.o : %.c
	$(CC) $(CFLAGS) $(INC) $< -o $@

clean:
	rm common/*.o math/*.o math.bc common.bc newlib.bc
