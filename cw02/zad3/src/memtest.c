#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "mymem.h"
#include "execution.h"

#ifdef DLL
#include <dlfnc.h>
#endif

int blocks = 500;
int myllocs = 50;
int myfrees = 50;
int maxSize = 1000;

int main() {
    srand(time(NULL));
    int ** data = (int**) malloc(myllocs * sizeof(int*));

    printf("Execution started\n");
    checkpoint();

    memInit(blocks);
    printf("MyMem library initialized for %d blocks\n", blocks);
    checkpoint();

    int i;
    size_t requested;
    for (i = 0; i < myllocs; i++) {
        requested = (rand() % maxSize + 1) * sizeof(int);
        printf("%d %d\n", i, (int)requested);
        data[i] = (int*) mylloc(requested);
    }
    printf("%d chunks of max size of %dB allocated\n", myllocs, maxSize * sizeof(int));
    checkpoint();

    int id, done = 0;
    for (i = 0; i < myfrees; i++) {
        id = rand() % myllocs;
        if (data[id] != NULL) {
            myfree(data[id]);
            data[id] = NULL;
            done++;
        }
    }
    printf("%d chunks deallocated\n", done);
    checkpoint();

    MyStatus * status = getMyStatus();

    printf("Trying to allocate %dB chunk\n", status->maxFreeSize + 1);
    void * ptr = mylloc(status->maxFreeSize + 1);
    printf("Forced defragmentation. ");
    if (ptr != NULL) printf("Thanks to degragmentation, this chunk was allocated");
    putchar('\n');
    checkpoint();

    finalizeMemory();
    printf("Finalized library\n");
    checkpoint();

    free(data);
    return 0;
}
