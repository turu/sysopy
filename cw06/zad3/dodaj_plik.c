#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char **argv){
    if(argc < 3) {
        printf("Buka\n");
        return 1;
    }

    FILE * fifo = fopen(argv[1], "w");
    int i;

    for (i = 2; i < argc; i++) {
        fprintf(fifo, "%s\n", argv[i]);
    }

    fflush(fifo);
    fclose(fifo);

    return 0;
}
