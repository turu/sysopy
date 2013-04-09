CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -O2
SOURCES = zad2a.c
OBJ =  $(SOURCES:.c=.o)
.PHONY = clean all zad2a

all: zad2a

zad2a: $(OBJ)
	$(CC) $(CFLAGS)  $^ -o $@.out	

clean:
	rm -f *.out
	rm -f *.o
