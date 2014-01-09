CC = clang
CFLAGS = -O2 -c -emit-llvm -target le1
COMMON_SRC = $(wildcard libm/common/*.c)
MATH_SRC = $(wildcard libm/math/*.c)
LIBC_SRC = libc/string/memset.c
INC = -I./libc/include/

COMMON_OBJ = $(patsubst %.c, %.o, $(COMMON_SRC))
MATH_OBJ = $(patsubst %.c, %.o, $(MATH_SRC))

newlib.bc : common.bc math.bc string.bc
	llvm-link $^ -o $@

common.bc : $(COMMON_OBJ)
	llvm-link $(COMMON_OBJ) -o $@

math.bc : $(MATH_OBJ)
	llvm-link $(MATH_OBJ) -o $@

string.bc: $(LIBC_SRC)
	$(CC) $(CFLAGS) $(INC) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $(INC) $< -o $@

clean:
	rm common/*.o math/*.o math.bc common.bc newlib.bc
