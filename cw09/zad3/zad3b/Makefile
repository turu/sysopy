CC = gcc
CFLAGS = -Wall -lpthread

all : zad3b

zad3b: zad3b.o
	$(CC) -o $@.out $^ $(CFLAGS) 

clean :
	rm -rvf *.o
	rm -rvf *.out
