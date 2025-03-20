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
    init_tables();
}


int main(int argc, char* argv[]) {
    initAll();
    int debug = 0;
    if (debug) {
        board position;
        parseFEN("8/1R4R1/8/8/8/8/8/8 w - - 0 1", &position);
        pBoard(&position);
        U64 attacks = getRookAttacks(b7, position.occupancies[both]);
        printBitboard(attacks & position.bitboards[R]);
        //printBitboard(position.bitboards[R]);


        //perftRoot(7, &position);
        //printf("Nodes: %llu", perftNodes);*
        

    } else {
        uciProtocol(argc, argv);
    }
    return 0;
}
