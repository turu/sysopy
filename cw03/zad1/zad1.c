#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/resource.h>

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

void printStats(int i, struct rusage* r2, struct rusage* r1, struct timeval* t2, struct timeval* t1) {
	printf("Punkt kontrolny %d\n",i);
	printf("Aktualne dane.\nPomiar czasu\n");
	printf("rzeczywisty: %ld[s] %ld[us]\n",(long int)t2->tv_sec,(long int)t2->tv_usec);
	printf("uzytkownika: %ld[s] %ld[us]\n",(long int)r2->ru_utime.tv_sec,(long int)r2->ru_utime.tv_usec);
	printf("systemowy: %ld[s] %ld[us]\n",(long int)r2->ru_stime.tv_sec,(long int)r2->ru_stime.tv_usec);
	printf("\n\n");

	if (i!=1) {
		printf("Odniesienie do punktu kontrolnego %d\n",i-1);
		printf("Pomiar czasu\n");
        printf("rzeczywisty: %lf[s]\n", (double)(t2->tv_sec*1000000 + t2->tv_usec - t1->tv_sec*1000000 - t1->tv_usec)/1000000);
        printf("uzytkownika: %lf[s]\n", (double)(r2->ru_utime.tv_sec*1000000 + r2->ru_utime.tv_usec - r1->ru_utime.tv_sec*1000000 -
				r1->ru_utime.tv_usec) / 1000000);
        printf("systemowy: %lf[s]\n", (double)(r2->ru_stime.tv_sec*1000000 + r2->ru_stime.tv_usec - r1->ru_stime.tv_sec*1000000 -
                r1->ru_stime.tv_usec) / 1000000);
		printf("\n\n");
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
        write(file, &(myStruct->key), sizeof(int));
        write(file, myStruct->data, structSize);
        #else
        fwrite(&(myStruct->key), sizeof(int), 1, file);
        fwrite(myStruct->data, structSize, 1, file);
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

int headerSize = sizeof(size_t) + sizeof(int);

#ifdef SYSTEM
int process(int i, int file, size_t structSize, MyStruct * lhs, MyStruct * rhs) {
    long offset = i * structSize + headerSize;
    lseek(file, offset, SEEK_SET);
    structSize -= sizeof(int);
    read(file, &(lhs->key), sizeof(int));
    read(file, lhs->data, structSize);
    read(file, &(rhs->key), sizeof(int));
    read(file, rhs->data, structSize);

    if (lhs->key > rhs->key) {
        //printf("Swapping %d and %d\n", lhs->key, rhs->key);
        lseek(file, offset, SEEK_SET);
        write(file, &(rhs->key), sizeof(int));
        write(file, rhs->data, structSize);
        write(file, &(lhs->key), sizeof(int));
        write(file, lhs->data, structSize);
        return 1;
    }

    return 0;
}
#else
int process(int i, FILE * file, size_t structSize, MyStruct * lhs, MyStruct * rhs) {
    long offset = i * structSize + headerSize;
    fseek(file, offset, SEEK_SET);
    structSize -= sizeof(int);
    fread(&(lhs->key), sizeof(int), 1, file);
    fread(lhs->data, structSize, 1, file);
    fread(&(rhs->key), sizeof(int), 1, file);
    fread(rhs->data, structSize, 1, file);

    if (lhs->key > rhs->key) {
        //printf("Swapping %d and %d\n", lhs->key, rhs->key);
        fseek(file, offset, SEEK_SET);
        fwrite(&(rhs->key), sizeof(int), 1, file);
        fwrite(rhs->data, structSize, 1, file);
        fwrite(&(lhs->key), sizeof(int), 1, file);
        fwrite(lhs->data, structSize, 1, file);
        return 1;
    }

    return 0;
}
#endif

#ifdef SYSTEM
void anastazja_babelkowa(int file, size_t structSize, int structCount) {
#else
void anastazja_babelkowa(FILE * file, size_t structSize, int structCount) {
#endif
    int i;
    char swapped;
    MyStruct * lhs = (MyStruct*) malloc(sizeof(MyStruct));
    lhs->data = (char*) malloc(sizeof(structSize));
    MyStruct * rhs = (MyStruct*) malloc(sizeof(MyStruct));
    rhs->data = (char*) malloc(sizeof(structSize));

    do {
        swapped = 0;
        for (i = 1; i < structCount; i++) {
            if (process(i-1, file, structSize, lhs, rhs))
                swapped = 1;
        }
    } while (swapped);

    free(lhs->data);
    free(lhs);
    free(rhs->data);
    free(rhs);
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

    anastazja_babelkowa(file, structSize, structCount);

    #ifdef SYSTEM
    fsync(file);
    close(file);
    #else
    fflush(file);
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

    struct rusage p1r, p2r;
    struct timeval p1t, p2t;

    gettimeofday(&p1t, NULL);
    getrusage(RUSAGE_SELF, &p1r);
    printStats(1, &p1r, NULL, &p1t, NULL);

    char * modeString = argv[1];
    printf("%s\n", modeString);
    if (!strcmp(modeString, "generuj")) {
        return generuj(argc, argv);
    } else if (!strcmp(modeString, "sortuj")) {
        return sortuj(argc, argv);
    }

    gettimeofday(&p2t, NULL);
    getrusage(RUSAGE_SELF, &p2r);
    printStats(2, &p2r, &p1r, &p2t, &p1t);

    return -1;
}
