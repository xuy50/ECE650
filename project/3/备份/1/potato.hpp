#ifndef _POTATO_HPP_
#define _POTATO_HPP_

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

using namespace std;

class potato {
   private:
    size_t numHops;
    size_t trace[513];
    size_t hopstep;

   public:
    potato() : numHops(0), hopstep(0) {}

    potato(size_t hopsn) : numHops(hopsn), hopstep(0) {}

    int getNumHops() { return numHops; }

    string traceToString() {
        string traceStr = "";

        for (size_t i = 0; i <= numHops; ++i) {
            ostringstream oss;
            oss << trace[i];
            traceStr += (oss.str());
            // traceStr += (trace[i]);
            if (i != numHops) {
                traceStr += ",";
            }
        }
        return traceStr;
    }

    void addTrace(size_t id) {
        trace[hopstep] = id;
        hopstep++;
    }
};

#endif