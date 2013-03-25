CC=gcc
CFLAGS=-Wall -I src/include
LIBS= 
CDL=-ldl
AMEM=lib/libmymem.a
SMEM=lib/libmymem.so
AMAT=lib/libmymat.a
SMAT=lib/libmymat.so
VPATH=src/:src/include:src/commons
OBJ=execution.o

ifeq ($(MODE), STATIC)
	LIBS += $(AMEM) $(AMAT)
endif
ifeq ($(MODE), SHARED)
	LIBS += $(SMEM) $(SMAT)
endif
ifeq ($(MODE), DYNAMIC)
	CFLAGS += $(CDL) -D DLL
endif

mattest: clean copy $(OBJ) mattest.o
	$(CC) $^ -o $@.out $(CFLAGS) $(LIBS)

memtest: clean copy $(OBJ) memtest.o
	$(CC) $^ -o $@.out $(CFLAGS) $(LIBS)

copy:
	cp ../zad1/src/.libs/*.so lib/
	cp ../zad2/src/.libs/*.so lib/
	cp ../zad1/src/.libs/*.a lib/
	cp ../zad2/src/.libs/*.a lib/

clean:
	rm -rvf lib/*.so
	rm -rvf lib/*.a
	rm -rvf *.out

.PHONY: all copy clean
