CC = clang
CFLAGS = -target le1 -emit-llvm -D__LE1__ -c
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, obj/%.bc, $(SRC))
INC = -I../../libc/include -I../machine

math.bc : $(OBJ)
	llvm-link $(OBJ) -o $@

obj/%.bc : %.c
	$(CC) $(CFLAGS) $(INC) $< -o $@
