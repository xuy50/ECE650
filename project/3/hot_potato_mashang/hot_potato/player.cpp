#include <assert.h>
#include <limits.h>
#include <memory.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "potato.h"
#define BACKLOG 100

class Player {
   public:
    int id;
    int numPlayers;
    const char *rm_hostname;  // ringmaster's ip
    const char *rm_port;
    int neighbor_port;
    char neighbor_ip[1024];
    int port;  //  player's port allocated by ringmaster
    int socket_fd;
    int socket_fd_connect_master;
    int socket_fd_connect_player;
    int socket_fd_new;

    /* Constructor */
    Player(char **argv) : id(0), numPlayers(0) {
        rm_hostname = argv[1];
        rm_port = argv[2];
    }
    ~Player() {
        close(socket_fd);
        close(socket_fd_connect_master);
        close(socket_fd_connect_player);
    }

    /* connect to ringmaster and reply ready info*/
    void connectRingmaster() {
        // connect ringmaster (rm_hostname, ringmasterPort);
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        status = getaddrinfo(rm_hostname, rm_port, &host_info, &host_info_list);
        if (status != 0) {
            std::cout << "Error: cannot get address info for host" << std::endl;
            exit(1);
        }
        socket_fd_connect_master =
            socket(host_info_list->ai_family, host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
        if (socket_fd_connect_master == -1) {
            std::cout << "Error: cannot create socket" << std::endl;
            exit(1);
        }
        // std::cout << "socket_fd:" << socket_fd_connect_master << std::endl;
        // std::cout << "host_info_list->ai_addr" << host_info_list->ai_addr <<
        // std::endl;
        status = connect(socket_fd_connect_master, host_info_list->ai_addr,
                         host_info_list->ai_addrlen);
        if (status == -1) {
            std::cout << "Error: cannot connect to ringmaster" << std::endl;
            perror("connect");
            exit(1);
        }
        freeaddrinfo(host_info_list);

        // receive info
        recv(socket_fd_connect_master, &id, sizeof(id), 0);
        recv(socket_fd_connect_master, &numPlayers, sizeof(numPlayers), 0);
        // recv(socket_fd_connect_master, &port, sizeof(port), 0);
        buildServer();
        std::cout << "Connected as player " << id << " out of " << numPlayers
                  << " total players" << std::endl;
        // receive neighbors info from ringmaster
        recv(socket_fd_connect_master, neighbor_ip, sizeof(neighbor_ip), 0);
        recv(socket_fd_connect_master, &neighbor_port, sizeof(neighbor_port),
             0);
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
        std::string portstr = "0";
        status =
            getaddrinfo(NULL, portstr.c_str(), &host_info, &host_info_list);
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

        struct sockaddr_in psi;
        socklen_t len = sizeof(psi);
        if (getsockname(socket_fd, (struct sockaddr *)&psi, &len) == -1) {
            std::cerr << "Error: cannot getsockname" << std::endl;
            exit(EXIT_FAILURE);
        }
        port = ntohs(psi.sin_port);
        // cout << "port: " << port << endl;
        send(socket_fd_connect_master, &port, sizeof(port), 0);

        // connectNeighborPlayer();
        // std::cout << "Waiting for connection on port " << port << std::endl;
        // struct sockaddr_storage socket_addr;
        // socklen_t socket_addr_len = sizeof(socket_addr);
        // socket_fd_new = accept(socket_fd, (struct sockaddr *)&socket_addr,
        //                        &socket_addr_len);
        // if (socket_fd_new == -1) {
        //     std::cout << "Error: cannot accept connection on socket"
        //               << std::endl;
        //     exit(1);
        // }
        // int connected = 1;
        // send(socket_fd_connect_master, &connected, sizeof(connected),
        // MSG_WAITALL);
        freeaddrinfo(host_info_list);
    }

    /* Connect neighbor players */
    void connectNeighborPlayer() {
        // connect neighbor player
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;

        const char *neighip = neighbor_ip;
        std::string portstr = std::to_string(neighbor_port);
        const char *neighport = portstr.c_str();
        std::cout << "n ip: " << neighip << ", n port: " << portstr
                  << std::endl;
        status = getaddrinfo(neighip, neighport, &host_info, &host_info_list);
        if (status != 0) {
            std::cout << "Error: cannot get address info for host" << std::endl;
            exit(1);
        }
        socket_fd_connect_player =
            socket(host_info_list->ai_family, host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
        if (socket_fd_connect_player == -1) {
            std::cout << "Error: cannot create socket" << std::endl;
            exit(1);
        }
        status = connect(socket_fd_connect_player, host_info_list->ai_addr,
                         host_info_list->ai_addrlen);
        if (status == -1) {
            std::cout << "Error: cannot connect to neighbor" << std::endl;
            perror("connect");
            exit(1);
        }

        std::cout << "Waiting for connection on port " << port << std::endl;
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        socket_fd_new = accept(socket_fd, (struct sockaddr *)&socket_addr,
                               &socket_addr_len);
        if (socket_fd_new == -1) {
            std::cout << "Error: cannot accept connection on socket"
                      << std::endl;
            exit(1);
        }
        int connected = 1;
        send(socket_fd_connect_master, &connected, sizeof(connected),
             MSG_WAITALL);

        freeaddrinfo(host_info_list);
    }
    /* connect with ringmaster and other players; record and toss again when
     * receive potato*/
    void processPotato(potato_t p) {
        fd_set readfds;
        srand((unsigned int)time(NULL));
        std::vector<int> fds;
        fds.push_back(socket_fd_connect_master);
        fds.push_back(socket_fd_new);
        fds.push_back(socket_fd_connect_player);
        // receive potato until numHop = 0
        while (true) {
            FD_ZERO(&readfds);
            FD_SET(socket_fd_connect_master, &readfds);
            FD_SET(socket_fd_new, &readfds);
            FD_SET(socket_fd_connect_player, &readfds);
            int n = std::max(std::max(socket_fd_new, socket_fd_connect_master),
                             socket_fd_connect_player) +
                    1;
            int rv = select(n, &readfds, NULL, NULL, NULL);
            if (rv == -1) {
                std::cout << "Error: select failure" << std::endl;
                exit(1);
            } else if (rv == 0) {
                std::cout << "Error: Timeout occurred!" << std::endl;
                exit(1);
            } else if (rv > 0) {
                int rv1;
                bool whileStop = false;
                int from = -1;
                for (int i = 0; i < 3; i++) {
                    from++;
                    if (FD_ISSET(fds[i], &readfds)) {
                        rv1 = recv(fds[i], &p, sizeof(p), 0);
                        if (rv1 != sizeof(p)) {
                            std::cout << "Receive broken potato, sizeof(p):"
                                      << sizeof(p) << ", recv byte:" << rv1
                                      << std::endl;
                        }
                        std::cout << "received potato's numHops :" << p.leftHops
                                  << std::endl;
                        if (rv1 <= 0) {  // fd close
                            whileStop = true;
                            break;
                        }
                        break;
                    }
                }
                std::cout << "recv from: " << from << std::endl;
                if (whileStop) {
                    break;
                }
                if (rv1 == sizeof(p)) {
                    p.leftHops--;
                    std::cout << "potato in :" << id << std::endl;
                    p.trace[p.count] = id;
                    p.count++;
                    if (p.leftHops == 0) {  // send to ringmaster
                        send(socket_fd_connect_master, &p, sizeof(p), 0);
                        std::cout << "I'm it" << std::endl;
                        return;
                    } else if (p.leftHops == -1) {
                        return;
                    } else {  // send to a random neighbor
                        int random = rand() % 2;
                        int sendPByte;
                        if (random == 0) {
                            sendPByte = send(socket_fd_new, &p, sizeof(p), 0);
                            std::cout << "send p l byte: " << sendPByte
                                      << std::endl;
                            std::cout << "Sending potato to "
                                      << (id - 1 + numPlayers) % numPlayers
                                      << std::endl;
                        } else {
                            sendPByte = send(socket_fd_connect_player, &p,
                                             sizeof(p), 0);
                            std::cout << "send p r byte: " << sendPByte
                                      << std::endl;
                            std::cout << "Sending potato to "
                                      << (id + 1) % numPlayers << std::endl;
                        }
                    }
                }
            }
        }
    }
};
int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Error: wrong number of arguments(eg./player "
                     "vcm-30747.vm.duke.edu 12345)"
                  << std::endl;
    }
    Player player(argv);
    player.connectRingmaster();
    // std::cout << "player->neighbor_ip:" << player.neighbor_ip << std::endl;
    std::cout << "player->neighbor_port:" << player.neighbor_port << std::endl;
    std::cout << "player->port:" << player.port << std::endl;
    // build server and accept a neighbor player
    // player.buildServer();
    std::cout << "connects player successfully" << std::endl;
    player.connectNeighborPlayer();

    // const char *message1 = "to right";
    // const char *message2 = "to left";
    // send(player.socket_fd_new, message1, strlen(message1), 0);
    // send(player.socket_fd_connect_player, message2, strlen(message2), 0);
    // char buffer[512];
    // char buffer2[512];
    // recv(player.socket_fd_new, buffer, 12, 0);
    // recv(player.socket_fd_connect_player, buffer2, 12, 0);
    // buffer[12] = 0;
    // std::cout << "received from socket_fd_new: " << buffer << std::endl;
    // std::cout << "received from socket_fd_connect_player: " << buffer2 <<
    // std::endl; receive potato and send again

    potato_t p;
    player.processPotato(p);
}
