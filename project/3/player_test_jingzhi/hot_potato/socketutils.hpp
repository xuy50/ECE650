#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "potato.hpp"

class ClientSocket {
 private:
  int socket_fd;
  struct addrinfo * host_info_list;
  const char * hostname;
  const char * port;

 public:
  ClientSocket(const char * hostname, const char * port);
  virtual ~ClientSocket();

  void sendToServer(const void * message, int length);
  void sendPotatoToServer(Potato & hot_potato);
  void receiveFromServer(void * buf, int length);
  Potato receivePotatoFromServer();
  int getClientSocketFd();
};

class ServerSocket {
 private:
  int socket_fd;
  struct addrinfo * host_info_list;
  const char * hostname;
  const char * port;

 public:
  ServerSocket(const char * hostname, const char * port);
  virtual ~ServerSocket();

  int acceptConnection();

  void sendToClient(int client_connection_fd, const void * message, int length);
  void sendPotatoToClient(int client_connection_fd, Potato & hot_potato);
  void receiveFromClient(int client_connnection_fd, void * buf, int length);
  Potato receivePotatoFromClient(int client_connection_fd);
  uint16_t getPort();
  int getServerSocketFd();
};

typedef struct {
  char hostname[_SC_HOST_NAME_MAX];
  uint16_t port;
} hostname_port_t;
