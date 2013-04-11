#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

time_t prevTime = -1;
int childPID;
char * args[] = {"cp", "-r", (char*) 0};

void printHelp() {
    printf("Argumenty: <sciezka do pliku> <okres w sekundach> <limit wielkosci w bajtach>\n");
}

int main(int argc, char ** argv) {
    if (argc < 4) {
        printf("Za malo argumentow\n");
        printHelp();
        return 1;
    }

    char * filepath = argv[1];
    int delay = atoi(argv[2]);
    int limit = atoi(argv[3]);

    if (delay < 0) {
        printf("Podano ujemna wartosc okresu\n");
        printHelp();
        return 2;
    }

    if (limit <= 0) {
        printf("Podano niewlasciwa wartosc limitu wielkosci pliku\n");
        printHelp();
        return 3;
    }

    struct stat * fstat = (struct stat*) malloc(sizeof(struct stat));

    while (1) {
        if (stat(filepath, fstat) == -1) {
            printf("Error occured while reading file %s\n", filepath);
            return 4;
        }

        if (prevTime != -1 && fstat->st_mtime != prevTime && fstat->st_size > limit) {
            childPID = fork();
            if (childPID < 0) {
                printf("Internal error\n");
                return 5;
            } else if (childPID == 0) {
                char destination[100];
                char strDate[30];
                strftime(strDate, 30, "%Y-%m-%d_%T", localtime(&fstat->st_mtime));
                sprintf(destination, "archiwum/%s_%s", filepath, strDate);
                execlp("cp", "cp", filepath, destination, NULL);
            } else {
                int status = -1;
                wait(&status);
                FILE* fd = fopen(filepath, "w");
                fclose(fd);
            }
        }

        prevTime = fstat->st_mtime;
        sleep(delay);
    }

    free(fstat);

    return 0;
}
