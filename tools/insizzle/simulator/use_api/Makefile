# Makefile for API

CC = g++
CFLAGS = -c -Wall -Wextra -DAPI -I../../le1_driver -I../inc `xml2-config --cflags`
LDFLAGS = -lm `xml2-config --libs` -lpcre


all:
	cd ../ && make clean && make API && cd .
	$(CC) main.cpp $(CFLAGS)
	$(CC) -o TEST main.o ../INSIZZLEAPI.so $(LDFLAGS)

test:
	./TEST -i test_data/iram0.bin -d test_data/dram.bin -m test_data/model_1lsu_1.xml

clean:
	cd ../ && make clean && cd .
	rm -f *~ main.o TEST
