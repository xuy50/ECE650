#include "potato.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Please in this form 'player <machine_name> <port_num>'\n";
        return EXIT_FAILURE;
    }

    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = argv[1];
    const char *port = argv[2];

    // check whether the input numbers are valid
    char *endptr;
    unsigned short portNum = strtoul(port, &endptr, 10);
    if (*endptr != '\0') {
        std::cerr << "Please enter a valid Port number.\n";
        return EXIT_FAILURE;
    } else if (portNum > 65535) {
        std::cerr << "Please enter a valid Port number(from 0-65535).\n";
        return EXIT_FAILURE;
    }

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_INET;
    host_info.ai_socktype = SOCK_STREAM;

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

    cout << "Connecting to " << hostname << " on port " << port << "..."
         << endl;

    status =
        connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    }  // if

    size_t id;
    recv(socket_fd, &id, sizeof(id), 0);
    size_t numPlayers;
    recv(socket_fd, &numPlayers, sizeof(numPlayers), 0);
    size_t numHops;
    // recv(socket_fd, &numHops, sizeof(numHops), 0);

    // recv test
    // cout << id << endl;
    // cout << numPlayers << endl;
    // cout << numHops << endl;

    // send(socket_fd, &id, sizeof(id), 0);
    // send(socket_fd, &numPlayers, sizeof(numPlayers), 0);

    // char finishMessage = 'f';
    // send(socket_fd, &finishMessage, sizeof(finishMessage), 0);

    // The id of the neighbors, 0 for left, 1 for right
    size_t lrid[2];
    if (id == (numPlayers - 1)) {
        lrid[0] = id - 1;
        lrid[1] = 0;
    } else if (id == 0) {
        lrid[0] = numPlayers - 1;
        lrid[1] = 1;
    } else {
        lrid[0] = id - 1;
        lrid[1] = id + 1;
    }

    // cout << "L: " << lrid[0] << ", R: " << lrid[1] << endl;

    srand((int)time(0));

    do {
        int recvCheck = recv(socket_fd, &numHops, sizeof(numHops), 0);
        // cout << recvCheck << endl; // recv check test
        if (recvCheck == 0 || recvCheck == -1) {
            break;
        } else if (numHops == 0) {
            cout << "I'm it" << endl;
        } else {
            int next = rand() % 2;
            send(socket_fd, &(lrid[next]), sizeof(lrid[next]), 0);
            cout << "Sending potato to " << lrid[next] << endl;
        }
    } while (numHops != 0);

    freeaddrinfo(host_info_list);
    close(socket_fd);

    return EXIT_SUCCESS;
}
