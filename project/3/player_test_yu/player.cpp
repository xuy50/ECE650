#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <utility>
#include <vector>

#include "hot_potato.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    int status;
    const char *hostname = argv[1];
    const char *port = argv[2];

    int socket_fd = initClient(hostname, port);

    const char *connect1 = "connected";
    send(socket_fd, &connect1, sizeof(connect1), 0);

    int myId;
    status = recv(socket_fd, &myId, sizeof(myId), 0);
    int totalPlayers;
    status = recv(socket_fd, &totalPlayers, sizeof(totalPlayers), 0);
    cout << "Connected as player " << myId << " out of " << totalPlayers
         << " total players" << endl;

    int serverSocket_fd = initServer("");
    struct sockaddr_in localaddr;
    socklen_t len = sizeof(localaddr);
    int local =
        getsockname(serverSocket_fd, (struct sockaddr *)&localaddr, &len);
    int myPort = ntohs(localaddr.sin_port);
    // cout << "my port: " << myPort << endl;
    send(socket_fd, &myPort, sizeof(myPort), MSG_WAITALL);

    // cout << "BEGIN CONNECT NEIGHBOR" << endl;

    int right_player_port;
    recv(socket_fd, &right_player_port, sizeof(right_player_port), 0);
    const char *port_right = to_string(right_player_port).c_str();
    // cout << "GET PORT!" << port_right << endl;

    const char *message1 = "got port!";
    send(socket_fd, message1, strlen(message1), 0);

    char ip_right[512];
    int size = recv(socket_fd, &ip_right, sizeof(ip_right), 0);
    ip_right[size] = 0;
    // cout << "GET IP!" << ip_right << endl;
    // cout << "The neighbor port is: " << port_right << " and the ip is: " <<
    // ip_right << endl;
    const char *message2 = "got ip!";
    send(socket_fd, message2, strlen(message2), 0);

    int clientSocket_fd = initClient(ip_right, port_right);
    // cout << "Client created!" << endl;

    struct sockaddr_in socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int left_player_fd = accept(
        serverSocket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    // cout << "ACCEPT" << endl;
    // cout << "Connect to my right" << endl;
    const char *message3 = "hi";
    send(clientSocket_fd, message3, strlen(message3), 0);

    char buffer[512];
    recv(left_player_fd, buffer, sizeof(buffer), 0);
    // cout << "Connect to my left" << endl;

    Potato potato;
    int rightId;
    int leftId;
    if (myId == 0) {
        rightId = 1;
        leftId = totalPlayers - 1;
    } else if (myId == totalPlayers - 1) {
        rightId = 0;
        leftId = myId - 1;
    } else {
        rightId = myId + 1;
        leftId = myId - 1;
    }

    srand((unsigned int)time(NULL) + totalPlayers);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(socket_fd, &readfds);
        FD_SET(clientSocket_fd, &readfds);
        FD_SET(left_player_fd, &readfds);
        int maxfdp = max(max(socket_fd, clientSocket_fd), left_player_fd) + 1;
        int label = select(maxfdp, &readfds, NULL, NULL, NULL);
        int rightrecv = 1;
        int leftrecv = 1;
        if (label > 0) {
            if (FD_ISSET(socket_fd, &readfds)) {
                recv(socket_fd, &potato, sizeof(potato), MSG_WAITALL);
                if (potato.total_hops != 0 && potato.hops_left <= 0) {
                    break;
                } else if (potato.total_hops == 0) {
                    // if (potato.passNum == 0) {
                    //     potato.passNum = 1;
                    //     send(socket_fd, &potato, sizeof(potato), MSG_WAITALL);
                    //     //break;
                    // } else {
                    //     break;
                    // }
                    break;
                }
            } else if (FD_ISSET(clientSocket_fd, &readfds)) {
                rightrecv = recv(clientSocket_fd, &potato, sizeof(potato), MSG_WAITALL);

            } else if (FD_ISSET(left_player_fd, &readfds)) {
                leftrecv = recv(left_player_fd, &potato, sizeof(potato), MSG_WAITALL);
            }
            if(rightrecv != 0 && leftrecv != 0 && potato.total_hops != 0){
                if (potato.hops_left == 0 && potato.passNum != 0) {
                    potato.passNum++;
                    // cout << "I'm it" << endl;
                    send(socket_fd, &potato, sizeof(potato), MSG_WAITALL);
                } else {
                    potato.path[potato.passNum] = myId;
                    potato.hops_left--;
                    if(potato.hops_left == 0){
                        cout << "I'm it" << endl;
                        send(socket_fd, &potato, sizeof(potato), MSG_WAITALL);
                    }else{
                        potato.passNum++;
                        int random = rand() % 2;
                        // cout << "Number of left potatos: " << potato.hops_left << endl;
                        if (random == 0) {
                            cout << "Sending potato to " << rightId << endl;
                            send(clientSocket_fd, &potato, sizeof(potato), MSG_WAITALL);
                        } else {
                            cout << "Sending potato to " << leftId << endl;
                            send(left_player_fd, &potato, sizeof(potato), MSG_WAITALL);
                        }
                    }
                }
            }
        } else {
            break;
        }
    }

    close(socket_fd);
    close(left_player_fd);
    close(clientSocket_fd);
    close(serverSocket_fd);
    return 0;
}
