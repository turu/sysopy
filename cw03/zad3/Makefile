CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -O2
SOURCES = zad3.c
OBJ =  $(SOURCES:.c=.o)
.PHONY = clean all zad3

all: zad3

zad3: $(OBJ)
	$(CC) $(CFLAGS)  $^ -o $@.out	

clean:
	rm -f *.out
	rm -f *.o
