#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mymem.h"
#include "execution.h"

#ifdef DLL
#include <dlfnc.h>
#endif

int blocks = 500;
int myllocs = 500;
int myfrees = 500;
int maxSize = 1000;

int main() {
    srand(time(NULL));
    int ** data = (int**) malloc(myllocs * sizeof(int*));

    printf("Execution started\n");
    checkpoint();

    memInit(blocks);
    printf("MyMem library initialized for %d blocks", BLOCKS);
    checkpoint();

    int i;
    for (i = 0; i < myllocs; i++) {
        data[i] = (int*) mylloc((rand() % maxSize + 1) * sizeof(int));
    }
    printf("%d chunks of max size of %d integers allocated\n", myllocs, maxSize);
    checkpoint();

    int id, done = 0;
    for (i = 0; i < myfrees; i++) {
        id = rand() % myllocs;
        if (data[id] != NULL) {
            myfree(data[id]);
            done++;
        }
    }
    printf("%d chunks deallocated\n", done);
    checkpoint();

    mylloc(getMyStatus()->maxFreeSize + 1);
    printf("Forced defragmentation\n");
    checkpoint();

    finalizeMemory();
    printf("Finalized library\n");
    checkpoint();

    free(data);
    return 0;
}
