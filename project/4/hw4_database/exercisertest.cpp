#include "exerciser.h"

void exercise(connection *C) {
    query1(C, 1, 20, 30, 1, 10, 20, 1, 2, 5, 1, 1, 3, 1, 1, 2, 1, 0, 1);
    query1(C, 1, 35, 40, 0, 10, 20, 0, 2, 5, 0, 1, 3, 0, 1, 2, 0, 0, 1);
    query2(C, "Maroon");
    query3(C, "Duke");
    query4(C, "VA", "Maroon");
    query5(C, 10);
}