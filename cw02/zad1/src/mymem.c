#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <config.h>
#include "mymem.h"
#include "memlist.h"


DescriptorNode * _usedList;
int _usedCount = 0;

DescriptorNode * _freeList;
int _freeCount = 0;

size_t _minFreeSize = INT_MAX;
size_t _maxFreeSize = INT_MIN;

char _initialized = 0;

size_t _totalFreeSize = 0;

void * _entry;


void memInit(int blocks) {
    _totalFreeSize = blocks * BLOCK_SIZE * (1 << 10);

    MyDescriptor * descriptor = (MyDescriptor*) malloc(sizeof(MyDescriptor));
    descriptor->memory = malloc(_totalFreeSize);
    _entry = descriptor->memory;
    descriptor->blockCount = blocks;

    _freeList = createDescriptorList(NULL);
    pushFront(createDescriptorList(descriptor), &_freeList);
    _usedList = createDescriptorList(NULL);
    _minFreeSize = _totalFreeSize;
    _maxFreeSize = _minFreeSize;
    _freeCount = 1;
    _initialized = 1;
}

void * mylloc(size_t requestedSize) {
    if (!_initialized || requestedSize > _totalFreeSize)
        return NULL;

    if (requestedSize > _maxFreeSize)
        defragmentDescriptorList(&_freeList, &_minFreeSize, &_maxFreeSize, &_freeCount);

    if (requestedSize > _maxFreeSize)
        return NULL;

    //printf("About to enter internalAlloc\n");
    MyDescriptor * ret = internalAlloc(requestedSize, &_freeList, &_usedList, &_usedCount, &_freeCount);

    //printf("Allocated chunk: blocks=%d, ptr=%d\n", ret->blockCount, ret->memory);

    _totalFreeSize -= requestedSize;

    return ret->memory;
}

int myfree(void * blockPtr) {
    if (!_initialized)
        return -1;

    DescriptorNode * node = findDescriptor(blockPtr, _usedList);
    //printf("Chunk to free: blocks=%d, ptr=%d\n", node->value->blockCount, node->value->memory);

    size_t nodeSize = node->value->blockCount * (BLOCK_SIZE << 10);

    if (node == NULL)
        return 0;

    removeFromList(node, &_usedList);
    pushFront(node, &_freeList);

    _usedCount--;
    _freeCount++;
    _totalFreeSize += nodeSize;

    if (nodeSize < _minFreeSize)
        _minFreeSize = nodeSize;
    if (nodeSize > _maxFreeSize)
        _maxFreeSize = nodeSize;

    return 1;
}

MyStatus * getMyStatus() {
    if (!_initialized)
        return NULL;

    updateExtremes(&_minFreeSize, &_maxFreeSize, _freeList, &_freeCount);

    MyStatus * ret = (MyStatus*) malloc(sizeof(MyStatus));

    ret->usedCount = _usedCount;
    ret->freeCount = _freeCount;
    ret->minFreeSize = _minFreeSize;
    ret->maxFreeSize = _maxFreeSize;

    return ret;
}

void finalizeMemory() {
    if (!_initialized) return;

    DescriptorNode * node;

    while (_usedCount--) {
        node = _usedList;
        removeFromList(node, &_usedList);
        free(node->value);
        free(node);
    }

    while (_freeCount--) {
        node = _freeList;
        removeFromList(node, &_freeList);
        free(node->value);
        free(node);
    }

    free(_entry);

    _minFreeSize = INT_MAX;
    _maxFreeSize = 0;
    _initialized = 0;
    _totalFreeSize = 0;
}
