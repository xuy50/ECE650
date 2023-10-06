#include "potato.h"


int main(int argc, char *argv[])
{
  if (argc != 4){
    printErrorMsg("Usage: programName <port_num> <num_players> <num_hops>.");
  }
  srand((unsigned int)time(NULL));
  int status = 0;
  char * ptr;
  int num_players = strtol(argv[2], &ptr, 10);
  if (num_players <= 0){
    printErrorMsg("The number of player must larget or equal than 2");
  }
  if (num_players == 1){
    return 0;
  }
  int num_hops = strtol(argv[3], &ptr, 10);
  if (num_hops >= 513 || num_hops < 0){
    printErrorMsg("The number of hops should be less or equal than 512 and larger or equal than 0");
  }
  int socket_fd = start_listen(argv[1]);
  struct potato_t Potato;
  memset(&Potato, 0, sizeof(Potato));
  Potato.num_hops = num_hops;
  Potato.total_hops = num_hops;
  std::cout << "Potato Ringmaster" << std::endl;
  std::cout << "Players = " << num_players << std::endl;
  std::cout << "Hops = " << num_hops << std::endl;
  char ipAddress[INET6_ADDRSTRLEN];
  std::vector<int> client_connection_fd_list;
  // vector<Player> client_connection_player_list;
  std::vector<player_info> client_connection_info_list;
  std::vector<unsigned int> client_connection_port_list;
  for (int i = 0; i < num_players; i ++ ){
    int client_connection_fd = tryAccept(socket_fd);
    client_connection_fd_list.push_back(client_connection_fd);
    int player_id_info[2] = {i, num_players}; // pointer decay
    size_t len = sizeof(player_id_info);
    // can not send data at a time
    
    send(client_connection_fd, player_id_info, len, 0); 
    struct player_info now_player;
    recv(client_connection_fd, &now_player, sizeof(now_player), MSG_WAITALL);
    client_connection_info_list.push_back(now_player);
  }

  for (int i = 0; i < num_players; i ++ ){
      // send prev player's info to the current player
      // TODO: can I send a class through socket
        struct player_info next_player = client_connection_info_list[(i + 1) % num_players];
        try_send_all(client_connection_fd_list[i], &next_player, sizeof(next_player), 0);
        std::cout << "Player " << i << " is ready to play" << std::endl;
  }

  int random = rand() % num_players;
  std::cout <<"Ready to start the game, sending potato to player " << random << std::endl;
  if (Potato.num_hops == 0){
      Potato.num_hops = -1;
      std::cout << "Trace of potato:" << std::endl;
      std::cout << std::endl;
      for (int i = 0; i < num_players; i ++ ){
          try_send_all(client_connection_fd_list[i], &Potato, sizeof(Potato), 0);
          close(client_connection_fd_list[i]);
      }
      close(socket_fd);
      return 0;
  } else {
    try_send_all(client_connection_fd_list[random], &Potato, sizeof(Potato), 0);
  }


  // ready to recv potato back
  memset(&Potato, 0, sizeof(Potato));
  fd_set readfds;
  struct timeval tv;
  tv.tv_sec = 1;
  FD_ZERO(&readfds);
  int n = client_connection_fd_list[0];
  for (int i = 0; i < num_players; i ++ ){
      n = std::max(n, client_connection_fd_list[i]);
      FD_SET(client_connection_fd_list[i], &readfds);
  }
  int rv = select(n + 1, &readfds, NULL, NULL, &tv);
  int ending_player;
  for (int i = 0; i < num_players; i ++ ){
    if (FD_ISSET(client_connection_fd_list[i], &readfds)){
        recv(client_connection_fd_list[i], &Potato, sizeof(Potato), MSG_WAITALL);
        ending_player = i;
        break;
    }
  }

  // print the trace
  assert(Potato.num_hops == 0);
  std::cout << "Trace of potato:" << std::endl;
  std::string delim;
  for (int i = 0; i < Potato.total_hops; i ++ ){
    std::cout << delim << Potato.traces[i];
    delim = ",";
  }
  std::cout << std::endl;

  // send ending msg to every player
  Potato.num_hops = -1;
  for (int i = 0; i < num_players; i ++ ){
    if (i != ending_player){
      try_send_all(client_connection_fd_list[i], &Potato, sizeof(Potato), 0);
      close(client_connection_fd_list[i]);
    }
  }
  close(socket_fd);
  return 0;
}
