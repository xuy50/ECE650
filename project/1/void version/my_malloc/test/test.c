#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p) ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p) bf_free(p)
#endif

int main(int argc, char *argv[])
{
    printf("md_t size: %zu\n", sizeof(md_t));

    int *t2 = bf_malloc(sizeof(int));
    printf("2:\npos: %p, size: %zu\n", t2, sizeof(int)+32);
    // ff_free(t2);

    double *t3 = bf_malloc(sizeof(double));
    printf("3:\npos: %p, size: %zu\n", t3, sizeof(double)+32);

    printFreeList();
    ff_free(t2);
    printFreeList();

    double *t4 = bf_malloc(sizeof(double));
    printf("4:\npos: %p, size: %zu\n", t4, sizeof(double)+32);

    bf_free(t4);
    printFreeList();
    // bf_free(t3);
    // printFreeList();
    
    int *t1 = bf_malloc(sizeof(int));
    printf("1:\npos: %p, size: %zu\n", t1, sizeof(int)+32);

    printFreeList();
    bf_free(t1);
    printFreeList();
    bf_free(t3);
    printFreeList();

    return 0;
}
