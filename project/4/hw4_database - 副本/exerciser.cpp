#include "exerciser.h"

void exercise(connection *C) {
    // q1 test
    // void query1(connection *C, int use_mpg, int min_mpg, int max_mpg, int use_ppg,
    //         int min_ppg, int max_ppg, int use_rpg, int min_rpg, int max_rpg,
    //         int use_apg, int min_apg, int max_apg, int use_spg, double min_spg,
    //         double max_spg, int use_bpg, double min_bpg, double max_bpg) {
    /*
    query1(C, 1, 35, 40, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2);
    // query1(C, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2);
    // query1(C, 1, 35, 40, 0, 1, 2, 0, 1, 2, 0, 1, 2, 1, 0, 1, 0, 1, 2);
    // query1(C, 1, 35, 40, 0, 1, 2, 0, 1, 2, 0, 1, 2, 1, 0, 1, 1, 1, 2);
    */
    // q2 test
    /*
    query2(C, "black");
    query2(C, "Black");
    */
    // q3 test
    /*
    query3(C, "duke");
    query3(C, "Duke");
    */
    query3(C, "UNC");
    // q4 test
    /*
    query4(C, "NC", "lightblue");
    query4(C, "NC", "LightBlue");
    query4(C, "nc", "LightBlue");
    */
    // q5 test
    // /*
    // query5(C, 0);
    // query5(C, 2);
    // query5(C, 10);
    // query5(C, 13);
    // */
}
