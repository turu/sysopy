#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "mymem.h"
#include "execution.h"

#ifdef DLL
#include <dlfcn.h>
#endif

int blocks = 500;
int myllocs = 50;
int myfrees = 50;
int maxSize = 1000;

int main() {
    #ifdef DLL
	void * handle;
	handle = dlopen("lib/libmymem.so", RTLD_LAZY);
	if(handle == NULL) {
		fprintf(stderr, "Nieudane zaladowanie biblioteki libmymem.so\n");
		return -1;
	}

    void (*memInit)(int) = dlsym(handle, "memInit");
    void (*finalizeMemory)() = dlsym(handle, "finalizeMemory");
    void * (*mylloc)(size_t) = dlsym(handle, "mylloc");
    int (*myfree)(void*) = dlsym(handle, "myfree");
    MyStatus * (*getMyStatus)() = dlsym(handle, "getMyStatus");
	#endif

    srand(time(NULL));
    int ** data = (int**) malloc(myllocs * sizeof(int*));

    printf("Execution started!!!\n");
    checkpoint();
    #ifndef DLL
    printMemStatus(getMyStatus());
    #endif

    sleep(1);

    memInit(blocks);
    printf("\nMyMem library initialized for %d blocks\n", blocks);
    checkpoint();
    #ifndef DLL
    printMemStatus(getMyStatus());
    #endif

    printf("\nAllocating chunks:\n");
    int i;
    size_t requested;
    for (i = 0; i < myllocs; i++) {
        requested = (rand() % maxSize + 1) * sizeof(int);
        printf("Chunk ID=%d, requested size = %dB\n", i, (int)requested);
        data[i] = (int*) mylloc(requested);
    }
    printf("%d chunks of max size of %dB allocated\n", myllocs, (int)(maxSize * sizeof(int)));
    checkpoint();
    #ifndef DLL
    printMemStatus(getMyStatus());
    #endif

    printf("\nDeallocating random chunks:\n");
    int id, done = 0;
    for (i = 0; i < myfrees; i++) {
        id = rand() % myllocs;
        if (data[id] != NULL) {
            myfree(data[id]);
            data[id] = NULL;
            printf("Deallocating chunk ID=%d\n", id);
            done++;
        }
    }
    printf("%d chunks deallocated\n", done);
    checkpoint();
    #ifndef DLL
    printMemStatus(getMyStatus());
    #endif

    MyStatus * status = getMyStatus();

    printf("\nTrying to allocate %dB chunk\n", (int)(status->maxFreeSize + 1));
    void * ptr = mylloc(status->maxFreeSize + 1);
    printf("Forced defragmentation. ");
    if (ptr != NULL) printf("Thanks to degragmentation, this chunk was allocated");
    putchar('\n');
    checkpoint();
    #ifndef DLL
    printMemStatus(getMyStatus());
    #endif

    finalizeMemory();
    printf("\nFinalized library\n");
    checkpoint();
    #ifndef DLL
    printMemStatus(getMyStatus());
    #endif

    free(data);

    #ifdef DLL
	dlclose(handle);
	#endif

    return 0;
}
