#include "potato.hpp"
#include "socketutils.hpp"

void checkStringIsAllNumber(const char * str) {
  for (size_t i = 0; i < strlen(str); i++) {
    if (str[i] > '9' || str[i] < '0') {
      std::cerr << "Argument should consist of numbers, but is " << str << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

int stringToInt(const char * str) {
  checkStringIsAllNumber(str);
  int ans = strtol(str, NULL, 10);
  if (errno == ERANGE) {
    std::cerr << "Number is out of the range of representable values by a long int. Your "
                 "input: "
              << str << std::endl;
    exit(EXIT_FAILURE);
  }
  return ans;
}

void checkNumPlayers(const int & num_players) {
  if (num_players <= 1) {
    std::cerr << "num_players must be greater than 1, but is " << num_players
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

void checkNumHops(const int & num_hops) {
  if (num_hops < 0 || num_hops > 512) {
    std::cerr << "num_hops must be greater than or equal to zero and less than or equal "
                 "to 512, but is "
              << num_hops << std::endl;
    exit(EXIT_FAILURE);
  }
}

void parseCommandLineArguments(int argc,
                               char * argv[],
                               int & num_players,
                               int & num_hops) {
  if (argc != 4) {
    std::cerr << "Usage of ringmaster: ./ringmaster <port_num> <num_players> <num_hops>"
              << std::endl;
    exit(EXIT_FAILURE);
  }

  num_players = stringToInt(argv[2]);
  checkNumPlayers(num_players);
  num_hops = stringToInt(argv[3]);
  checkNumHops(num_hops);
}

void printMasterInfo(string name, int num_players, int num_hops) {
  std::cout << name << std::endl;
  std::cout << "Players = " << num_players << std::endl;
  std::cout << "Hops = " << num_hops << std::endl;
}

// Accept connections from all the players
vector<int> get_all_client_connection_fds(
    vector<hostname_port_t> & all_hostname_and_port_info,
    ServerSocket & server_socket,
    int num_players) {
  vector<int> all_client_connection_fds(num_players);
  for (size_t i = 0; i < all_client_connection_fds.size(); i++) {
    all_client_connection_fds[i] = server_socket.acceptConnection();
    int playerID = i;
    server_socket.sendToClient(all_client_connection_fds[i], &playerID, sizeof(int));
    server_socket.sendToClient(all_client_connection_fds[i], &num_players, sizeof(int));
    hostname_port_t curr_hostname_and_port;
    server_socket.receiveFromClient(
        all_client_connection_fds[i], &curr_hostname_and_port, sizeof(hostname_port_t));
    all_hostname_and_port_info[i] = curr_hostname_and_port;
  }
  return all_client_connection_fds;
}

void sendNeighborInfoToClient(vector<int> & all_client_connection_fds,
                              vector<hostname_port_t> & all_hostname_and_port_info,
                              ServerSocket & server_socket) {
  for (size_t i = 0; i < all_hostname_and_port_info.size(); i++) {
    server_socket.sendToClient(
        all_client_connection_fds[i],
        &all_hostname_and_port_info[(i + 1) % all_hostname_and_port_info.size()],
        sizeof(hostname_port_t));
  }
}

void receiveInfoFromAllPlayer(ServerSocket & server_socket,
                              vector<int> & all_client_connection_fds,
                              int num_players,
                              int ready_or_exit) {
  int ready_player_num = 0;
  fd_set readfds;
  fd_set action_readfds;
  FD_ZERO(&readfds);

  int fd_max = all_client_connection_fds[0];

  for (int i = 0; i < num_players; i++) {
    FD_SET(all_client_connection_fds[i], &readfds);
    fd_max = std::max(fd_max, all_client_connection_fds[i]);
  }

  while (ready_player_num != num_players) {
    action_readfds = readfds;
    if (select(fd_max + 1, &action_readfds, NULL, NULL, NULL) == -1) {
      std::cerr << "Select error! "
                << "Currently " << ready_player_num << " players are ready!" << std::endl;
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_players; i++) {
      if (FD_ISSET(all_client_connection_fds[i], &action_readfds)) {
        int ready_playerID = 0;
        server_socket.receiveFromClient(
            all_client_connection_fds[i], &ready_playerID, sizeof(int));
        if (ready_playerID != i) {
          std::cout << "Received ID: " << ready_playerID << std::endl;
          std::cerr << "Error playerID ! Expected: " << i << " but is: " << ready_playerID
                    << std::endl;
          exit(EXIT_FAILURE);
        }
        if (ready_or_exit == 0) {
          std::cout << "Player " << ready_playerID << " is ready to play" << std::endl;
        }

        ready_player_num++;
      }
    }
  }
}

void receiveReadyInfo(ServerSocket & server_socket,
                      vector<int> & all_client_connection_fds,
                      int num_players) {
  receiveInfoFromAllPlayer(server_socket, all_client_connection_fds, num_players, 0);
}

void receiveExitInfo(ServerSocket & server_socket,
                     vector<int> & all_client_connection_fds,
                     int num_players) {
  receiveInfoFromAllPlayer(server_socket, all_client_connection_fds, num_players, 1);
}

int getRandomPlayerID(int num_players) {
  return rand() % num_players;
}

void sendGameOverInfo(ServerSocket & server_socket,
                      vector<int> & all_client_connection_fds) {
  Potato end_potato(0);
  for (size_t i = 0; i < all_client_connection_fds.size(); i++) {
    server_socket.sendPotatoToClient(all_client_connection_fds[i], end_potato);
  }
}

void quitGame(ServerSocket & server_socket,
              vector<int> & all_client_connection_fds,
              int num_players) {
  sendGameOverInfo(server_socket, all_client_connection_fds);
  receiveExitInfo(server_socket, all_client_connection_fds, num_players);
  sendGameOverInfo(server_socket, all_client_connection_fds);
}

int main(int argc, char * argv[]) {
  int num_players = 0;
  int num_hops = 0;
  parseCommandLineArguments(argc, argv, num_players, num_hops);
  printMasterInfo("Potato Ringmaster", num_players, num_hops);

  srand((unsigned int)time(NULL));
  ServerSocket server_socket(NULL, argv[1]);

  vector<hostname_port_t> all_hostname_and_prot_info(num_players);

  // Get all players' client_connection_fd and their hostname_and_port
  vector<int> all_client_connection_fds = get_all_client_connection_fds(
      all_hostname_and_prot_info, server_socket, num_players);

  // Send to all players about their neighbor's hostname_and_port
  // Rules: send to player 0 with player 1's hostname and port, send to player 1 with player 2's hostname and port ...
  sendNeighborInfoToClient(
      all_client_connection_fds, all_hostname_and_prot_info, server_socket);

  // use select() to receiev "Ready message" from player
  receiveReadyInfo(server_socket, all_client_connection_fds, num_players);

  // hops = 0, immediately shuts down the game, no trace is print.
  if (num_hops == 0) {
    quitGame(server_socket, all_client_connection_fds, num_players);
          std::cout << "Trace of potato:\n" << std::endl;
    return EXIT_SUCCESS;
  }

  // Create potato object and pass it to a player randomly
  Potato hot_potato(num_hops);
  int random_playerID = getRandomPlayerID(num_players);
  std::cout << "Ready to start the game, sending potato to player " << random_playerID
            << std::endl;
  server_socket.sendPotatoToClient(all_client_connection_fds[random_playerID],
                                   hot_potato);

  // Listen to all players for the return of hot potato
  fd_set readfds;
  FD_ZERO(&readfds);

  int fd_max = all_client_connection_fds[0];

  for (int i = 0; i < num_players; i++) {
    FD_SET(all_client_connection_fds[i], &readfds);
    fd_max = std::max(fd_max, all_client_connection_fds[i]);
  }

  if (select(fd_max + 1, &readfds, NULL, NULL, NULL) == -1) {
    std::cerr << "Select error! " << std::endl;
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < num_players; i++) {
    if (FD_ISSET(all_client_connection_fds[i], &readfds)) {
      Potato return_potato =
          server_socket.receivePotatoFromClient(all_client_connection_fds[i]);
      std::cout << "Trace of potato:" << std::endl;
      std::cout << return_potato.getTrace() << std::endl;
    }
  }

  quitGame(server_socket,all_client_connection_fds,num_players);

  /*
  Potato updated_potato_object =
      server_socket.receivePotatoFromClient(client_connection_fd);
  std::cout << "Server received: " << updated_potato_object.getTrace() << std::endl;
 */

  return EXIT_SUCCESS;
}