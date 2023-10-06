CC=gcc
CFLAGS=-ggdb3 -fPIC
MALLOC_VERSION=BF
WDIR=..

all: test

test: test.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath=$(WDIR) -o $@ test.c -lmymalloc -lrt

clean:
	rm -f *~ *.o test

clobber:
	rm -f *~ *.o