#include "query_funcs.h"

#include <iomanip>

int playerId = 1;
int teamId = 1;
int stateId = 1;
int colorId = 1;

string escapeSingleQuotes(const string &input) {
    std::string escaped;
    for (char c : input) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

void add_player(connection *C, int team_id, int jersey_num, string first_name,
                string last_name, int mpg, int ppg, int rpg, int apg,
                double spg, double bpg) {
    // escape the single quotes for all string
    first_name = escapeSingleQuotes(first_name);
    last_name = escapeSingleQuotes(last_name);

    // output check line parse
    // cout << playerId << " " << team_id << " " << jersey_num << " "
    //      << first_name << " " << last_name << " " << mpg << " " << ppg << " "
    //      << rpg << " " << apg << " " << spg << " " << bpg << " " << endl;

    /* Create a transactional object. */
    work W(*C);

    string insert =
        "INSERT INTO PLAYER (PLAYER_ID, TEAM_ID, UNIFORM_NUM, FIRST_NAME, "
        "LAST_NAME, MPG, PPG, RPG, APG, SPG, BPG) "
        "VALUES (" +
        to_string(playerId) + ", " + to_string(team_id) + ", " +
        to_string(jersey_num) + ", '" + first_name + "', '" + last_name +
        "', " + to_string(mpg) + ", " + to_string(ppg) + ", " + to_string(rpg) +
        ", " + to_string(apg) + ", " + to_string(spg) + ", " + to_string(bpg) +
        " ); ";

    playerId++;

    W.exec(insert);
    W.commit();
}

void add_team(connection *C, string name, int state_id, int color_id, int wins,
              int losses) {
    // escape the single quotes for all string
    name = escapeSingleQuotes(name);

    /* Create a transactional object. */
    work W(*C);

    string insert =
        "INSERT INTO TEAM (TEAM_ID, NAME, STATE_ID, COLOR_ID, WINS, LOSSES) "
        "VALUES (" +
        to_string(teamId) + ", '" + name + "', " + to_string(state_id) + ", " +
        to_string(color_id) + ", " + to_string(wins) + ", " +
        to_string(losses) + " ); ";

    teamId++;

    W.exec(insert);
    W.commit();
}

void add_state(connection *C, string name) {
    // escape the single quotes for all string
    name = escapeSingleQuotes(name);

    /* Create a transactional object. */
    work W(*C);

    string insert =
        "INSERT INTO STATE (STATE_ID, NAME) "
        "VALUES (" +
        to_string(stateId) + ", '" + name + "' ); ";

    stateId++;

    W.exec(insert);
    W.commit();
}

void add_color(connection *C, string name) {
    // escape the single quotes for all string
    name = escapeSingleQuotes(name);

    /* Create a transactional object. */
    work W(*C);

    string insert =
        "INSERT INTO COLOR (COLOR_ID, NAME) "
        "VALUES (" +
        to_string(colorId) + ", '" + name + "' ); ";

    colorId++;

    W.exec(insert);
    W.commit();
}

/*
 * All use_ params are used as flags for corresponding attributes
 * a 1 for a use_ param means this attribute is enabled (i.e. a WHERE clause is
 * needed) a 0 for a use_ param means this attribute is disabled
 */
void query1(connection *C, int use_mpg, int min_mpg, int max_mpg, int use_ppg,
            int min_ppg, int max_ppg, int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg, int use_spg, double min_spg,
            double max_spg, int use_bpg, double min_bpg, double max_bpg) {
    string queryStr = "SELECT * from PLAYER";
    if (use_mpg != 0 || use_ppg != 0 || use_rpg != 0 || use_apg != 0 ||
        use_spg != 0 || use_bpg != 0) {
        queryStr += " WHERE ";
        bool useExsitCheck = false;
        if (use_mpg != 0) {
            if (useExsitCheck) queryStr += " AND ";
            useExsitCheck = true;
            queryStr += "mpg BETWEEN " + to_string(min_mpg) + " AND " +
                        to_string(max_mpg);
        }
        if (use_ppg != 0) {
            if (useExsitCheck) queryStr += " AND ";
            useExsitCheck = true;
            queryStr += "ppg BETWEEN " + to_string(min_ppg) + " AND " +
                        to_string(max_ppg);
        }
        if (use_rpg != 0) {
            if (useExsitCheck) queryStr += " AND ";
            useExsitCheck = true;
            queryStr += "rpg BETWEEN " + to_string(min_rpg) + " AND " +
                        to_string(max_rpg);
        }
        if (use_apg != 0) {
            if (useExsitCheck) queryStr += " AND ";
            useExsitCheck = true;
            queryStr += "apg BETWEEN " + to_string(min_apg) + " AND " +
                        to_string(max_apg);
        }
        if (use_spg != 0) {
            if (useExsitCheck) queryStr += " AND ";
            useExsitCheck = true;
            queryStr += "spg BETWEEN " + to_string(min_spg) + " AND " +
                        to_string(max_spg);
        }
        if (use_bpg != 0) {
            if (useExsitCheck) queryStr += " AND ";
            useExsitCheck = true;
            queryStr += "bpg BETWEEN " + to_string(min_bpg) + " AND " +
                        to_string(max_bpg);
        }
    }

    /* Create a non-transactional object. */
    nontransaction N(*C);
    /* Execute SQL query */
    result R(N.exec(queryStr));
    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG "
            "APG SPG BPG"
         << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>()
             << " " << c[3].as<string>() << " " << c[4].as<string>() << " "
             << c[5].as<int>() << " " << c[6].as<int>() << " " << c[7].as<int>()
             << " " << c[8].as<int>() << " " << fixed << setprecision(1)
             << c[9].as<float>() << " " << fixed << setprecision(1)
             << c[10].as<float>() << endl;
    }
}

void query2(connection *C, string team_color) {
    team_color = escapeSingleQuotes(team_color);
    string queryStr =
        "SELECT NAME FROM TEAM WHERE COLOR_ID IN ( SELECT COLOR_ID from COLOR "
        "WHERE NAME = '" +
        team_color + "' )";
    /* Create a non-transactional object. */
    nontransaction N(*C);
    /* Execute SQL query */
    result R(N.exec(queryStr));

    cout << "NAME" << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<string>() << endl;
    }
}

void query3(connection *C, string team_name) {
    team_name = escapeSingleQuotes(team_name);
    string queryStr =
        "SELECT FIRST_NAME, LAST_NAME FROM PLAYER WHERE TEAM_ID IN ( SELECT "
        "TEAM_ID from TEAM WHERE NAME = '" +
        team_name + "') ORDER BY PPG DESC";
    /* Create a non-transactional object. */
    nontransaction N(*C);
    /* Execute SQL query */
    result R(N.exec(queryStr));

    cout << "FIRST_NAME LAST_NAME" << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
    }
}

void query4(connection *C, string team_state, string team_color) {
    team_state = escapeSingleQuotes(team_state);
    team_color = escapeSingleQuotes(team_color);
    string queryStr =
        "SELECT UNIFORM_NUM, FIRST_NAME, LAST_NAME FROM PLAYER WHERE "
        "TEAM_ID IN ( SELECT TEAM_ID FROM TEAM WHERE STATE_ID IN "
        "( SELECT STATE_ID from STATE WHERE NAME = '" +
        team_state +
        "') AND COLOR_ID IN ( SELECT COLOR_ID FROM COLOR WHERE NAME = '" +
        team_color + "'))";
    /* Create a non-transactional object. */
    nontransaction N(*C);
    /* Execute SQL query */
    result R(N.exec(queryStr));

    cout << "UNIFORM_NUM FIRST_NAME LAST_NAME" << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<int>() << " " << c[1].as<string>() << " "
             << c[2].as<string>() << endl;
    }
}

void query5(connection *C, int num_wins) {
    string queryStr =
        "SELECT A.FIRST_NAME, A.LAST_NAME, B.NAME, B.WINS FROM PLAYER AS A "
        "INNER JOIN TEAM AS B ON A.TEAM_ID = B.TEAM_ID WHERE B.WINS > " +
        to_string(num_wins);
    // string queryStr =
    //     "SELECT FIRST_NAME, LAST_NAME FROM PLAYER WHERE "
    //     "TEAM_ID IN ( SELECT TEAM_ID FROM TEAM WHERE WINS > " +
    //     to_string(num_wins) + " ) UNION SELECT NAME, WINS WHERE WINS > " +
    //     to_string(num_wins);
    /* Create a non-transactional object. */
    nontransaction N(*C);
    /* Execute SQL query */
    result R(N.exec(queryStr));

    cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<string>() << " " << c[1].as<string>() << " "
             << c[2].as<string>() << " " << c[3].as<int>() << endl;
    }
}
