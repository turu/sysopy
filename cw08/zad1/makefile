CC=gcc
CFLAGS = -Wall -lm

SRC1 = producent.c
SRC2 = konsument.c
HEAD = commons.h

.PHONY : all clean
all : producent konsument

producent : producent.o
	$(CC) $(CFLAGS) -o $@ $^

producent.o : $(SRC1) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC1)
	
konsument : konsument.o
	$(CC) $(CFLAGS) -o $@ $^

konsument.o : $(SRC2) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC2)

clean :
	rm -rvf *.o
	rm -rvf producent konsument

