CC = gcc
CXX = g++ -fPIC
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

LIB = libAMDUtil.so

$(LIB): $(OBJ)
	$(CC) -shared -o $(LIB) $(OBJ)

%.o: $(CXX) -fPIC -c $< -o $@

install: $(LIB)
	cp $(LIB) /usr/lib/

clean:
	rm *.o $(LIB)

.PHONY: clean
