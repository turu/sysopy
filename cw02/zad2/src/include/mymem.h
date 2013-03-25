#ifndef _mymem_h_
#define _mymem_h_

#include <stdlib.h>

struct {
    int usedCount;
    int freeCount;
    size_t minFreeSize;
    size_t maxFreeSize;
} typedef MyStatus;

void memInit(int blocks);

void finalizeMemory();

void * mylloc(size_t requestedSize);

int myfree(void * blockPtr);

MyStatus * getMyStatus();

#endif
