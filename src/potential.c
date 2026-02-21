#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>



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
    init_helper_bb();
}


int main(int argc, char* argv[]) {
    // init main thread
    init_threads(1);

    initAll();
    int debug = 0;
    if (debug) {
        board position;
        parseFEN(startPosition, &position);

        setup_main_thread(&position);

        pBoard(&thread_pool.threads[0]->pos);
    
        /*
        perftRoot(7, &position);
        printf("Nodes: %llu", perftNodes);*/
        

        //perftRoot(7, &position);
        //printf("Nodes: %llu", perftNodes);

        /*pthread_t t1, t2;

        pthread_create(&t1, NULL, sayac_arttir, NULL);
        pthread_create(&t2, NULL, sayac_arttir, NULL);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        printf("Beklenen: 20000000\n");
        printf("Gerçekleşen: %lld\n", counter);
        
        if (counter != 20000000) {
            printf("HATA! Bazı sayılar kayboldu!\n");
        } else {
            printf("Tam isabet!\n");
        }
        return 0;*/
        

    } else {
        int safety_margin = STACK_SAFETY_MARGIN;
        int offset = STACK_OFFSET; // offset for safety margin to allow negative indexing in the search stack

        board *position = (board *)malloc(sizeof(board));
        my_time *time_ctrl = (my_time *)malloc(sizeof(my_time));

        // allocate search stack with a safety margin to prevent overflow
        //                             Safety Margin Layot
        //      [-10 ply safety margin ... max ply ... +10 ply safety margin]        
        SearchStack *ss_base = (SearchStack *)malloc(sizeof(SearchStack) * (maxPly + safety_margin));

        if (!ss_base) {
            fprintf(stderr, "Failed to allocate memory for search stack\n");
            return 1;
        }

        SearchStack *ss = ss_base + offset; // shift pointer to the middle of the allocated stack to create a safety margin for negative indices

        uciProtocol(argc, argv, position, time_ctrl, ss);
        free(position);
        free(time_ctrl);
        free(ss_base);
    }
    return 0;
}
