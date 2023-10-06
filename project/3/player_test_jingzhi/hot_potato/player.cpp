#include "potato.hpp"
#include "socketutils.hpp"

void checkArgc(int argc) {
  if (argc != 3) {
    std::cerr << "Usage of player: ./player <machine_name> <port_num>" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void sendCurrHostnameAndPortToMaster(ClientSocket & client_socket_for_master,
                                     ServerSocket & server_socket_for_neighbor) {
  hostname_port_t hostname_and_port;
  char hostname[_SC_HOST_NAME_MAX] = {'\0'};
  if (gethostname(hostname, _SC_HOST_NAME_MAX) != 0) {
    std::cerr << "Failed to get hostname." << std::endl;
    exit(EXIT_FAILURE);
  }

  // send player's hostname and port to ringmaster
  memset(hostname_and_port.hostname, '\0', sizeof(hostname_and_port.hostname));
  strcpy(hostname_and_port.hostname, hostname);
  hostname_and_port.port = server_socket_for_neighbor.getPort();

  client_socket_for_master.sendToServer(&hostname_and_port, sizeof(hostname_port_t));
}

void recriveAnotherHostnameAndPortFromMaster(hostname_port_t * another_hostname_and_port,
                                             ClientSocket & client_socket_for_master) {
  client_socket_for_master.receiveFromServer(another_hostname_and_port,
                                             sizeof(hostname_port_t));
}

int getRandomNum() {
  return rand() % 2;
}

bool amIIt(ClientSocket & client_socket_for_master,
           int curr_playerID,
           Potato & hot_potato) {
  hot_potato.passToPlayer(curr_playerID);
  if (hot_potato.isGameOver()) {
    client_socket_for_master.sendPotatoToServer(
        hot_potato);  // Hops = 0, send the potato back to ringmaster
    std::cout << "I'm it" << std::endl;
    return true;
  }

  return false;
}

bool playOneTurn(int playerID,
                 Potato & hot_potato,
                 ClientSocket & client_socket_for_master,
                 ClientSocket & client_socket_for_neighbor,
                 ServerSocket & server_socket_for_neighbor,
                 int client_neighbor_connection_fd,
                 int left_playerID,
                 int right_playerID) {
  if (!amIIt(client_socket_for_master, playerID, hot_potato)) {
    if (getRandomNum() == 0) {
      server_socket_for_neighbor.sendPotatoToClient(
          client_neighbor_connection_fd, hot_potato);  // send hot potato to its left
      std::cout << "Sending potato to " << left_playerID << std::endl;
      return false;
    }
    else {
      client_socket_for_neighbor.sendPotatoToServer(
          hot_potato);  // send hot potato to its right
      std::cout << "Sending potato to " << right_playerID << std::endl;
      return false;
    }
  }
  return true;
}

int main(int argc, char * argv[]) {
  checkArgc(argc);
  ClientSocket client_socket_for_master(argv[1], argv[2]);
  ServerSocket server_socket_for_neighbor(NULL, "0");

  int playerID = 0;
  client_socket_for_master.receiveFromServer(&playerID, sizeof(int));

  int num_player = 0;
  client_socket_for_master.receiveFromServer(&num_player, sizeof(int));

  srand((unsigned int)time(NULL) + playerID);

  std::cout << "Connected as player " << playerID << " out of " << num_player
            << " total players" << std::endl;

  int left_player_ID = (playerID - 1 + num_player) % num_player;
  int right_player_ID = (playerID + 1) % num_player;

  // send current player's hostname and port as a structure to ringmaster
  sendCurrHostnameAndPortToMaster(client_socket_for_master, server_socket_for_neighbor);

  // receive neighbor's hostname and port as a structure from ringmaster
  hostname_port_t neighbor_hostname_and_port;
  recriveAnotherHostnameAndPortFromMaster(&neighbor_hostname_and_port,
                                          client_socket_for_master);

  // connect to it's neighbor as a client
  stringstream port_ss;
  port_ss << neighbor_hostname_and_port.port;

  ClientSocket client_socket_for_neighbor(neighbor_hostname_and_port.hostname,
                                          port_ss.str().c_str());

  //connect to another neighbor as a server
  int client_neighbor_connection_fd = server_socket_for_neighbor.acceptConnection();

  // Notify the ringmaster that I am ready to play
  client_socket_for_master.sendToServer(&playerID, sizeof(int));

  // Listen from 3 sides: ringmaster, left neighbor, right neighbor
  fd_set readfds;
  fd_set action_fds;
  FD_ZERO(&readfds);

  int fd_max = client_socket_for_master.getClientSocketFd();
  FD_SET(client_socket_for_master.getClientSocketFd(), &readfds);

  fd_max = std::max(fd_max, client_neighbor_connection_fd);
  FD_SET(client_neighbor_connection_fd, &readfds);

  fd_max = std::max(fd_max, client_socket_for_neighbor.getClientSocketFd());
  FD_SET(client_socket_for_neighbor.getClientSocketFd(), &readfds);

  while (true) {
    action_fds = readfds;
    if (select(fd_max + 1, &action_fds, NULL, NULL, NULL) == -1) {
      std::cerr << "Select error! " << std::endl;
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(client_socket_for_master.getClientSocketFd(), &action_fds)) {
      Potato hot_potato = client_socket_for_master.receivePotatoFromServer();
      if (hot_potato.isGameOver()) {
        client_socket_for_master.sendToServer(
            &playerID, sizeof(int));  // Nofify the ring master I will exit.
        break;  // Receive gameover info from ringmaster, time to exit
      }
      if (playOneTurn(playerID,
                      hot_potato,
                      client_socket_for_master,
                      client_socket_for_neighbor,
                      server_socket_for_neighbor,
                      client_neighbor_connection_fd,
                      left_player_ID,
                      right_player_ID)) {
        // break;
      }
    }

    if (FD_ISSET(client_neighbor_connection_fd, &action_fds)) {
      Potato hot_potato = server_socket_for_neighbor.receivePotatoFromClient(
          client_neighbor_connection_fd);
      if (playOneTurn(playerID,
                      hot_potato,
                      client_socket_for_master,
                      client_socket_for_neighbor,
                      server_socket_for_neighbor,
                      client_neighbor_connection_fd,
                      left_player_ID,
                      right_player_ID)) {
        // break;
      }
    }

    if (FD_ISSET(client_socket_for_neighbor.getClientSocketFd(), &action_fds)) {
      Potato hot_potato = client_socket_for_neighbor.receivePotatoFromServer();
      if (playOneTurn(playerID,
                      hot_potato,
                      client_socket_for_master,
                      client_socket_for_neighbor,
                      server_socket_for_neighbor,
                      client_neighbor_connection_fd,
                      left_player_ID,
                      right_player_ID)) {
        // break;
      }
    }
  }

  Potato endGame_potato = client_socket_for_master.receivePotatoFromServer();

  if (endGame_potato.getHops() != 0) {
    std::cout << "The final end game potato is " << endGame_potato.getHops() << std::endl;
  }

  return EXIT_SUCCESS;
}
