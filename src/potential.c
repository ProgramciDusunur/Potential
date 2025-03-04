#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#include "uci.h"
#include "magic.h"
#include "move.h"
#include "table.h"
#include "fen.h"
#include "perft.h"
#include "see.h"














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

void initAll(void) {
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


int main(int argc, char* argv[]) {
    initAll();
    int debug = 1;
    if (debug) {
        /*board position;
        parseFEN(startPosition, &position);

        perftRoot(7, &position);
        printf("Nodes: %llu", perftNodes);*/
        testSEE();
    } else {
        uciProtocol(argc, argv);
    }
    return 0;
}
