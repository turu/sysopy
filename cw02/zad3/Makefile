CC=gcc
CFLAGS=-Wall -I src/include -I ../zad1/src/include -I ../zad2/src/include
VPATH=src/:src/include:src/commons:../zad1/src/include:../zad2/src/include
LIB=-lmymat -lmymem -lrt

ifeq ($(MODE), STATIC)
	CFLAGS += -static
endif
ifeq ($(MODE), DYNAMIC)
	CFLAGS += -ldl -D DLL
	LIB = 
endif

all: clean copy mem mat

mat: execution.o mattest.o
	$(CC) $(CFLAGS) -L./lib $^ $(LIB) -o $@.out

mem: execution.o memtest.o 
	$(CC) $(CFLAGS) -L./lib $^ $(LIB) -o $@.out

clean:
	rm -rvf lib/*.so*
	rm -rvf lib/*.a
	rm -rvf *.out
	rm -rvf *.o

copy:
	cp ../zad1/src/.libs/*.so* lib/
	cp ../zad2/src/.libs/*.so* lib/
	cp ../zad1/src/.libs/*.a lib/
	cp ../zad2/src/.libs/*.a lib/

.PHONY: all copy clean mat mem

