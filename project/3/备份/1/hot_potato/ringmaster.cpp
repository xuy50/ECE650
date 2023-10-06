#include <algorithm>
#include <map>
#include <vector>

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
    size_t portNum = strtoul(port, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid Port number.\n";
        return EXIT_FAILURE;
    } else if (portNum > 65535) {
        std::cerr << "Please enter a valid Port number(from 0-65535).\n";
        return EXIT_FAILURE;
    }

    size_t numPlayers = strtoul(numPlayersStr, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid player number.\n";
        return EXIT_FAILURE;
    }
    // else if (numPlayers > 1) {
    //     std::cerr << "Please enter a valid player number(larger than 1).\n";
    //     return EXIT_FAILURE;
    // }

    size_t numHops = strtoul(numHopsStr, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid Hop number.\n";
        return EXIT_FAILURE;
    } else if (numHops > 512) {
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
    vector<size_t> playersID;
    for (size_t i = 0; i < numPlayers; ++i) {
        playersID.push_back(i);
    }
    srand((int)time(0));
    random_shuffle(playersID.begin(), playersID.end());

    // map<size_t, int> players;
    vector<int> playersPos(numPlayers, 0);

    for (size_t i = 0; i < numPlayers; ++i) {
        // cout << "Waiting for connection on port " << port << endl;
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int client_connection_fd;
        client_connection_fd = accept(
            socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (client_connection_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        }

        // Assign a random id to each player client
        // players[client_connection_fd] = playersID[i];
        playersPos[playersID[i]] = client_connection_fd;

        cout << "Player " << playersID[i] << " is ready to play" << endl;

        size_t id = playersID[i];
        send(client_connection_fd, &id, sizeof(id), 0);
        send(client_connection_fd, &numPlayers, sizeof(numPlayers), 0);
        // send(client_connection_fd, &numHops, sizeof(numHops), 0);

        // recv check
        // size_t idcheck;
        // recv(playersPos[playersID[i]], &idcheck, sizeof(idcheck), 0);
        // cout << "player " << id << " id check : " << idcheck << endl;
        // size_t npcheck;
        // recv(playersPos[playersID[i]], &npcheck, sizeof(npcheck), 0);
        // cout << "player " << id << " ps check : " << npcheck << endl;
    }

    potato *p = new potato(numHops);

    // first player
    int next = rand() % numPlayers;

    cout << "Ready to start the game, sending potato to player " << next
         << endl;

    // p->addTrace(next);

    for (int i = numHops; i >= 0; i--) {
        p->addTrace(next);
        send(playersPos[next], &i, sizeof(i), 0);
        if (i != 0) {
            recv(playersPos[next], &next, sizeof(next), 0);
        }
        // cout << "next: " << next << endl;
    }

    cout << "Trace of potato:\n" << p->traceToString() << endl;

    delete (p);
    freeaddrinfo(host_info_list);
    close(socket_fd);

    return EXIT_SUCCESS;
}
