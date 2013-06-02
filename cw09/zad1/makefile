CFLAGS=-Wall -lpthread
CC=gcc

ifdef VER
	CFLAGS += -D $(VER)
endif

all: zad1

zad1: zad1.c
	$(CC) -o $@.out $^ $(CFLAGS)
	
clean:
	rm -rvf *.o
	rm -rvf *.out
