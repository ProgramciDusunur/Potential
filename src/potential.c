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
    int debug = 0;
    if (debug) {
        board position;
        parseFEN(startPosition, &position);

        perftRoot(6, &position);
        printf("Nodes: %llu", perftNodes);
    } else {
        int safetyMargin = 10;

        board *position = (board *)malloc(sizeof(board));
        time *time_ctrl = (time *)malloc(sizeof(time));
        SearchStack *ss = (SearchStack *)calloc(maxPly + safetyMargin, sizeof(SearchStack));
        SearchStack *ss_start = ss + safetyMargin;

        uciProtocol(argc, argv, position, ss_start, time_ctrl);

        // free SearchStack struct
        free(ss);
        // free board struct
        free(position);
        // free time struct
        free(time_ctrl);
        // free hash table
        free(hashTable);

    }
    return 0;
}
