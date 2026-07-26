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
#include "spsa.h"




void init_all(void) {
    init_leaper_attacks();
    // init random keys for tranposition table
    init_random_keys();
    init_magic_numbers();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);    
    // clear hash table
    clear_hash_table();
    // init mask
    init_evaluation_masks();
    // init Late Move Reduction Table
    init_lmr_table();
    // init tranposition table
    init_hash_table(64);
    init_tables();
    init_helper_bb();
    init_king_anti_diag_mask();
    // init SPSA tuning parameters (no-op in normal builds)
    spsa_init();
}


int main(int argc, char* argv[]) {
    // init main thread
    init_threads(1);

    init_all();
    int debug = 0;
    if (debug) {
        board position;
        parseFEN(startPosition, &position);
        
        U64 b_long_castle_occupancy_mask = 0ULL;        
        setBit(b_long_castle_occupancy_mask, c8);
        setBit(b_long_castle_occupancy_mask, d8);
                
        
        printBitboard(b_long_castle_occupancy_mask);
        printf("b_long_castle_occupancy_mask = %llu (0x%llx)\n", b_long_castle_occupancy_mask, b_long_castle_occupancy_mask);
        
        
        /*int depth = 6;
        perftNodes = 0;
        int startTime3 = getTimeMiliSecond();        
        perft_root_legal_bulk(depth, &position);
        int duration3 = getTimeMiliSecond() - startTime3;
        printf("total: %llu\n", perftNodes);
        printf("Legal Bulk Move Generator NPS: %llu\n", (U64)perftNodes * 1000 / myMAX(1, duration3));*/

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
