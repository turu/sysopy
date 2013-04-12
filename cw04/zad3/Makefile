CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -lrt -O2
.PHONY = clean all

all: policz

policz: policz.o
	$(CC) $(CFLAGS)  $^ -o $@.out

clean:
	rm -f *.out
	rm -f *.o
