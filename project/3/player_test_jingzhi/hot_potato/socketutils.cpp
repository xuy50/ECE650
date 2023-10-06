#include "socketutils.hpp"

ClientSocket::ClientSocket(const char * _hostname, const char * _port) :
    hostname(_hostname), port(_port) {
  struct addrinfo host_info;
  std::memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

ClientSocket::~ClientSocket() {
  freeaddrinfo(host_info_list);
  close(socket_fd);
}

void ClientSocket::sendToServer(const void * message, int length) {
  if (send(this->socket_fd, message, length, 0) == -1) {
    std::cerr << "Error: sendToServer failed! The length try to send is :" << length
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

void ClientSocket::sendPotatoToServer(Potato & hot_potato) {
  potato_t hot_potato_struct = hot_potato.toStruct();
  this->sendToServer(&hot_potato_struct, sizeof(potato_t));
}

void ClientSocket::receiveFromServer(void * buf, int length) {
  if (recv(this->socket_fd, buf, length, MSG_WAITALL) == -1) {
    std::cerr << "Error: receiveFromServer failed! The length try to send is :" << length
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

Potato ClientSocket::receivePotatoFromServer() {
  potato_t struct_hot_potato;
  this->receiveFromServer(&struct_hot_potato, sizeof(potato_t));
  return Potato(struct_hot_potato);
}

int ClientSocket::getClientSocketFd(){
  return this->socket_fd;
}

ServerSocket::ServerSocket(const char * _hostname, const char * _port) :
    hostname(_hostname), port(_port) {
  struct addrinfo host_info;
  std::memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host " << gai_strerror(status)
              << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  status = listen(socket_fd, SOMAXCONN);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

ServerSocket::~ServerSocket() {
  freeaddrinfo(host_info_list);
  close(socket_fd);
}

int ServerSocket::acceptConnection() {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  return client_connection_fd;
}

void ServerSocket::sendToClient(int client_connection_fd,
                                const void * message,
                                int length) {
  if (send(client_connection_fd, message, length, 0) == -1) {
    std::cerr << "Error: sendToClient failed! The length try to send is :" << length
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

void ServerSocket::sendPotatoToClient(int client_connection_fd, Potato & hot_potato) {
  potato_t hot_potato_struct = hot_potato.toStruct();
  this->sendToClient(client_connection_fd, &hot_potato_struct, sizeof(potato_t));
}

void ServerSocket::receiveFromClient(int client_connnection_fd, void * buf, int length) {
  if (recv(client_connnection_fd, buf, length, MSG_WAITALL) == -1) {
    std::cerr << "Error: receiveFromClient failed! The length try to send is :" << length
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

Potato ServerSocket::receivePotatoFromClient(int client_connnection_fd) {
  potato_t struct_hot_potato;
  this->receiveFromClient(client_connnection_fd, &struct_hot_potato, sizeof(potato_t));
  return Potato(struct_hot_potato);
}

uint16_t ServerSocket::getPort() {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  if (getsockname(this->socket_fd, (struct sockaddr *)&addr, &len) == -1) {
    std::cerr << "getsockname failed!" << std::endl;
    exit(EXIT_FAILURE);
  }

  return ntohs(addr.sin_port);
}

int ServerSocket::getServerSocketFd() {
  return this->socket_fd;
}
