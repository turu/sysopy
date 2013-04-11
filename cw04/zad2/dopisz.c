#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

char strDate[30];

void randomChars(char * data, size_t size) {
    while (size--) {
        *data = (char) (rand() % 95 + 32);
        data++;
    }
    *data = '\0';
}

void printHelp() {
    printf("Argumenty: <sciezka do pliku> <okres w sekundach> <wielkosc soli w bajtach>\n");
}

int main(int argc, char ** argv) {
    srand(time(NULL));

    if (argc < 4) {
        printf("Za malo argumentow\n");
        printHelp();
        return 1;
    }

    char * filepath = argv[1];
    int delay = atoi(argv[2]);
    int bytes = atoi(argv[3]);

    char salt[bytes+1];

    if (delay < 0) {
        printf("Podano ujemna wartosc okresu\n");
        printHelp();
        return 2;
    }

    if (bytes <= 0) {
        printf("Podano niewlasciwa wielkosc soli\n");
        printHelp();
        return 3;
    }

    while (1) {
        time_t rawTime;
        time(&rawTime);
        int PID = getpid();
        strftime(strDate, 30, "%Y-%m-%d_%T", localtime(&rawTime));
        randomChars(salt, bytes);

        FILE * file = fopen(filepath, "a+");
        fprintf(file, "%d %s %s\n", PID, strDate, salt);
        fclose(file);

        sleep(delay);
    }

    return 0;
}
