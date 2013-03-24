#include <config.h>
#include "memlist.h"


DescriptorNode * createDescriptorList(MyDescriptor * root) {
    DescriptorNode * ret = (DescriptorNode*) malloc(sizeof(DescriptorNode));

    ret->value = root;
    ret->next = NULL;
    ret->prev = NULL;

    return ret;
}

void defragmentDescriptorList(DescriptorNode ** head, size_t * minFree, size_t * maxFree) {
    DescriptorNode * node = *head;
    DescriptorNode * cand;

    while (node) {
        cand = findDescriptor(node->value->memory + node->value->blockCount * BLOCK_SIZE * (1 << 10), *head);

        if (cand != NULL) {
            node->value->blockCount += cand->value->blockCount;
            removeFromList(cand, head);
        }

        node = node->next;
    }

    updateExtremes(minFree, maxFree, *head);
}

MyDescriptor * internalAlloc(size_t requestedSize, DescriptorNode ** freeHead, DescriptorNode ** usedHead) {
    DescriptorNode * bestNode, * node;
    size_t nodeSize, bestSize;

    #ifdef FIRST_ALLOC
    bestNode = *freeHead;
    while (bestNode) {
        nodeSize = bestNode->value->blockCount * BLOCK_SIZE * (1 << 10);
        if (nodeSize >= requestedSize) break;
        bestNode = bestNode->next;
    }
    #endif
    #ifndef FIRST_ALLOC
    bestNode = *freeHead;
    node = bestNode;
    bestSize = node->value->blockCount * BLOCK_SIZE * (1 << 10);
    while (node) {
        nodeSize = bestNode->value->blockCount * BLOCK_SIZE * (1 << 10);

        #ifdef MAX_ALLOC
        if (nodeSize > bestSize) {
            bestSize = nodeSize;
            bestNode = node;
        }
        #endif
        #ifndef MAX_ALLOC
        if (nodeSize > requestedSize && nodeSize < bestSize) {
            bestSize = nodeSize;
            bestNode = node;
        }
        #endif

        node = node->next;
    }
    #endif

    int blocksNeeded = requestedSize / BLOCK_SIZE;
    if (requestedSize - blocksNeeded * BLOCK_SIZE > 0)
        blocksNeeded++;


    MyDescriptor * bestDescr = bestNode->value;
    bestDescr->blockCount -= blocksNeeded;

    MyDescriptor * ret = (MyDescriptor*) malloc(sizeof(MyDescriptor));
    ret->memory = bestDescr->memory;
    //ret->memory = realloc(ret->memory, blocksNeeded * BLOCK_SIZE * (1 << 10));
    ret->blockCount = blocksNeeded;

    DescriptorNode * newNode = (DescriptorNode*) malloc(sizeof(DescriptorNode));
    newNode->value = ret;
    pushFront(newNode, usedHead);

    if (bestDescr->blockCount == 0) {
        removeFromList(bestNode, freeHead);
        free(bestDescr);
        free(bestNode);
    } else {
        bestDescr->memory += blocksNeeded * BLOCK_SIZE * (1 << 10);
        //bestDescr->memory = realloc(bestDescr->memory, bestDescr->blockCount * BLOCK_SIZE * (1 << 10));
    }

    return ret;
}

DescriptorNode * findDescriptor(void * ptr, DescriptorNode * head) {
    while (head) {
        if (head->value->memory == ptr)
            return head;
        head = head->next;
    }

    return NULL;
}

void removeFromList(DescriptorNode * node, DescriptorNode ** head) {
    if (*head == node) {
        *head = (*head)->next;
        (*head)->prev = node->prev;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
}

void pushFront(DescriptorNode * node, DescriptorNode ** head) {
    node->next = *head;
    (*head)->prev = node;
    *head = node;
}

void updateExtremes(size_t * minFree, size_t * maxFree, DescriptorNode * head) {
    size_t nodeSize;

    while (head) {
        nodeSize = head->value->blockCount * BLOCK_SIZE * (1 << 10);
        if (nodeSize < minFree)
            *minFree = nodeSize;
        else if(nodeSize > maxFree)
            *maxFree = nodeSize;

        head = head->next;
    }
}
