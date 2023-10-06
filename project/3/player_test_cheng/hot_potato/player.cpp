#include "potato.h"

int main(int argc, char *argv[])
{
  if (argc != 3){
    printErrorMsg("Usage: programName <host_name> <port_num>.\n");
  }
  // connect to server
  int ringmaster_socket_fd = connect_to_server(argv[1], argv[2]);
  // receive player id and total number 
  int player_id_info[2]; 
  recv(ringmaster_socket_fd, player_id_info, sizeof(player_id_info), MSG_WAITALL);
  srand((unsigned int)time(NULL) + player_id_info[0]);
  int client_listen_socket_fd = start_listen(""); // random select a port to listen
  std::string ownIpaddress = getIpaddress(ringmaster_socket_fd, "Own");
  unsigned int ownPort = getPort(ringmaster_socket_fd, "Own");
  struct player_info own_info;
  strcpy(own_info.ipAddress, ownIpaddress.c_str());
  own_info.port_num = ownPort;
  try_send_all(ringmaster_socket_fd, &own_info, sizeof(own_info), 0);

  struct player_info next_player;
  recv(ringmaster_socket_fd, &next_player, sizeof(next_player), MSG_WAITALL);
  char port_num_input[10];
  sprintf(port_num_input, "%u", next_player.port_num);
  // connect to next player
  int next_player_fd = connect_to_server(next_player.ipAddress, port_num_input);
  int listen_socket_fd = tryAccept(client_listen_socket_fd);
  std::cout << "Connected as player " << player_id_info[0] << " out of " << player_id_info[1] << " total players"<< std::endl;
  


  fd_set readfds;
  struct timeval tv;
  tv.tv_sec = 1;
  bool end_flag = false;
  struct potato_t Potato;
  int temp_n = std::max(listen_socket_fd, next_player_fd);
  int n = std::max(ringmaster_socket_fd, temp_n) + 1;
  int socket_fd_group[3] = {listen_socket_fd, next_player_fd, ringmaster_socket_fd};
  while(!end_flag){
    FD_ZERO(&readfds);
    FD_SET(listen_socket_fd, &readfds);
    FD_SET(next_player_fd, &readfds);
    FD_SET(ringmaster_socket_fd, &readfds);
    int rv = select(n, &readfds, NULL, NULL, &tv);
    if (rv == -1)
    {
        perror("select"); // error occurred in select()
        break;
    }
    else{
          if (FD_ISSET(ringmaster_socket_fd, &readfds))
          {
              // recv from ringmaster
              int status = recv(ringmaster_socket_fd, &Potato, sizeof(Potato), 0);
              if (status <= 0){
                continue;
              }
              if (Potato.num_hops == -1){
                // ending situation
                end_flag = true;
                break;
              }
              int index = Potato.total_hops - Potato.num_hops;
              Potato.traces[index] = player_id_info[0];
              Potato.num_hops -= 1;
              if (Potato.num_hops == 0){
                  end_flag = true;
                  std::cout << "I'm it" << std::endl;
                  try_send_all(ringmaster_socket_fd, &Potato, sizeof(Potato), 0);
                  break;
              } else {
                // send to another player 
                int random = rand() % 2;
                std::cout << "Sending potato to " << getPlayerNum(player_id_info[0], player_id_info[1], random) << std::endl;
                try_send_all(socket_fd_group[random], &Potato, sizeof(Potato), 0);
              }
          }         
          if (FD_ISSET(listen_socket_fd, &readfds))
          {
              // recv from prev player
              int status = recv(listen_socket_fd, &Potato, sizeof(Potato), 0);
              if (status <= 0){
                // recv failed
                continue;
              }
              int index = Potato.total_hops - Potato.num_hops;
              Potato.traces[index] = player_id_info[0];
              Potato.num_hops -= 1;
              if (Potato.num_hops == 0){
                  end_flag = true;
                  std::cout << "I'm it" << std::endl;
                  try_send_all(ringmaster_socket_fd, &Potato, sizeof(Potato), 0);
                  break;
              } else {
                // send to another player 
                int random = rand() % 2;
                std::cout << "Sending potato to " << getPlayerNum(player_id_info[0], player_id_info[1], random) << std::endl;;
                try_send_all(socket_fd_group[random], &Potato, sizeof(Potato), 0);
              }
          }
          if (FD_ISSET(next_player_fd, &readfds))
          {
              // recv from next player
              int status = recv(next_player_fd, &Potato, sizeof(Potato), 0);
              if (status <= 0){
                // recv failed
                continue;
              }
              int index = Potato.total_hops - Potato.num_hops;
              Potato.traces[index] = player_id_info[0];
              Potato.num_hops -= 1;
              if (Potato.num_hops == 0){
                  end_flag = true;
                  std::cout << "I'm it" << std::endl;
                  try_send_all(ringmaster_socket_fd, &Potato, sizeof(Potato), MSG_WAITALL);
                  break;
              } else {
                // send to another player 
                int random = rand() % 2;
                std::cout << "Sending potato to " << getPlayerNum(player_id_info[0], player_id_info[1], random) << std::endl;;
                try_send_all(socket_fd_group[random], &Potato, sizeof(Potato), 0);
                // if (Potato.num_hops == 1){
                //   end_flag = true;
                //   break;
                // }
              }
          }
    }
  }
  
  close(listen_socket_fd);
  close(next_player_fd);
  close(client_listen_socket_fd);
  close(ringmaster_socket_fd);
  return 0;
}
