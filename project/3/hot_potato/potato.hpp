#ifndef _POTATO_HPP_
#define _POTATO_HPP_

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct potato {
    int numHops;
    int trace[513];
    int leftHops;
};
typedef struct potato ptt_t;

string traceToString(int numHops, int trace[513]) {
    string traceStr = "";

    for (int i = 0; i < numHops; ++i) {
        ostringstream oss;
        oss << trace[i];
        traceStr += (oss.str());
        // traceStr += (trace[i]);
        if (i != numHops - 1) {
            traceStr += ",";
        }
    }
    return traceStr;
}

#endif