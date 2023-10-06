#include <utility>
#include "hot_potato.hpp"

using namespace std;

int main(int argc, char *argv[]){
    int status;
    const char *port = argv[1];
    const int userNum = strtoull(argv[2], NULL, 10);
    int hops = strtoull(argv[3], NULL, 10);
    if(userNum <= 1 || hops < 0 || hops > 512){
        return EXIT_SUCCESS;
    }
    // vector<pair<int, pair<int, string> > > players;
    vector<pair<int, pair<int, string> > > players(userNum); 
    vector<int> randNum;

    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << userNum << endl;
    cout << "Hops = " << hops << endl;

    for(int i = 0; i < userNum; i++){
        randNum.push_back(i);
    }
    srand((unsigned int)time(NULL));
    random_shuffle(randNum.begin(), randNum.end());

    int serverSocket_fd = initServer(port);

    for(int i = 0; i < userNum; i++){
        int playerId = randNum[i];
        // cout << "player id is: " << playerId << endl;
        // cout << "list size is: " << players.size() << endl;
        vector<pair<int, pair<int, string> > >::iterator it = players.begin();
        struct sockaddr_in socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int client_connection_fd;

        client_connection_fd = accept(serverSocket_fd, (struct sockaddr*)&socket_addr, &socket_addr_len);
        
        if (client_connection_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        }

        string playerIp = string(inet_ntoa(socket_addr.sin_addr));
        
        cout << "Player " << playerId <<" is ready to play" << endl;
        
        char buffer[513];
        recv(client_connection_fd, buffer, sizeof(buffer), 0);
        
        // Send the initial info to the user
        send(client_connection_fd, &playerId, sizeof(playerId), 0);
        int totalPlayers = userNum;
        send(client_connection_fd, &totalPlayers, sizeof(totalPlayers), 0);

        int playerPort;
        recv(client_connection_fd, &playerPort, sizeof(playerPort), 0);
        players[playerId] = make_pair(client_connection_fd, make_pair(playerPort, playerIp));
        // cout << "The player's port is: " << players[i].second.first << " and the ip is: " << players[i].second.second << endl;
    } 
    // cout << "BEGIN FORMING LOOP" << endl;
    // Begin forming the loop
    for(int i = 0; i < userNum; i++){
        int target_player_fd = players[i].first; 
        const char* player_right_ip = (players[(i + 1) % userNum].second.second).c_str();
        // cout << "The ip is: " << player_right_ip << endl;
        int port = players[(i + 1) % userNum].second.first;
        send(target_player_fd, &port, sizeof(port), 0);
        char buffer2[512];
        recv(target_player_fd, buffer2, sizeof(buffer2), 0);
        // cout << "port is sent: " << port << endl;
        send(target_player_fd, player_right_ip, strlen(player_right_ip), 0);
        // cout << "ip is sent: " << player_right_ip << endl;
        recv(target_player_fd, buffer2, sizeof(buffer2), 0);
        // cout << "Client Recieved" << endl;
    }
    // Begin the game: send the potato to random player
    Potato potato(hops);
    srand((unsigned int)time(NULL) + userNum);
    int random = rand() % userNum;
    cout << "Ready to start the game, sending potato to player " << random << endl;
    send(players[random].first, &potato, sizeof(potato), 0);
    
    fd_set readfds;
    FD_ZERO(&readfds);
    int maxfdp = players[0].first;
    for(int i = 0; i < userNum; i++){
        FD_SET(players[i].first, &readfds);
        if(players[i].first > maxfdp){
            maxfdp = players[i].first;
        }
    }
    maxfdp++;
    select(maxfdp, &readfds, NULL, NULL, NULL);
    for(int i = 0; i < userNum; i++){
        if(FD_ISSET(players[i].first, &readfds)){
            recv(players[i].first, &potato, sizeof(potato), MSG_WAITALL);
            break;
        }
    }
    // cout << potato.total_hops << endl;
    // cout << potato.passNum << endl;
    // cout << potato.hops_left << endl;
    for(int i = 0; i < userNum; i++){
        send(players[i].first, &potato, sizeof(potato), MSG_WAITALL);
    }
    for(int i = 0; i < userNum; i++){
        close(players[i].first);
    }
    potato.printPath();

    close(serverSocket_fd);

    return 0;
}

