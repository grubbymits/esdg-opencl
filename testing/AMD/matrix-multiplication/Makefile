CC = clang
LIBS = -lm `xml2-config --libs` -lpcre -lInsizzleAPI -lOpenCL -lclangFrontendTool \
-lclangFrontend -lclangDriver -lclangEdit -lAMDUtil

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

mm: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $@

%.o: $(CC) $(SRC) -o $<

clean:
	rm -r binaries microblaze main.c *.o *.txt *.s *.bc mm

.PHONY: clean
