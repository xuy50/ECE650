#include <cstdio>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <assert.h>

typedef struct potato_t{
    int total_hops;
    int num_hops;
    int traces[512];
}potato;

struct player_info{
  char ipAddress[INET6_ADDRSTRLEN];
  unsigned int port_num;
};

void printErrorMsg(std::string msg){
  std::cerr << msg << std::endl;
  exit(EXIT_FAILURE);
}

long int convertNum(char* line){
  char * ptr;
  long int result = strtol(line, &ptr, 10);
  if (*ptr != '\0') {
    printErrorMsg("encounter some character before number in words");
  }
  // there is no digits at all
  if (result == 0 && *(ptr - 1) != 0) {
    printErrorMsg("there is no digits at all");
  }
  // result is not a valid number
  if (result < 0) {
    printErrorMsg("result is not a valid number");
  }
  return result;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int try_send_all(int sock_fd, void* data, size_t data_len, int flag){
    size_t total = 0;
    size_t byteleft = data_len;
    int n;
    while (total < data_len){
      n = send(sock_fd, (char*)data + total, byteleft, flag); 
      if (n == -1) {break;}
      total += n;
      byteleft -= n;
    }
    return n == -1 ? -1 : 0;
}


unsigned short get_in_port(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    }
    return (((struct sockaddr_in6*)sa)->sin6_port);
}


int start_listen(char* port_num){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port     = port_num;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl; 
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if
  freeaddrinfo(host_info_list);
  return socket_fd;
}

int connect_to_server(char* hostname_input, char* port_input){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = hostname_input;
  const char *port     = port_input;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } //if
  freeaddrinfo(host_info_list);
  return socket_fd;
}


std::string getIpaddress(int socket_fd, const std::string& mode){
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);

  char ipAddress[INET6_ADDRSTRLEN];
  if (mode == "Own"){
    getsockname(socket_fd, (struct sockaddr*)&socket_addr, &socket_addr_len);//获取sockfd表示的连接上的本地地址
  }
  if (mode == "Peer"){
    getpeername(socket_fd, (struct sockaddr*)&socket_addr, &socket_addr_len);//获取sockfd表示的连接上的本地地址
  }
  inet_ntop(socket_addr.ss_family,
    get_in_addr((struct sockaddr *)&socket_addr),
    ipAddress, sizeof ipAddress);
  std::cout << std::string(ipAddress, strlen(ipAddress)) << std::endl;
  return std::string(ipAddress, strlen(ipAddress));
}

unsigned int getPort(int socket_fd, const std::string& mode){
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  char ipAddress[INET6_ADDRSTRLEN];
  if (mode == "Own"){
    getsockname(socket_fd, (struct sockaddr*)&socket_addr, &socket_addr_len);//获取sockfd表示的连接上的本地地址
  }
  if (mode == "Peer"){
    getpeername(socket_fd, (struct sockaddr*)&socket_addr, &socket_addr_len);//获取sockfd表示的连接上的本地地址
  }

  inet_ntop(socket_addr.ss_family,
      get_in_addr((struct sockaddr *)&socket_addr),
      ipAddress, sizeof ipAddress);
  unsigned int client_port_num = ntohs(get_in_port((struct sockaddr *)&socket_addr));
  return client_port_num;
}


int getPlayerNum(int own, int total, int mode){
  if (mode == 0){
     // get prev player number
    if (own == 0){
      return total - 1;
    } else {
      return own - 1;
    }
  } else{
    // get next player number
    return (own + 1) % total;
  }
}



int tryAccept(int listen_socket_fd){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd = client_connection_fd = accept(listen_socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      std::cerr << "Error: cannot accept connection on socket" << std::endl;
      return -1;
    }
    return client_connection_fd;
}