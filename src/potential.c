#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif



#include "uci.h"
#include "magic.h"
#include "move.h"
#include "table.h"
#include "fen.h"
#include "perft.h"














/*void perft(int depth, board* position);

int areSubStringsEqual(char *command, char *uciCommand, int stringSize);

void pBoard(board* position);

void printMoveList(moves *moveList);

void initAll();

void uciProtocol(int argc, char* argv[]);

void goCommand(char *command, board* position, time* time);

void perftRoot(int depth, board* position);

void perftChild(int depth, board* position);

void initRandomKeys();*/


void initAll() {
    initLeaperAttacks();
    initMagicNumbers();
    initSlidersAttacks(bishop);
    initSlidersAttacks(rook);
    // init random keys for tranposition table
    initRandomKeys();
    // clear hash table
    clearHashTable();
    // init mask
    initEvaluationMasks();
    // init Late Move Reduction Table
    initializeLMRTable();
    // init tranposition table
    init_hash_table(64);
}

int test_function(int a, int b) {
    return a + b;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int main(int argc, char* argv[]) {
    initAll();
    int debug = 1;
    if (debug) {
        board position;
        time_struct time_control;
        parseFEN("1rbq1rk1/6pp/2p2b2/2n2p2/2p2P2/R3P2B/1P4QP/1NB2KNR b - - 1 19", &position);

        //perftRoot(6, &position);
        goCommand("go movetime 10000 depth 40", &position, &time_control);
        //printf("Nodes: %llu\n", perftNodes);
    } else {
        uciProtocol(argc, argv);
    }
    return 0;
}
