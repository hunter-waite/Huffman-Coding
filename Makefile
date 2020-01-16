CFLAGS = -g -ansi -Wall -pedantic
CC = gcc
LD = gcc


all: hencode hdecode

hencode: hencode.o huff_help.o
	$(LD) -o hencode hencode.o huff_help.o

hencode.o: hencode.c hencode.h
	$(CC) $(CLFAGS) -c -o hencode.o hencode.c

hdecode: hdecode.o huff_help.o
	$(LD) -o hdecode hdecode.o huff_help.o

hdecode.o: hdecode.c hdecode.h
	$(CC) $(CFLAGS) -c -o hdecode.o hdecode.c

huff_help.o: huff_help.c huff_help.h
	$(CC) $(CFLAGS) -c -o huff_help.o huff_help.c

clean:
	rm hencode.o
	rm hdecode.o
	rm huff_help.o
