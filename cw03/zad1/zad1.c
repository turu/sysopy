#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef SYSTEM
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

typedef struct {
    int key;
    char * data;
} MyStruct;

void randomBytes(char * data, size_t size) {

}

int generate(char * filePath, size_t structSize, int structCount) {
    srand(time(NULL));
    if (structSize <= sizeof(int) || structCount <= 0) {
        return -1;
    }

    #ifndef SYSTEM
    FILE * file = fopen(filePath, "wb+");
    if (file == NULL) {
        return -1;
    }
    fwrite(&structSize, sizeof(size_t), 1, file);
    fwrite(&structCount, sizeof(int), 1, file);
    #else
    int file = open(filePath, O_CREAT | O_WRONLY);
    if (file == -1) {
        return -1;
    }
    write(file, &structSize, sizeof(size_t));
    write(file, &structCount, sizeof(int));
    #endif

    structSize -= sizeof(int);
    MyStruct * myStruct = (MyStruct *) malloc(sizeof(MyStruct));
    myStruct->data = (char*) malloc(structSize);

    int i;
    for (i = 0; i < structCount; i++) {
        myStruct->key = rand();
        randomBytes(myStruct->data, structSize);
        #ifdef SYSTEM
        write(file, &myStruct->key, sizeof(int));
        write(file, &myStruct->data, structSize);
        #else
        fwrite(&myStruct->key, sizeof(int), 1, file);
        fwrite(&myStruct->data, structSize, 1, file);
        #endif
    }

    free(myStruct->data);
    free(myStruct);

    #ifdef SYSTEM
    close(file);
    #else
    fclose(file);
    #endif

    return 0;
}

int sort(char * filePath) {
    #ifndef SYSTEM
    FILE * file = fopen(filePath, "rb+");
    if (file == NULL) {
        return -1;
    }
    #else
    int file = open(filePath, O_RDWR);
    if (file == -1) {
        return -1;
    }
    #endif

    #ifdef SYSTEM
    close(file);
    #else
    fclose(file);
    #endif

    return 0;
}

int generuj (int argc, char ** argv) {
    if (argc < 4) {
        return -1;
    }

    size_t structSize = atoi(argv[1]);
    int structCount = atoi(argv[2]);
    char * filePath = argv[3];

    generate(filePath, structSize, structCount);

    return 0;
}

int sortuj (int argc, char ** argv) {
    if (argc < 2) {
        return -1;
    }

    char * filePath = argv[1];
    sort(filePath);

    return 0;
}

int main(int argc, char ** argv) {
    if (argc < 1) {
        printf("Za malo argumentow!\n");
        return -1;
    }

    char * modeString = argv[0];
    if (!strcmp(modeString, "generuj")) {
        return generuj(argc, argv);
    } else if (!strcmp(modeString, "sortuj")) {
        return sortuj(argc, argv);
    }

    return -1;
}
