CFLAGS=-Wall -lpthread
CC=gcc

all: zad2

zad2: zad2.c
	$(CC) -o $@.out $^ $(CFLAGS)
	
clean:
	rm -rvf *.o
	rm -rvf *.out
	rm -rvf core.*
