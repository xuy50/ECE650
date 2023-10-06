#ifndef __T_POTATO_H__
#define __T_POTATO_H__

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using std::string;
using std::stringstream;
using std::vector;

#define MAX_HOP 512

typedef struct {
  int hops;
  int index;
  int path_array[MAX_HOP];
} potato_t;

class Potato {
 private:
  int hops;
  int index;
  vector<int> path;

  void addPlayerIDToPath(int playerID) {
    path[index] = playerID;
    index++;
  }

 public:
  Potato() : hops(0), index(0), path(vector<int>(MAX_HOP, 0)) {}
  Potato(int _hops) : hops(_hops), index(0), path(vector<int>(MAX_HOP, 0)) {}
  Potato(potato_t & potato_struct) :
      hops(potato_struct.hops),
      index(potato_struct.index),
      path(vector<int>(potato_struct.path_array, potato_struct.path_array + MAX_HOP)) {}

  string getTrace() {
    stringstream s_trace;
    if (index == 0) {
      return "";
    }
    s_trace << path[0];
    if (index == 1) {
      return s_trace.str();
    }
    for (int i = 1; i < index; i++) {
      s_trace << ',' << path[i];
    }
    return s_trace.str();
  }

  void passToPlayer(int playerID) {
    addPlayerIDToPath(playerID);
    this->hops--;
  }

  bool isGameOver() { return this->hops == 0; }

  potato_t toStruct() {
    potato_t potato_struct;
    potato_struct.hops = this->hops;
    potato_struct.index = this->index;
    memcpy(potato_struct.path_array, &path[0], MAX_HOP * sizeof(int));
    return potato_struct;
  }

  int getHops(){
    return this->hops;
  }
};

#endif