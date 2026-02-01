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
#include "generate_fen.h"





void initAll(void) {
    initLeaperAttacks();
    // init random keys for tranposition table
    initRandomKeys();
    initMagicNumbers();
    initSlidersAttacks(bishop);
    initSlidersAttacks(rook);    
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
    int debug = 1;
    if (debug) {
        board position;
     parseFEN("8/8/8/k2n4/8/4P3/8/K2N1r2 b - - 0 1", &position);
        
    uint16_t move = encodeMove(d5, e3, mf_capture);
    
   
   
    int is_profitable = SEE(&position, move, 0);
    
    init_helper_bb();

        


        /*
        perftRoot(7, &position);
        printf("Nodes: %llu", perftNodes);*/
        

        //perftRoot(7, &position);
        //printf("Nodes: %llu", perftNodes);
        

    } else {
        board *position = (board *)malloc(sizeof(board));
        my_time *time_ctrl = (my_time *)malloc(sizeof(my_time));
        uciProtocol(argc, argv, position, time_ctrl);
        free(position);
        free(time_ctrl);
    }
    return 0;
}
