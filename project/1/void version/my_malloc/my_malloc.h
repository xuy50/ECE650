#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct metadata
{
    size_t size;
    int isFree;
    struct metadata *prev;
    struct metadata *next;
};
typedef struct metadata md_t;

// First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);
// Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

unsigned long get_data_segment_size();            // in bytes
unsigned long get_data_segment_free_space_size(); // in byte

// help funcitons for test
void printFreeList();