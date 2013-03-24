#ifndef _memlist_h_
#define _memlist_h_

#include <stdlib.h>
#include <string.h>

typedef struct {
    void * memory;
    int blockCount;
} MyDescriptor;

typedef struct DescriptorNode {
    MyDescriptor * value;
    struct DescriptorNode * next;
    struct DescriptorNode * prev;
} DescriptorNode;


DescriptorNode * createDescriptorList(MyDescriptor * root);

void defragmentDescriptorList(DescriptorNode ** head, size_t * minFree, size_t * maxFree);

MyDescriptor * internalAlloc(size_t requestedSize, DescriptorNode ** freeHead, DescriptorNode ** usedHead);

DescriptorNode * findDescriptor(void * ptr, DescriptorNode * usedHead);

void removeFromList(DescriptorNode * node, DescriptorNode ** head);

void pushFront(DescriptorNode * node, DescriptorNode ** head);

void updateExtremes(size_t * minFree, size_t * maxFree, DescriptorNode * head);

#endif
