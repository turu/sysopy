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
    while (size--) {
        *data = (char) rand();
        data++;
    }
}

int generate(char * filePath, size_t structSize, int structCount) {
    srand(time(NULL));
    if (structSize <= sizeof(int) || structCount <= 0) {
        return -1;
    }

    #ifndef SYSTEM
    FILE * file = fopen(filePath, "w+b");
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
    fsync(file);
    close(file);
    #else
    fflush(file);
    fclose(file);
    #endif

    return 0;
}

int sort(char * filePath) {
    size_t structSize;
    int structCount;

    #ifndef SYSTEM
    FILE * file = fopen(filePath, "r+b");
    if (file == NULL) {
        return -1;
    }
    fread(&structSize, sizeof(size_t), 1, file);
    fread(&structCount, sizeof(int), 1, file);
    #else
    int file = open(filePath, O_RDWR);
    if (file == -1) {
        return -1;
    }
    read(file, &structSize, sizeof(size_t));
    read(file, &structCount, sizeof(int));
    #endif



    #ifdef SYSTEM
    close(file);
    #else
    fclose(file);
    #endif

    return 0;
}

int generuj (int argc, char ** argv) {
    if (argc < 5) {
        printf("Za malo argumentow!!!\n");
        return -1;
    }

    size_t structSize = atoi(argv[2]);
    int structCount = atoi(argv[3]);
    char * filePath = argv[4];

    if (generate(filePath, structSize, structCount) == -1) {
        printf("Generacja nie powiodla sie\n");
        return -1;
    }

    return 0;
}

int sortuj (int argc, char ** argv) {
    if (argc < 3) {
        return -1;
    }

    char * filePath = argv[2];

    if(sort(filePath) == -1) {
        printf("Sortowanie nie powiodlo sie\n");
        return -1;
    }

    return 0;
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Za malo argumentow!\n");
        return -1;
    }

    char * modeString = argv[1];
    printf("%s\n", modeString);
    if (!strcmp(modeString, "generuj")) {
        printf("Entering generacja\n");
        return generuj(argc, argv);
    } else if (!strcmp(modeString, "sortuj")) {
        return sortuj(argc, argv);
    }

    return -1;
}
