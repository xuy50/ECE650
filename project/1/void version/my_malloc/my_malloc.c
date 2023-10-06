#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "my_malloc.h"

md_t *freeList = NULL;
md_t *flTail = NULL;
size_t segSize = 0;
size_t freeSize = 0;
size_t md_tSize = sizeof(md_t);

// help funcitons for test
void printFreeList()
{
    printf("Free region list: total free region size: %zu, heap size: %zu\n", freeSize, segSize);
    md_t *curr = freeList;
    while (curr != NULL)
    {
        printf("    pos: %p, size: %zu\n", curr, curr->size);
        curr = curr->next;
    }
}

void printFreeInfo(md_t *fr)
{
    printf("Freeing: pos: %p, size: %zu\n", fr, fr->size);
}

// help function for merge the continuous free regions
void mergeFreeList()
{
    md_t *curr = freeList;
    // loop 1
    while (curr != NULL)
    {
        if (curr->next != NULL)
        {
            if (((void *)curr + curr->size) == curr->next)
            {
                md_t *nextTemp = curr->next;
                curr->size = curr->size + curr->next->size;
                if (nextTemp->next != NULL)
                    curr->next = nextTemp->next;
                else
                    curr->next = NULL;
                // nextTemp->prev = curr;
                nextTemp->prev = NULL;
                nextTemp->next = NULL;
                continue;
            }
        }
        // if (curr->next != NULL)
        curr = curr->next;
        // else
        //     break;
    }
}

// First Fit malloc/free
void *ff_malloc(size_t size)
{
    size_t tSize = size + md_tSize;
    // new malloc address position, which need to be return
    md_t *newMd = NULL;

    int freeRegionEnough = 0;
    md_t *curr = freeList; // get free region from free list
    // loop 2
    while (curr != NULL)
    {
        // if the free region size is enough (>=0)
        if (curr->size >= tSize)
        {
            size_t isEnough = curr->size - tSize; // current free region size - total malloc size
            // printf("ls: %zu\n", isEnough); // loop test
            freeRegionEnough = 1;
            newMd = curr;
            newMd->isFree = 0;
            // if the left region is not enough for other one malloc use
            // all this free region space, else segement the used space
            // and left space
            if (isEnough <= md_tSize)
            {
                newMd->size = curr->size;
                if (curr->prev == NULL)
                {
                    // freeList = curr->next;
                    // freeList->prev = NULL;
                    if (curr->next != NULL)
                    {
                        md_t *headTemp = curr->next;
                        freeList = headTemp;
                        headTemp->prev = NULL;
                    }
                    else
                    {
                        freeList = NULL;
                    }
                }
                else
                {
                    if (curr->next != NULL)
                    {
                        curr->prev->next = curr->next;
                        curr->next->prev = curr->prev;
                    }
                    else
                    {
                        curr->prev->next = NULL;
                    }
                }
                newMd->next = NULL;
                newMd->prev = NULL;
                // decreace the free region size
                freeSize -= curr->size;
            }
            else
            {
                newMd->size = tSize;
                // get the left free region for free list
                md_t *newCurr = (void *)curr + tSize;
                newCurr->size = isEnough;
                newCurr->isFree = 1;
                if (curr->prev != NULL)
                    newCurr->prev = curr->prev;
                else
                    newCurr->prev = NULL;
                if (curr->next != NULL)
                    newCurr->next = curr->next;
                else
                    newCurr->next = NULL;
                if (curr->prev == NULL)
                {
                    freeList = newCurr;
                    freeList->prev = NULL;
                }
                else
                {
                    curr->prev->next = newCurr;
                }
                if (curr->next != NULL)
                    curr->next->prev = newCurr;
                newMd->next = NULL;
                newMd->prev = NULL;
                // decreace the free region size
                freeSize -= tSize;
            }
            break;
        }
        curr = curr->next;
    }
    // if the free region is not enough,
    // use sbrk to increase heap space
    if (freeRegionEnough == 0)
    {
        newMd = sbrk(tSize);
        if (newMd == (void *)(-1))
        {
            return NULL;
        }
        segSize += tSize;
        newMd->size = tSize;
        newMd->isFree = 0;
        newMd->next = NULL;
        newMd->prev = NULL;
    }
    // printf("mlc: pos: %p, size: %zu, tSize: %zu\n", newMd, newMd->size, tSize); //test
    // printf("     pos: %p\n", (void*)newMd + md_tSize); //test
    return (void *)newMd + md_tSize;
}

// first fit free help
// merge next
void mergeNext(md_t *curr, md_t *newFR)
{
    newFR->size += curr->size;
    if (curr->next != NULL)
    {
        newFR->next = curr->next;
        curr->next->prev = newFR;
        curr->prev = NULL;
        curr->next = NULL;
    }
    else
    {
        newFR->next = NULL;
        curr->prev = NULL;
        curr->next = NULL;
    }
}

// first fit free
void ff_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    // printf("ptr pos: %p\n", ptr); // test
    //  get the metadata point position
    md_t *newFR;
    newFR = (md_t *)((void *)ptr - md_tSize);
    // printFreeInfo(ptr - md_tSize); // test

    // chcek whether the region is double freed
    // if (newFR->isFree == 1)
    // {
    //     fprintf(stderr, "ERROR: double free \n");
    // }

    // add new free region to freed region size
    freeSize += newFR->size;
    // change the region state to freed
    newFR->isFree = 1;

    // add the freed region to the free list
    md_t *curr = freeList;
    int isAdded = 0;
    if (freeList == NULL)
    {
        freeList = newFR;
        newFR->next = NULL;
        newFR->prev = NULL;
        // printf("flhead: prev: %p, next: %p\n", newFR->next, newFR->prev);
        isAdded = 1;
    }
    else
    {
        // loop 3
        while (curr != NULL && isAdded == 0)
        {
            // printf("flcurr: prev: %p, next: %p\n", curr->next, curr->prev); // test
            // printf("newfl: prev: %p, next: %p\n", newFR->next, newFR->prev); // test

            // printf("loop1"); // loop test
            if (curr > newFR)
            {
                // printf("free check: pos: %p, size: %zu\n", newFR, newFR->size); //test
                // newFR->next = curr;
                if (curr->prev != NULL)
                {
                    if (((void *)curr->prev + curr->prev->size) == newFR)
                    {
                        // check prev
                        newFR->prev = NULL;
                        newFR->next = NULL;
                        curr->prev->size += newFR->size;
                        newFR = curr->prev;
                        // check next
                        if (((void *)newFR + newFR->size) == curr)
                        {
                            mergeNext(curr, newFR);
                        }
                    }
                    else
                    {
                        newFR->prev = curr->prev;
                        curr->prev->next = newFR;

                        if (((void *)newFR + newFR->size) == curr)
                        {
                            mergeNext(curr, newFR);
                        }
                        else
                        {
                            newFR->next = curr;
                            curr->prev = newFR;
                        }
                    }
                }
                else
                {
                    // check next
                    if (((void *)newFR + newFR->size) == curr)
                    {
                        mergeNext(curr, newFR);
                    }
                    else
                    {
                        curr->prev = newFR;
                        newFR->next = curr;
                    }
                    freeList = newFR;
                    newFR->prev = NULL;
                }
                // curr->prev = newFR;
                isAdded = 1;

                break;
            }
            if (curr->next != NULL)
                curr = curr->next;
            else
                break;
        }
        if (isAdded == 0)
        {
            if (((void *)curr + curr->size) == newFR)
            {
                // check prev
                newFR->prev = NULL;
                newFR->next = NULL;
                curr->size += newFR->size;
                newFR = curr;
            }
            else
            {
                curr->next = newFR;
                newFR->prev = curr;
                newFR->next = NULL;
            }
        }
    }

    // mergeFreeList();
}

// Best Fit malloc/free
void *bf_malloc(size_t size)
{
    size_t tSize = size + md_tSize;
    // new malloc address position, which need to be return
    md_t *newMd = NULL;

    int freeRegionEnough = 0;
    md_t *target = NULL;
    md_t *curr = freeList; // get free region from free list
    // loop 2
    while (curr != NULL)
    {
        if (curr->size == tSize)
        {
            target = curr;
            freeRegionEnough = 1;
            break;
        }
        else if (curr->size > tSize)
        {
            freeRegionEnough = 2;
            if(target == NULL){
                target = curr;
            }
            else if (curr->size < target->size)
            {
                target = curr;
            }
        }
        curr = curr->next;
    }

    // if the free region size is enough (>=0)
    if (freeRegionEnough != 0)
    {
        size_t leftSize = target->size - tSize; // target free region size - total malloc size
        if ((freeRegionEnough == 2) && (leftSize <= md_tSize))
        {
            freeRegionEnough = 1;
        }
        newMd = target;
        newMd->isFree = 0;
        // if the left region is not enough for other one malloc use
        // all this free region space, else segement the used space
        // and left space
        if (freeRegionEnough == 1)
        {
            newMd->size = target->size;
            if (target->prev == NULL)
            {
                // freeList = curr->next;
                // freeList->prev = NULL;
                if (target->next != NULL)
                {
                    md_t *headTemp = target->next;
                    freeList = headTemp;
                    headTemp->prev = NULL;
                }
                else
                {
                    freeList = NULL;
                }
            }
            else
            {
                if (target->next != NULL)
                {
                    target->prev->next = target->next;
                    target->next->prev = target->prev;
                }
                else
                {
                    target->prev->next = NULL;
                }
            }
            newMd->next = NULL;
            newMd->prev = NULL;
            // decreace the free region size
            freeSize -= target->size;
        }
        else if(freeRegionEnough == 2)
        {
            newMd->size = tSize;
            // get the left free region for free list
            md_t *newCurr = (void *)target + tSize;
            newCurr->size = leftSize;
            newCurr->isFree = 1;
            if (target->prev != NULL)
                newCurr->prev = target->prev;
            else
                newCurr->prev = NULL;
            if (target->next != NULL)
                newCurr->next = target->next;
            else
                newCurr->next = NULL;
            if (target->prev == NULL)
            {
                freeList = newCurr;
                freeList->prev = NULL;
            }
            else
            {
                target->prev->next = newCurr;
            }
            if (target->next != NULL)
                target->next->prev = newCurr;
            newMd->next = NULL;
            newMd->prev = NULL;
            // decreace the free region size
            freeSize -= tSize;
        }
    }

    // if the free region is not enough,
    // use sbrk to increase heap space
    if (freeRegionEnough == 0)
    {
        newMd = sbrk(tSize);
        if (newMd == (void *)(-1))
        {
            return NULL;
        }
        segSize += tSize;
        newMd->size = tSize;
        newMd->isFree = 0;
        newMd->next = NULL;
        newMd->prev = NULL;
    }

    return (void *)newMd + md_tSize;
}

// best fit free which is the same as the first fit free
void bf_free(void *ptr) { ff_free(ptr); }

unsigned long get_data_segment_size() // in bytes
{
    return segSize;
}

unsigned long get_data_segment_free_space_size() // in bytes
{
    return freeSize;
}