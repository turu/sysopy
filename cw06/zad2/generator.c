#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main (int argc, char **argv){
    srand(time(NULL));

    if (argc < 2) {
        printf("Bo zupa byla za slona.\n");
        return 1;
    }

    char * path = argv[1];
    int clients = 1;

    if (argc < 3) {
        printf("Nie podano liczby klientow. Ustawiam %d.\n", clients);
    } else {
        clients = atoi(argv[2]);
    }

    if(mkfifo(path, 0660) == -1){
        printf("Nie udalo sie utworzyc rury.\n");
        exit(2);
    }

    int pid;
    FILE * fifo;
    char buff[128];

    while (--clients > 0) {
        pid = fork();
        if (pid == 0) {
            fifo = fopen(path, "w");
            while (1) {
                sprintf(buff, "%d\n", rand() % 1000);
                fprintf(fifo, "%s\n", buff);
            }
        }
    }

    fifo = fopen(path, "w");
    while (1) {
        sprintf(buff, "%d\n", rand() % 1000);
        fprintf(fifo, "%s\n", buff);
    }

    return 0;
}
