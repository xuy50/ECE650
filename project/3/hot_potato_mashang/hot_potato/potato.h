#ifndef POTATO_H
#define POTATO_H

struct potato_tag{
    int numHops;
    int leftHops;
    int numPlayers;
    int count;
    int trace[512];
};

typedef struct potato_tag potato_t;
#endif
