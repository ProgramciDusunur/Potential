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
#include "cuckoo.h"
#include "threats.h"




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
    init_hash_table(1);
    init_tables();
    init_helper_bb();
    init_king_anti_diag_mask();
    // init SPSA tuning parameters (no-op in normal builds)
    spsa_init();
    cuckoo_init();
    init_threat_geometry();
    init_threat_sliders();
    init_nnue();
    
}


int main(int argc, char* argv[]) {    
    // init main thread
    init_threads(1);

    init_all();
    board *position = (board *)malloc(sizeof(board));
    my_time *time_ctrl = (my_time *)malloc(sizeof(my_time));

    uciProtocol(argc, argv, position, time_ctrl);
    free(position);
    free(time_ctrl);
    return 0;
}
