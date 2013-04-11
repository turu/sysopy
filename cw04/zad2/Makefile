CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -lrt -O2
.PHONY = clean all archiwizuj dopisz

all: archiwizuj dopisz

archiwizuj: archiwizuj.o
	$(CC) $(CFLAGS)  $^ -o $@.out

dopisz: dopisz.o
	$(CC) $(CFLAGS) $^ -o $@.out

clean:
	rm -f *.out
	rm -f *.o
