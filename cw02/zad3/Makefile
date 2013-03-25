CC=gcc
CFLAGS=-Wall -I src/include -I ../zad1/src/include -I ../zad2/src/include
VPATH=src/:src/include:src/commons:../zad1/src/include:../zad2/src/include
OBJ=execution.o memtest.o
LM=-lmymem
LT=-lmymat
LD_LIBRARY_PATH:=/usr/local/bin
export LD_LIBRARY_PATH

ifeq ($(MODE), STATIC)
	CFLAGS += -static
endif
ifeq ($(MODE), DYNAMIC)
	CFLAGS += -ldl -D DLL
	LM = 
	LT = 
endif

mat: clean copy $(OBJ)
	$(CC) $(CFLAGS) -L./lib $(LT) $(OBJ) mattest.o -o $@.out

mem: clean copy $(OBJ)
	$(CC) $(CFLAGS) -L./lib $(LM) $(OBJ) -o $@.out

clean:
	rm -rvf lib/*.so
	rm -rvf lib/*.a
	rm -rvf *.out

copy:
	cp ../zad1/src/.libs/*.so lib/
	cp ../zad2/src/.libs/*.so lib/
	cp ../zad1/src/.libs/*.a lib/
	cp ../zad2/src/.libs/*.a lib/

.PHONY: all copy clean mat mem
