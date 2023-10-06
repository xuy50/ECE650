#include "potato.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Please in this form 'player <machine_name> <port_num>'\n";
        return EXIT_FAILURE;
    }

    int status_master;
    int socket_master;
    struct addrinfo host_info_master;
    struct addrinfo *host_info_list_master;
    const char *hostname_master = argv[1];
    const char *port_master = argv[2];

    // set client
    {
        // check whether the input numbers are valid
        char *endptr;
        int portNumM = strtoul(port_master, &endptr, 10);
        if (*endptr != '\0') {
            std::cerr << "Please enter a valid Port number.\n";
            return EXIT_FAILURE;
        } else if (portNumM > 65535) {
            std::cerr << "Please enter a valid Port number(from 0-65535).\n";
            return EXIT_FAILURE;
        }

        memset(&host_info_master, 0, sizeof(host_info_master));
        host_info_master.ai_family = AF_UNSPEC;
        host_info_master.ai_socktype = SOCK_STREAM;

        status_master = getaddrinfo(hostname_master, port_master,
                                    &host_info_master, &host_info_list_master);
        if (status_master != 0) {
            cerr << "Error: cannot get address info for host" << endl;
            cerr << "  (" << hostname_master << "," << port_master << ")"
                 << endl;
            return -1;
        }  // if

        socket_master = socket(host_info_list_master->ai_family,
                               host_info_list_master->ai_socktype,
                               host_info_list_master->ai_protocol);
        if (socket_master == -1) {
            cerr << "Error: cannot create socket" << endl;
            cerr << "  (" << hostname_master << "," << port_master << ")"
                 << endl;
            return -1;
        }  // if

        status_master = connect(socket_master, host_info_list_master->ai_addr,
                                host_info_list_master->ai_addrlen);
        if (status_master == -1) {
            cerr << "Error: cannot connect to socket" << endl;
            cerr << "  (" << hostname_master << "," << port_master << ")"
                 << endl;
            return -1;
        }  // if
    }
    // master cliend set end

    // get player ID and players' number
    int id;
    recv(socket_master, &id, sizeof(id), MSG_WAITALL);
    // int sPort;
    // recv(socket_master, &sPort, sizeof(sPort), 0);
    int numPlayers;
    recv(socket_master, &numPlayers, sizeof(numPlayers), MSG_WAITALL);

    // set as server
    int status_s;
    int socket_s;
    struct addrinfo host_info_s;
    struct addrinfo *host_info_list_s;
    const char *hostname_s = NULL;
    const char *port_s = "0";
    // cout << "poty_s: " << port_s << endl;

    memset(&host_info_s, 0, sizeof(host_info_s));

    host_info_s.ai_family = AF_UNSPEC;
    host_info_s.ai_socktype = SOCK_STREAM;
    host_info_s.ai_flags = AI_PASSIVE;

    status_s = getaddrinfo(hostname_s, port_s, &host_info_s, &host_info_list_s);
    if (status_s != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname_s << "," << port_s << ")" << endl;
        return -1;
    }  // if

    socket_s =
        socket(host_info_list_s->ai_family, host_info_list_s->ai_socktype,
               host_info_list_s->ai_protocol);
    if (socket_s == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname_s << "," << port_s << ")" << endl;
        return -1;
    }  // if

    int yes = 1;
    status_s =
        setsockopt(socket_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status_s =
        bind(socket_s, host_info_list_s->ai_addr, host_info_list_s->ai_addrlen);
    if (status_s == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname_s << "," << port_s << ")" << endl;
        return -1;
    }  // if

    status_s = listen(socket_s, 100);
    if (status_s == -1) {
        cerr << "Error: cannot listen on socket" << endl;
        cerr << "  (" << hostname_s << "," << port_s << ")" << endl;
        return -1;
    }  // if

    // get self port
    struct sockaddr_in psi;
    socklen_t len = sizeof(psi);
    if (getsockname(socket_s, (struct sockaddr *)&psi, &len) == -1) {
        cerr << "Error: cannot getsockname" << endl;
        exit(EXIT_FAILURE);
    }
    int port = ntohs(psi.sin_port);
    // cout << "port: " << port << endl;
    send(socket_master, &port, sizeof(port), 0);

    // set server end

    cout << "Connected as player " << id << " out of " << numPlayers
         << " total players" << endl;

    // The id of the neighbors, 0 for left, 1 for right
    int lrid[2];
    // cout << "Lid: " << lrid[0] << ", Rid: " << lrid[1] << endl;
    recv(socket_master, &lrid[0], sizeof(lrid[0]), MSG_WAITALL);
    recv(socket_master, &lrid[1], sizeof(lrid[1]), MSG_WAITALL);

    int lrport[2];
    // cout << "Lport: " << lrport[0] << ", Rport: " << lrport[1] << endl;
    recv(socket_master, &lrport[0], sizeof(lrport[0]), MSG_WAITALL);
    recv(socket_master, &lrport[1], sizeof(lrport[1]), MSG_WAITALL);

    ostringstream oslp;
    oslp << lrport[0];
    string lportStr = oslp.str();
    ostringstream osrp;
    oslp << lrport[1];
    string rportStr = osrp.str();
    const char *lp = &lportStr[0];
    // const char* rp = &rportStr[0];

    // cout << "lp: " << lrport[0] << ", rp: " << lrport[1] << endl;

    int ipLen;
    recv(socket_master, &ipLen, sizeof(ipLen), MSG_WAITALL);
    // cout << "lipLen: " << ipLen << endl;
    char lipStr[512];
    recv(socket_master, lipStr, ipLen, MSG_WAITALL);
    lipStr[ipLen] = 0;

    char sflag[512];
    recv(socket_master, sflag, 5, MSG_WAITALL);
    // int r = recv(socket_master, sflag, 5, 0);
    // cout << "frecv: " << r << endl;
    sflag[5] = 0;
    bool firstCheck = false;
    if (sflag[0] == 'f') {
        firstCheck = true;
    }

    // cout << "lip: " << lipStr << endl;

    // set client for left
    int status_l;
    int socket_l;
    struct addrinfo host_info_l;
    struct addrinfo *host_info_list_l;
    const char *hostname_l = lipStr;
    const char *port_l = lp;

    // port ip test
    // cout << "port_l: " << port_l << endl;
    // cout << "lip: " << hostname_l << endl;

    // set client

    memset(&host_info_l, 0, sizeof(host_info_l));
    host_info_l.ai_family = AF_UNSPEC;
    host_info_l.ai_socktype = SOCK_STREAM;

    status_l = getaddrinfo(hostname_l, port_l, &host_info_l, &host_info_list_l);
    if (status_l != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname_l << "," << port_l << ")" << endl;
        return -1;
    }  // if

    socket_l =
        socket(host_info_list_l->ai_family, host_info_list_l->ai_socktype,
               host_info_list_l->ai_protocol);
    if (socket_l == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname_l << "," << port_l << ")" << endl;
        return -1;
    }  // if

    // if (id != 0) {
    status_l = connect(socket_l, host_info_list_l->ai_addr,
                       host_info_list_l->ai_addrlen);
    // cout << "status_l: " << status_l << endl;
    if (status_l == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname_l << "," << port_l << ")" << endl;
        return -1;
    }  // if
    // }else {

    // cliend right set end

    // connect with right player
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int socket_s_fd;
    // cout << "test1" << endl;  // pass test
    socket_s_fd =
        accept(socket_s, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (socket_s_fd == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    }
    int connectFinish = 1;
    send(socket_master, &connectFinish, sizeof(connectFinish), 0);

    // cout << "test2" << endl;  // pass test

    // test sflag and fistCheck bool
    // cout << "sflag: " << sflag << endl;
    // cout << "firstCheck: " << firstCheck << endl;

    // connect test
    // int checkID;
    // fd_set rls;
    // fd_set wrs;
    // do {
    //     FD_ZERO(&rls);
    //     FD_ZERO(&wrs);
    //     FD_SET(socket_s_fd, &wrs);
    //     FD_SET(socket_l, &rls);
    //     int pSele = select(1024, &rls, &wrs, NULL, NULL);
    //     if (pSele > 0) {
    //         if (FD_ISSET(socket_s_fd, &wrs)) {
    //             send(socket_s_fd, &id, sizeof(id), 0);
    //         } else if (FD_ISSET(socket_l, &rls)) {
    //             recv(socket_l, &checkID, sizeof(checkID), 0);
    //             cout << "id: " << id << endl;
    //             cout << "left id: " << checkID << endl;
    //             break;
    //         }
    //     } else if (pSele == -1) {
    //         cerr << "select error" << endl;
    //         return -1;
    //     } else if (pSele == 0) {
    //         cerr << "selcet timeout" << endl;
    //         return -1;
    //     }
    // } while (1);

    // wait potato
    ptt_t p;

    srand((int)time(0));

    char flag[512];

    bool printCheck = false;

    // size_t test = 0;
    fd_set readfds;
    do {
        if (firstCheck) {
            firstCheck = false;
            recv(socket_master, &p, sizeof(p), MSG_WAITALL);
            // cout << test << ": lh: " << p.leftHops << ", nh :" << p.numHops
            //      << endl;  // left hops and nh out put test
            p.trace[p.numHops - p.leftHops - 1] = id;
            if (p.leftHops != 0) {
                p.leftHops--;
                int next = rand() % 2;
                if (next == 0)
                    send(socket_l, &p, sizeof(p), 0);
                else
                    send(socket_s_fd, &p, sizeof(p), 0);

                cout << "Sending potato to " << lrid[next] << endl;
            } else {
                if (!printCheck) {
                    printCheck = true;
                    cout << "I'm it" << endl;
                }
                send(socket_master, &p, sizeof(p), 0);
            }
        } else {
            bool recvCheck = false;

            FD_ZERO(&readfds);
            FD_SET(socket_master, &readfds);
            FD_SET(socket_l, &readfds);
            FD_SET(socket_s_fd, &readfds);
            int pSele = select(1024, &readfds, NULL, NULL, NULL);
            // cout << "select test: " << pSele << endl;
            if (pSele > 0) {
                // test++;
                if (FD_ISSET(socket_master, &readfds)) {
                    int smrecv = recv(socket_master, flag, 7, MSG_WAITALL);
                    flag[6] = 0;
                    // cout << "flag: " << flag << endl;
                    if ((flag[0] == 'f' && flag[5] == 'h') || smrecv == 0) {
                        break;
                    }
                    // else if (flag[0] == 's') {
                    //     recv(socket_master, &p, sizeof(p), 0);
                    //     recvCheck = true;
                    //     cout << test << ": lh: " << p.leftHops
                    //          << ", nh :" << p.numHops << endl;  // test
                    // }
                } else if (FD_ISSET(socket_l, &readfds)) {
                    recv(socket_l, &p, sizeof(p), MSG_WAITALL);
                    recvCheck = true;
                } else if (FD_ISSET(socket_s_fd, &readfds)) {
                    recv(socket_s_fd, &p, sizeof(p), MSG_WAITALL);
                    recvCheck = true;
                }
                if (recvCheck) {
                    // cout << test << ": lh: " << p.leftHops
                    //      << ", nh :" << p.numHops
                    //      << endl;  // left hops and nh out put test
                    p.trace[p.numHops - p.leftHops - 1] = id;
                    if (p.leftHops != 0) {
                        p.leftHops--;
                        int next = rand() % 2;
                        if (next == 0)
                            send(socket_l, &p, sizeof(p), 0);
                        else
                            send(socket_s_fd, &p, sizeof(p), 0);

                        cout << "Sending potato to " << lrid[next] << endl;
                    } else {
                        if (!printCheck) {
                            printCheck = true;
                            cout << "I'm it" << endl;
                        }
                        send(socket_master, &p, sizeof(p), 0);
                    }
                }
            } else if (pSele == -1) {
                cerr << "select error" << endl;
                return -1;
            } else if (pSele == 0) {
                cerr << "selcet timeout" << endl;
                return -1;
            }
        }
    } while (1);

    freeaddrinfo(host_info_list_s);
    freeaddrinfo(host_info_list_l);
    freeaddrinfo(host_info_list_master);
    close(socket_s);
    close(socket_l);
    close(socket_master);

    return EXIT_SUCCESS;
}
