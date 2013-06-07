UNAME = $(shell uname)
CC = clang
CFLAGS =
SRC = structs.c
LIBPATH = /usr/lib:/opt/esdg-opencl/lib
LIBS = -lm `xml2-config --libs` -lpcre -L/opt/esdg-opencl/lib -lInsizzleAPI -lOpenCL -lclangFrontendTool \
-lclangFrontend -lclangDriver -lclangEdit -lpthread

ifeq ($(UNAME), Linux)
LIBS += -L/usr/lib/x86_64/linux-gnu -lstdc++
endif

INC = /opt/esdg-opencl/include

example: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -I$(INC) $(LIBS) -o $@

clean:
	rm -r binaries microblaze main.c *.o *.txt *.s *.bc *.ll example

.PHONY: clean
