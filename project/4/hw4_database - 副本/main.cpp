#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

void parsePlayer(connection *C, string line) {
    stringstream lineStream(line);
    int player_id;
    lineStream >> player_id;
    int team_id;
    lineStream >> team_id;
    int jersey_num;
    lineStream >> jersey_num;
    string first_name;
    lineStream >> first_name;
    string last_name;
    lineStream >> last_name;
    int mpg;
    lineStream >> mpg;
    int ppg;
    lineStream >> ppg;
    int rpg;
    lineStream >> rpg;
    int apg;
    lineStream >> apg;
    double spg;
    lineStream >> spg;
    double bpg;
    lineStream >> bpg;

    // output check line parse
    /*
    cout << line << endl;
    cout << player_id << " " << team_id << " " << jersey_num << " "
         << first_name << " " << last_name << " " << mpg << " " << ppg << " "
         << rpg << " " << apg << " " << spg << " " << bpg << " " << endl;
    */

    add_player(C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg,
               apg, spg, bpg);
}

void parseTeam(connection *C, string line) {
    stringstream lineStream(line);
    int team_id;
    lineStream >> team_id;
    string name;
    lineStream >> name;
    int state_id;
    lineStream >> state_id;
    int color_id;
    lineStream >> color_id;
    int wins;
    lineStream >> wins;
    int losses;
    lineStream >> losses;

    // output check line parse
    /*
    cout << line << endl;
    cout << team_id << " " << name << " " << state_id << " " << color_id << " "
         << wins << " " << losses << endl;
    */

    add_team(C, name, state_id, color_id, wins, losses);
}

void parseState(connection *C, string line) {
    stringstream lineStream(line);
    int state_id;
    lineStream >> state_id;
    string name;
    lineStream >> name;

    // output check line parse
    /*
    cout << line << endl;
    cout << state_id << " " << name << endl;
    */

    add_state(C, name);
}

void parseColor(connection *C, string line) {
    stringstream lineStream(line);
    int color_id;
    lineStream >> color_id;
    string name;
    lineStream >> name;

    // output check line parse
    /*
    cout << line << endl;
    cout << color_id << " " << name << endl;
    */

    add_color(C, name);
}

int main(int argc, char *argv[]) {
    // Allocate & initialize a Postgres connection object
    connection *C;

    try {
        // Establish a connection to the database
        // Parameters: database name, user name, user password
        C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
        // C = new connection("dbname=ACC_BBALL user=postgres password=3219");
        if (C->is_open()) {
            cout << "Opened database successfully: " << C->dbname() << endl;
        } else {
            cout << "Can't open database" << endl;
            return 1;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        return 1;
    }

    // TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL
    // database
    //       load each table with rows from the provided source txt files

    string dropPlayer = "DROP TABLE IF EXISTS PLAYER";
    string playerTable =
        "CREATE TABLE PLAYER ("
        "PLAYER_ID INT PRIMARY KEY     NOT NULL,"
        "TEAM_ID               INT     NOT NULL,"
        "UNIFORM_NUM           INT     NOT NULL,"
        "FIRST_NAME            TEXT    NOT NULL,"
        "LAST_NAME             TEXT    NOT NULL,"
        "MPG                   INT     NOT NULL,"
        "PPG                   INT     NOT NULL,"
        "RPG                   INT     NOT NULL,"
        "APG                   INT     NOT NULL,"
        "SPG                   REAL     NOT NULL,"
        "BPG                   REAL     NOT NULL );";

    string dropTeam = "DROP TABLE IF EXISTS TEAM";
    string teamTable =
        "CREATE TABLE TEAM ("
        "TEAM_ID INT PRIMARY KEY     NOT NULL,"
        "NAME                TEXT    NOT NULL,"
        "STATE_ID            INT     NOT NULL,"
        "COLOR_ID            INT     NOT NULL,"
        "WINS                INT     NOT NULL,"
        "LOSSES              INT     NOT NULL );";

    string dropState = "DROP TABLE IF EXISTS STATE";
    string stateTable =
        "CREATE TABLE STATE ("
        "STATE_ID INT PRIMARY KEY     NOT NULL,"
        "NAME                 TEXT    NOT NULL );";

    string dropColor = "DROP TABLE IF EXISTS COLOR";
    string colorTable =
        "CREATE TABLE COLOR ("
        "COLOR_ID INT PRIMARY KEY     NOT NULL,"
        "NAME                 TEXT    NOT NULL );";

    /* Create a transactional object. */
    work W(*C);

    /* Execute SQL query */
    // drop all exist table
    W.exec(dropPlayer);
    W.exec(dropTeam);
    W.exec(dropState);
    W.exec(dropColor);

    // initial all table
    W.exec(playerTable);
    W.exec(teamTable);
    W.exec(stateTable);
    W.exec(colorTable);

    W.commit();

    // read file and write informations into tables for database ACC_BBALL
    std::string line;

    // read player.txt
    std::ifstream filePlayer("player.txt");
    if (!filePlayer.is_open()) {
        std::cerr << "Unable to open player.txt." << std::endl;
    }
    while (getline(filePlayer, line)) {
        parsePlayer(C, line);
    }
    filePlayer.close();

    // read team.txt
    std::ifstream fileTeam("team.txt");
    if (!fileTeam.is_open()) {
        std::cerr << "Unable to open team.txt." << std::endl;
    }
    while (getline(fileTeam, line)) {
        // std::cout << line << std::endl;
        parseTeam(C, line);
    }
    fileTeam.close();

    // read state.txt
    std::ifstream fileState("state.txt");
    if (!fileState.is_open()) {
        std::cerr << "Unable to open state.txt." << std::endl;
    }
    while (getline(fileState, line)) {
        // std::cout << line << std::endl;
        parseState(C, line);
    }
    fileState.close();

    // read color.txt
    std::ifstream fileColor("color.txt");
    if (!fileColor.is_open()) {
        std::cerr << "Unable to open color.txt." << std::endl;
    }
    while (getline(fileColor, line)) {
        // std::cout << line << std::endl;
        parseColor(C, line);
    }
    fileColor.close();

    exercise(C);

    // Close database connection
    C->disconnect();

    return 0;
}
