CC = gcc
SHELL = /bin/bash
CFLAGS = -Wall -lrt -O2 -lm
SOURCES = zad2.c
OBJ =  $(SOURCES:.c=.o)
.PHONY = clean all zad2

all: zad2

zad2: $(OBJ)
	$(CC) $(CFLAGS)  $^ -o $@.out	

clean:
	rm -r archiwum
	rm -f *.out
	rm -f *.o
