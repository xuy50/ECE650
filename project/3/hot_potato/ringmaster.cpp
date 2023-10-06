#include <algorithm>

#include "potato.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Please in this form 'ringmaster <port_num> <num_players> "
                     "<num_hops>'\n";
        return EXIT_FAILURE;
    }

    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;
    const char *port = argv[1];

    const char *numPlayersStr = argv[2];
    const char *numHopsStr = argv[3];

    // check whether the input numbers are valid
    char *endptr;
    int portNum = strtol(port, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid Port number.\n";
        return EXIT_FAILURE;
    } else if (portNum > 65535 || portNum < 0) {
        std::cerr << "Please enter a valid Port number(0-65535).\n";
        return EXIT_FAILURE;
    }

    int numPlayers = strtol(numPlayersStr, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid player number.\n";
        return EXIT_FAILURE;
    } else if (numPlayers < 1 || numPlayers > 1023) {
        std::cerr << "Please enter a valid player number(1-1023).\n";
        return EXIT_FAILURE;
    } else if (numPlayers == 1) {
        return EXIT_SUCCESS;
    }


    int numHops = strtol(numHopsStr, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid Hop number.\n";
        return EXIT_FAILURE;
    } else if (numHops > 512 || numHops < 0) {
        std::cerr << "Please enter a valid Hop number(from 0-512).\n";
        return EXIT_FAILURE;
    }

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }  // if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }  // if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status =
        bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }  // if

    status = listen(socket_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }  // if

    // Initially output
    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << numPlayers << endl;
    cout << "Hops = " << numHops << endl;

    // Generate player random id
    vector<int> playersID;
    for (int i = 0; i < numPlayers; ++i) {
        playersID.push_back(i);
    }
    srand((int)time(0));
    random_shuffle(playersID.begin(), playersID.end());

    vector<int> playersPos(numPlayers, 0);
    vector<int> playersPort(numPlayers, 0);
    vector<string> playersIp(numPlayers, "");

    for (int i = 0; i < numPlayers; ++i) {
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int client_connection_fd;
        client_connection_fd = accept(
            socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);

        // cout << "pfdn: " << client_connection_fd << endl;

        if (client_connection_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        }

        struct sockaddr_in *psi =
            ((struct sockaddr_in *)(struct sockaddr *)&socket_addr);
        int port;
        // int port = psi->sin_port;
        // int port = ntohs(psi->sin_port);
        // cout << "port: " << port << endl;
        // cout << "port: " << port << ", hs: " << porths << endl;
        char *ip = inet_ntoa(psi->sin_addr);
        playersIp[playersID[i]] = ip;
        // send(client_connection_fd, ip, sizeof(*ip), 0);

        // Assign a random id to each player client
        playersPos[playersID[i]] = client_connection_fd;

        int id = playersID[i];
        send(client_connection_fd, &id, sizeof(id), 0);
        // send(client_connection_fd, &client_connection_fd,
        //      sizeof(client_connection_fd), 0);
        send(client_connection_fd, &numPlayers, sizeof(numPlayers), 0);

        recv(client_connection_fd, &port, sizeof(port), MSG_WAITALL);
        // cout << "play" << playersID[i] << " port: " << port << endl;
        playersPort[playersID[i]] = port;

        cout << "Player " << playersID[i] << " is ready to play" << endl;
    }

    // send the left and right neighbor for every player
    // random first player
    int next = rand() % numPlayers;
    for (int i = 0; i < numPlayers; ++i) {
        int lID;
        int rID;
        int lPort;
        int rPort;
        char *lIP;
        if (i == (numPlayers - 1)) {
            lID = i - 1;
            rID = 0;
        } else if (i == 0) {
            lID = numPlayers - 1;
            rID = 1;
        } else {
            lID = i - 1;
            rID = i + 1;
        }
        lPort = playersPort[lID];
        rPort = playersPort[rID];
        lIP = &(playersIp[lID])[0];
        int lIPLen = strlen(lIP);
        // cout << "lp: " << lPort << ", rp: " << rPort << ", lip: " << lIP
        //      << endl;
        // cout << "ip len: " << lIPLen << endl;
        send(playersPos[i], &lID, sizeof(lID), 0);
        send(playersPos[i], &rID, sizeof(rID), 0);
        send(playersPos[i], &lPort, sizeof(lPort), 0);
        send(playersPos[i], &rPort, sizeof(rPort), 0);
        send(playersPos[i], &lIPLen, sizeof(lIPLen), 0);
        send(playersPos[i], lIP, lIPLen, 0);
        // cout << "lIP: " << lIP << " : " << lIPLen << endl;

        string sflagStr;
        if (i == next && numHops != 0) {
            sflagStr = "first";
        } else {
            sflagStr = "start";
        }
        const char *sflag = &sflagStr[0];
        // cout << "sflag: " << sflag << " : " << strlen(sflag) << endl;
        send(playersPos[i], sflag, strlen(sflag), 0);
        // int s = send(playersPos[i], sflag, strlen(sflag), 0);
        // cout << "send: " << s << endl;
    }

    // check all player are ready
    vector<int> playReady(numPlayers, 0);
    bool prcheck;
    if (numHops == 0) {
        prcheck = true;
    } else {
        prcheck = false;
    }
    fd_set readRead;
    while (!prcheck) {
        FD_ZERO(&readRead);
        for (int i = 0; i < numPlayers; i++) {
            FD_SET(playersPos[i], &readRead);
        }
        int pSele = select(1024, &readRead, NULL, NULL, NULL);
        if (pSele > 0) {
            for (int i = 0; i < numPlayers; i++) {
                if (FD_ISSET(playersPos[i], &readRead)) {
                    recv(playersPos[i], &playReady[i], sizeof(playReady[i]),
                         MSG_WAITALL);
                }
            }
            // int fp = readfds.fd_array[0];
        } else if (pSele == -1) {
            cerr << "select error" << endl;
            return -1;
        } else if (pSele == 0) {
            cerr << "selcet timeout" << endl;
            return -1;
        }

        prcheck = true;
        for (int i = 0; i < numPlayers; i++) {
            if (playReady[i] == 0) {
                prcheck = false;
                break;
            }
        }
    }

    // cout << "all ready test" << endl;

    ptt_t p;
    p.numHops = numHops;
    p.leftHops = numHops - 1;

    // start
    cout << "Ready to start the game, sending potato to player " << next
         << endl;

    // send the potato to the first player
    if (numHops != 0) {
        send(playersPos[next], &p, sizeof(p), 0);
    }

    // // get each step next player to check who is the final player
    // for (int i = numHops; i > 0; --i) {
    //     recv(playersPos[next], &next, sizeof(next), 0);
    // }
    // // get the potato infor from the final player
    // recv(playersPos[next], &p, sizeof(p), 0);

    // use select to check whether any player send the final step for potato
    if (numHops) {
        fd_set readfds;
        do {
            FD_ZERO(&readfds);
            for (int i = 0; i < numPlayers; i++) {
                FD_SET(playersPos[i], &readfds);
            }
            int pSele = select(1024, &readfds, NULL, NULL, NULL);
            if (pSele > 0) {
                for (int i = 0; i < numPlayers; i++) {
                    if (FD_ISSET(playersPos[i], &readfds)) {
                        recv(playersPos[i], &p, sizeof(p), MSG_WAITALL);
                        break;
                    }
                }
                // int fp = readfds.fd_array[0];
            } else if (pSele == -1) {
                cerr << "select error" << endl;
                return -1;
            } else if (pSele == 0) {
                cerr << "selcet timeout" << endl;
                return -1;
            }
        } while (p.leftHops != 0);
    }

    cout << "Trace of potato:\n" << traceToString(numHops, p.trace) << endl;

    // send the finish message to every player
    const char *fflag = "finish";
    for (int i = 0; i < numPlayers; i++) {
        send(playersPos[i], fflag, strlen(fflag), 0);
    }

    freeaddrinfo(host_info_list);
    close(socket_fd);

    return EXIT_SUCCESS;
}
