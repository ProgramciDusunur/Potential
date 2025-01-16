#include <stdio.h>



#include "uci.h"
#include "magic.h"
#include "move.h"
#include "table.h"
#include "fen.h"
#include "perft.h"



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

        //int move = encodeMove(f1, f4, R, 0, 1, 0, 0, 0);

        //printf("SEE result: %d", SEE(&position, move, 100));

        perftRoot(6, &position);
        printf("Nodes: %llu", perftNodes);
    } else {
        uciProtocol(argc, argv);
    }
    return 0;
}
