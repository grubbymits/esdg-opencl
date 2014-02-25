CC=gcc
MODE=X86_INSIZZLE
CFLAGS=-I. -D$(MODE)
DEPS = driver.h, errno-base.h, global.h, mti.h, sctb.h, stubs.h, vc_vt_top_esl.h, vtapi.h xil_config.h 
MODE=X86_INSIZZLE

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
driver:	driver.o stubs.o vtapi.o
	gcc -o driver driver.o stubs.o vtapi.o -I. -D$(MODE)

clean:
	rm -f *.o driver