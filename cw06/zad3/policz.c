#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int countBytes(char* name) {
    int file = open(name,O_RDONLY);

    if (file < 0) {
        return file;
    }

    char byte;
    int ct =0;
    while (read(file,&byte,sizeof(char))) {
        if (byte) {
            ct++;
        }
    }

    return ct;
}


int main (int argc, char **argv){
    if (argc < 2) {
        printf("Buka\n");
        return 1;
    }

    char * path = argv[1];
    if(mkfifo(path, 0660) == -1){
        printf("Could not create pajp.\n");
        return 2;
    }

    int fifo = open(path, O_RDONLY | O_NONBLOCK);
    FILE * wyniki = fopen("wyniki.log", "w");
    char buff[128];
    int res;

    while (1) {
        res = read(fifo, buff, sizeof(buff));
        if(!res){
            sleep(1);
        } else {
            char * ptr = strtok(buff, "\n"), * ptr2;
            ptr2 = strtok(NULL, "\n");
            int nonZero;

            while (ptr2 != NULL) {
                nonZero = countBytes(ptr);
                printf("%s: %d\n", ptr, nonZero);
                fprintf(wyniki, "%s: %d\n", ptr, nonZero);
                fflush(wyniki);
                ptr = ptr2;
                ptr2 = strtok(NULL, "\n");
            }
        }
    }

    fclose(wyniki);
    close(fifo);

    return 0;
}
