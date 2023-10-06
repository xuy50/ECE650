#include <algorithm>
#include <map>
#include <vector>

#include "potato.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    const char *numPlayersStr = argv[1];
    const char *numHopsStr = argv[2];

    char *endptr;
    size_t numPlayers = strtoul(numPlayersStr, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid player number.\n";
        return EXIT_FAILURE;
    }

    size_t numHops = strtoul(numHopsStr, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid Hop number.\n";
        return EXIT_FAILURE;
    } else if (numHops > 512) {
        std::cerr << "Please enter a valid Hop number(from 0-512).\n";
        return EXIT_FAILURE;
    }

    // cout << numPlayersStr << " : " << numPlayers << endl;
    // cout << numHopsStr << " : " << numHops << endl;

    vector<size_t> playersID;
    for (size_t i = 0; i < numPlayers; ++i) {
        playersID.push_back(i);
    }
    srand((int)time(0));
    random_shuffle(playersID.begin(), playersID.end());

    map<size_t, int> players;

    for (size_t i = 0; i < numPlayers; ++i) {
        cout << playersID[i];
        players[playersID[i]] = i;
        if (i != numPlayers - 1)
            cout << ", ";
        else
            cout << endl;
    }
    for (size_t i = 0; i < numPlayers; ++i) {
        cout << "id: " << i << " = " << players[i] << endl;
    }

    // potato *p1 = new potato(numHops);

    // p1->addTrace(2);
    // p1->addTrace(1);
    // p1->addTrace(0);
    // p1->addTrace(1);

    // cout << p1->traceToString() << endl;

    // delete (p1);

    int sPortInt = 0;
    char sPort[6];
    snprintf(sPort, sizeof(sPort), "%d", sPortInt);

    {
        int status_p;
        int socket_fd_p;
        struct addrinfo host_info_p;
        struct addrinfo *host_info_list_p;
        const char *hostname_p = NULL;
        const char *port_p = sPort;

        memset(&host_info_p, 0, sizeof(host_info_p));

        host_info_p.ai_family = AF_UNSPEC;
        host_info_p.ai_socktype = SOCK_STREAM;
        host_info_p.ai_flags = AI_PASSIVE;

        status_p = getaddrinfo(hostname_p, port_p, &host_info_p, &host_info_list_p);
        if (status_p != 0) {
            cerr << "Error: cannot get address info for host" << endl;
            cerr << "  (" << hostname_p << "," << port_p << ")" << endl;
            return -1;
        }  // if

        socket_fd_p =
            socket(host_info_list_p->ai_family, host_info_list_p->ai_socktype,
                   host_info_list_p->ai_protocol);
        if (socket_fd_p == -1) {
            cerr << "Error: cannot create socket" << endl;
            cerr << "  (" << hostname_p << "," << port_p << ")" << endl;
            return -1;
        }  // if

        int yes = 1;
        status_p = setsockopt(socket_fd_p, SOL_SOCKET, SO_REUSEADDR, &yes,
                              sizeof(int));
        status_p = bind(socket_fd_p, host_info_list_p->ai_addr,
                        host_info_list_p->ai_addrlen);
        if (status_p == -1) {
            cerr << "Error: cannot bind socket" << endl;
            cerr << "  (" << hostname_p << "," << port_p << ")" << endl;
            return -1;
        }  // if

        status_p = listen(socket_fd_p, 100);
        if (status_p == -1) {
            cerr << "Error: cannot listen on socket" << endl;
            cerr << "  (" << hostname_p << "," << port_p << ")" << endl;
            return -1;
        }  // if
    }

    return EXIT_SUCCESS;
}
