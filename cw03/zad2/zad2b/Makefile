CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -O2
SOURCES = zad2b.c
OBJ =  $(SOURCES:.c=.o)
.PHONY = clean all zad2b

all: zad2b

zad2b: $(OBJ)
	$(CC) $(CFLAGS)  $^ -o $@.out	

clean:
	rm -f *.out
	rm -f *.o
