#include <stdio.h>
#include <stdlib.h>



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

        perftRoot(6, &position);
        printf("Nodes: %llu", perftNodes);
    } else {
        board *position = (board *)malloc(sizeof(board));
        time *time_ctrl = (time *)malloc(sizeof(time));
        SearchData *sd = (SearchData *)malloc(sizeof(SearchData));


        uciProtocol(argc, argv, position, time_ctrl, sd);

        // free SearchData struct
        free(sd);
        // free board struct
        free(position);
        // free time struct
        free(time_ctrl);
        // free hash table
        free(hashTable);
    }
    return 0;
}
