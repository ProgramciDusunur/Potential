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
#include "nnue.h"




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
    // load the net
    if (!nnue_load("beans.bin")) {
        fprintf(stderr, "Failed to load NNUE file: nnue.bin\n");
        exit(EXIT_FAILURE);
    }
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

        test_nnue_indicies(&position);

        board position2;
        parseFEN(kiwipete, &position2);
        int startpos_eval = nnue_evaluate_pos(&position);
        int kiwipete_eval = nnue_evaluate_pos(&position2);
        printf("NNUE evaluation of startpos: %d\n", startpos_eval);
        printf("NNUE evaluation of kiwipete: %d\n", kiwipete_eval);
        
    } else {
        board *position = (board *)malloc(sizeof(board));
        my_time *time_ctrl = (my_time *)malloc(sizeof(my_time));

        uciProtocol(argc, argv, position, time_ctrl);
        free(position);
        free(time_ctrl);
    }
    return 0;
}
