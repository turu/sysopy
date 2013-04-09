CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall
SOURCES = zad1.c
OBJ =  $(SOURCES:.c=.o)
.PHONY = clean all zad1

all: zad1

zad1: $(OBJ)
	$(CC) $(CFLAGS)  $^ -o $@.out	

clean:
	rm -f *.out
	rm -f *.o
