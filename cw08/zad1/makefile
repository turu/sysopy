CC=gcc
CFLAGS = -Wall -lm

.PHONY : all clean
all : producent konsument

producent : producent.o
	$(CC) $(CFLAGS) -o $@.out $^
	
konsument : konsument.o
	$(CC) $(CFLAGS) -o $@.out $^

clean :
	rm -rvf *.o
	rm -rvf *.out

