CC=gcc
CFLAGS = -Wall

SRC1 = producent.c
SRC2 = konsument.c
HEAD = types.h

.PHONY : all
all : producent konsument

producent : producent.o
	$(CC) $(CFLAGS) -o $@ $^

producent.o : $(SRC1) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC1)
	
konsument : konsument.o
	$(CC) $(CFLAGS) -o $@ $^

konsument.o : $(SRC2) $(HEAD)
	$(CC) $(CFLAGS) -c -o $@ $(SRC2)

.PHONY : clean
clean :
	rm -f *.o
	
.PHONY : uninstall
uninstall :
	rm -f producent konsument


