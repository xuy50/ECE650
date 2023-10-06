#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "potato.h"
#define BACKLOG 100

class Ringmaster {
   public:
    int numHops;
    int numPlayer;
    std::vector<int> player_fds;  // created for listening from players
    std::vector<std::string> player_ip;
    std::vector<int> player_port;
    const char *port;
    int socket_fd;

    /* Constructor */
    Ringmaster(char **argv) {
        port = argv[1];
        numPlayer = atoi(argv[2]);
        numHops = atoi(argv[3]);
    }
    /* Destructor */
    ~Ringmaster() {
        std::vector<int>::iterator it = player_fds.begin();
        while (it != player_fds.end()) {
            close(*it);
            it++;
        }
        close(socket_fd);
    }
    /* Build a server and listen to the specific port: socket bind listen */
    void buildServer() {
        // get host address info
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags = AI_PASSIVE;
        status = getaddrinfo(NULL, port, &host_info, &host_info_list);
        if (status != 0) {
            std::cout << "Error: cannot get address info for host" << std::endl;
            exit(1);
        }
        // set a socket descriptor
        socket_fd =
            socket(host_info_list->ai_family, host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
        if (socket_fd == -1) {
            std::cout << "Error: cannot create socket" << std::endl;
            exit(1);
        }
        int yes = 1;
        status =
            setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(socket_fd, host_info_list->ai_addr,
                      host_info_list->ai_addrlen);
        if (status == -1) {
            std::cout << "Error: cannot bind socket" << std::endl;
            perror("bind");
            exit(1);
        }
        // listen, accept and receive
        status = listen(socket_fd, BACKLOG);
        if (status == -1) {
            std::cout << "Error: cannot listen on socket" << std::endl;
            exit(1);
        }
        freeaddrinfo(host_info_list);
    }
    /* Accept one Player */
    int acceptPlayer() {
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int player_connection_fd;  // new fd
        // std::cout << "fd:" << socket_fd << std::endl;
        player_connection_fd = accept(
            socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (player_connection_fd == -1) {
            std::cout << "Error: cannot accept connection on socket"
                      << std::endl;
            perror("socket");
            exit(1);
        }
        // store ip address/port of players
        char ip_str[INET6_ADDRSTRLEN];
        struct sockaddr_in *addr = (struct sockaddr_in *)&socket_addr;
        inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN);
        // allocate a port for player randomly
        srand((unsigned int)time(NULL));
        // int p = rand() % 10000 + 10000;
        std::string s(ip_str, strlen(ip_str));
        player_ip.push_back(s);
        // player_port.push_back(p);
        for (int i = 0; i < (int)player_ip.size(); i++) {
            std::cout << "ip:" << player_ip[i] << std::endl;
        }
        // for (int i = 0; i < (int)player_port.size(); i++) {
        //     std::cout << "port:" << player_port[i] << std::endl;
        // }
        return player_connection_fd;
    }

    /* Connect with N players: accept send receive */
    void acceptAllPlayers() {
        for (int i = 0; i < numPlayer; i++) {
            int player_connection_fd = acceptPlayer();
            std::cout << "player " << i << ": " << player_connection_fd
                      << std::endl;
            player_fds.push_back(player_connection_fd);
            // send player index and total number of players info to players
            send(player_connection_fd, &i, sizeof(i), 0);
            send(player_connection_fd, &numPlayer, sizeof(numPlayer), 0);

            int playPort;
            recv(player_connection_fd, &playPort, sizeof(playPort), 0);
            player_port.push_back(playPort);
            std::cout << player_port[i] << std::endl;

            // send(player_connection_fd, &player_port[i],
            // sizeof(player_port[i]),
            //      0);
        }
    }

    void sendPlayerNeighbor() {
        // send neighbors ip+port to each player
        for (int i = 0; i < numPlayer - 1; i++) {
            char tmp[1024];
            memset(tmp, 0, sizeof(tmp));
            strcpy(tmp, player_ip[i + 1].c_str());
            send(player_fds[i], &tmp, sizeof(tmp), 0);
            send(player_fds[i], &player_port[i + 1], sizeof(player_port[i + 1]),
                 0);
        }
        char tmp1[1024];
        memset(tmp1, 0, sizeof(tmp1));
        strcpy(tmp1, player_ip[0].c_str());
        send(player_fds[numPlayer - 1], &tmp1, sizeof(tmp1), 0);
        send(player_fds[numPlayer - 1], &player_port[0], sizeof(player_port[0]),
             0);

        for (int i = 0; i < numPlayer; i++) {
            int connected = 0;
            recv(player_fds[i], &connected, sizeof(connected), MSG_WAITALL);
            if (connected != 1) {
                std::cerr << "player" << i << " connect fail" << std::endl;
            } else {
                std::cout << "Player " << i << " is ready to play" << std::endl;
            }
        }
    }
    /* Print initialization info */
    void printInitInfo() {
        std::cout << "Potato Ringmaster" << std::endl;
        std::cout << "Players = " << numPlayer << std::endl;
        std::cout << "Hops = " << numHops << std::endl;
    }
    /* Begin toss potato to a random player*/
    void tossPotato(int numHops, int numPlayers) {
        potato_t potato;
        memset(&potato, 0, sizeof(potato));
        potato.numHops = numHops;
        potato.leftHops = numHops;
        potato.numPlayers = numPlayers;
        potato.count = 0;
        srand((unsigned int)time(NULL));
        int random = rand() % potato.numPlayers;
        send(player_fds[random], &potato, sizeof(potato), 0);
        std::cout << "pc: " << potato.count << ", phn: " << potato.numHops
                  << ", plhn: " << potato.numHops
                  << ", ppn: " << potato.numPlayers << std::endl;
        std::cout << "Ready to start the game, sending potato to player "
                  << random << std::endl;
    }
    /* Receive potato and print trace info*/
    void tracePotato() {
        fd_set readfds;
        potato_t p;
        while (true) {
            FD_ZERO(&readfds);
            int n = INT_MIN;
            std::vector<int>::iterator it = player_fds.begin();
            while (it != player_fds.end()) {
                FD_SET(*it, &readfds);
                n = std::max(n, *it) + 1;
                it++;
            }
            it = player_fds.begin();
            bool breakCheck = false;
            int rv = select(n, &readfds, NULL, NULL, NULL);
            std::cout << "select rv: " << rv << std::endl;
            if (rv == -1) {
                std::cout << "Error: select failure" << std::endl;
                exit(1);
            } else if (rv == 0) {
                continue;
            } else if (rv > 0) {
                while (it != player_fds.end()) {
                    if (FD_ISSET(*it, &readfds)) {
                        // recv(*it, &p, sizeof(p), 0);
                        int rvp0 = recv(*it, &p, sizeof(p), 0);
                        std::cout << "finish rv: " << rvp0 << std::endl;
                        std::cout << "pc: " << p.count
                                  << ", plhn: " << p.leftHops
                                  << ", ppn: " << p.numPlayers << std::endl;
                        breakCheck = true;
                        break;
                    }
                    it++;
                }
                if (breakCheck) {
                    break;
                }
            }
        }
        // print the trace
        std::cout << "Trace of potato:" << std::endl;
        for (int i = 0; i < p.numHops; i++) {
            if (i == 0) {
                std::cout << p.trace[i];
            } else {
                std::cout << "," << p.trace[i];
            }
        }
        std::cout << std::endl;
    }
};

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr
            << "Error: wrong number of arguments(eg./ringmaster 12345 3 100)"
            << std::endl;
    }
    Ringmaster rm(argv);
    rm.printInitInfo();
    rm.buildServer();
    rm.acceptAllPlayers();
    rm.sendPlayerNeighbor();
    // begin toss potato games
    rm.tossPotato(rm.numHops, rm.numPlayer);
    rm.tracePotato();
}