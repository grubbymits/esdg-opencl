
# Makefile for InsizzleV3

# CFLAGS extra options
# -DVTHREAD - for running code with vthreads
# -DPRINTOUT - print all instructions executed
# -DINSDEBUG - print lots of information about everything
# -DDEBUGmem - print information about memory accesses
# -DNOSTALLS - count stall events but don't execute them
# -DMEMORY_PREF_LOWER - switches to alternative memory service routine

CC = gcc
CFLAGS = -c -g -Wall -pedantic -std=c99 -Wextra `xml2-config --cflags` -D_XOPEN_SOURCE
LDFLAGS = -lm `xml2-config --libs` -lpcre

INCDIR =  inc
LE1_DRIVER_INC = ../le1_driver
SRCDIR = src
OBJDIR = obj

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

TARGET = INSIZZLE
TARGETAPI = libInsizzleAPI.so
#LDFLAGS += -nostartfiles

REV_PERL = `perl scripts/revision.pl ${TARGET} ${TARGET}`

debug:		CFLAGS += -g
release:	CFLAGS += -O3 -fno-strict-aliasing
mbMem:		CFLAGS += -DmicroblazeMemDump
API:		CFLAGS += -fPIC -DINSIZZLEAPI
NOSTALLS:	CFLAGS += -DNOSTALLS
VTHREAD:	CFLAGS += -DVTHREAD
VTHREAD_NOSTALLS: CFLAGS += -DVTHREAD -DNOSTALLS
all: $(SRC) $(TARGET)

debug:	$(SRC) $(TARGET)
release: $(SRC) $(TARGET)
mbMem:	$(SRC) $(TARGET)
API:	$(SRC) $(TARGETAPI)
NOSTALLS: $(SRC) $(TARGET)
VTHREAD: $(SRC) $(TARGET)
VTHREAD_NOSTALLS: $(SRC) $(TARGET)

$(TARGET): $(OBJ)
	   $(CC) $(OBJ) $(LDFLAGS) -o $@
	@echo "Versioning"
	@echo ${REV_PERL}

$(TARGETAPI): $(OBJ)
	$(CC) -o $(TARGETAPI) $(OBJ) -shared -lxml2 -lpcre -lm -lz -llzma

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@ -I$(INCDIR) -I$(LE1_DRIVER_INC) $(VTAPI)

clean:
		rm -f $(OBJ) $(TARGET) $(TARGETAPI)

distclean:
		rm -f $(OBJ) $(TARGET) $(TARGETAPI)
		find . -name '*~' -delete
		find . -name '#*' -delete
