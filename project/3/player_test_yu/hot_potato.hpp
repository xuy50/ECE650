#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string.h>
#include <vector>
#include <random>
#include <algorithm>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "assert.h"

// struct _potato_t{
//     int hops_left;
//     int paths[513];
// };
// typedef struct _potato_t potato_t;
class Potato{
public:
    int total_hops;
    int hops_left;
    int passNum;
    int path[513];

    Potato():total_hops(0), hops_left(0), passNum(0){
        memset(&path,0,sizeof(path));
    }
    Potato(int hops):total_hops(hops), hops_left(hops), passNum(0){
        memset(&path,0,sizeof(path));
    }

    void printPath(){
        std::cout << "Trace of potato:" << std::endl;
        for(int i = 0; i < total_hops; i++){
            std::cout << path[i];
            if(i != total_hops - 1){
                std::cout << ",";
            }
        }
        std::cout << "\n";
    }
};

int initServer(const char *port);

int initClient(const char *hostname, const char*port);
