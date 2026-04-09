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
    int debug = 1;
    if (debug) {
        board position;
        parseFEN("8/2kp3Q/8/8/8/2K5/8/8 b - - 0 1", &position);

        // Double checkers position
        /*U64 checkers = get_checkers(&position);
        printBitboard(checkers);*/

        moves moveList;

        legal_make_move(0, &position);

        // Attacked bitboard
        legal_move_generator(&moveList, &position);

        printMoveList(&moveList);

        board position2;
        parseFEN(startPosition, &position2);

        board position3;
        parseFEN(startPosition, &position3);
          

        
        /*int depth = 6;
        perftNodes = 0;
        int startTime = getTimeMiliSecond();
        perftRoot(depth, &position);
        int duration = getTimeMiliSecond() - startTime;
        printf("total: %llu\n", perftNodes);
        printf("nps: %llu\n", (U64)perftNodes * 1000 / myMAX(1, duration));*/

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
        board *position = (board *)malloc(sizeof(board));
        my_time *time_ctrl = (my_time *)malloc(sizeof(my_time));

        uciProtocol(argc, argv, position, time_ctrl);
        free(position);
        free(time_ctrl);
    }
    return 0;
}
