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


void memInit(int blocks) {
    _totalFreeSize = blocks * BLOCK_SIZE * (1 << 10);

    MyDescriptor * descriptor = (MyDescriptor*) malloc(sizeof(MyDescriptor));
    descriptor->memory = malloc(_totalFreeSize);
    descriptor->blockCount = blocks;

    _freeList = createDescriptorList(descriptor);
    _usedList = NULL;
    _minFreeSize = blocks * BLOCK_SIZE;
    _maxFreeSize = _minFreeSize;
    _freeCount = blocks;
    _initialized = 1;
}

void * mylloc(size_t requestedSize) {
    if (!_initialized || requestedSize > _totalFreeSize)
        return NULL;

    if (requestedSize > _maxFreeSize)
        defragmentDescriptorList(&_freeList, &_minFreeSize, &_maxFreeSize);

    if (requestedSize > _maxFreeSize)
        return NULL;

    MyDescriptor * ret = internalAlloc(requestedSize, &_freeList, &_usedList);

    _usedCount++;
    _freeCount--;
    _totalFreeSize += requestedSize;

    return ret->memory;
}

int myfree(void * blockPtr) {
    if (!_initialized)
        return -1;

    DescriptorNode * node = findDescriptor(blockPtr, _usedList);
    size_t nodeSize = node->value->blockCount * BLOCK_SIZE * (1 << 10);

    if (node == NULL)
        return 0;

    removeFromList(node, &_usedList);
    pushFront(node, &_freeList);

    //free(node->value->memory);
    free(node->value);
    free(node);

    _usedCount--;
    _freeCount++;
    _totalFreeSize += nodeSize;

    if (nodeSize < _minFreeSize)
        _minFreeSize = nodeSize;
    else if (nodeSize > _maxFreeSize)
        _maxFreeSize = nodeSize;

    return 1;
}

MyStatus * getMyStatus() {
    if (!_initialized)
        return NULL;

    updateExtremes(&_minFreeSize, &_maxFreeSize, _freeList);

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
        free(node->value->memory);
        free(node->value);
        free(node);
    }

    while (_freeCount--) {
        node = _freeList;
        removeFromList(node, &_freeList);
        free(node->value);
        free(node);
    }

    _minFreeSize = INT_MAX;
    _maxFreeSize = INT_MIN;
    _initialized = 0;
    _totalFreeSize = 0;
}
