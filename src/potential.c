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
    init_hash_table(64);
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

    int debug = 1;
    if (debug) {
        char *file_name = "C:\\Users\\erena\\Desktop\\viriformat-training\\combined-multilayer-60m-games.vf_sanitised";
        FILE *f = fopen(file_name, "rb");

        if (f == NULL) {
            printf("Error opening file: %s\n", file_name);
            return 1;
        } else {
            uint32_t BUFFER_SIZE = 4 * 1024 * 1024; // 16 MB
            uint32_t *buffer = (uint32_t *)malloc(BUFFER_SIZE * sizeof(uint32_t));
            int line_number = 0;
            size_t number_of_items_read;
            while ((number_of_items_read = fread(buffer, sizeof(uint32_t), BUFFER_SIZE, f)) > 0) {
                for (size_t i = 0; i < number_of_items_read; i++) {
                    printf("buffer[%zu] = %02X %02X %02X %02X\n", i,
                    (buffer[i] & 0xFF),
                    ((buffer[i] >> 8) & 0xFF),
                    ((buffer[i] >> 16) & 0xFF),
                    ((buffer[i] >> 24) & 0xFF));
                    if (i == 7) {
                        break;
                    }
                }
                
            }

            free(buffer);
            fclose(f);
        }
    } 
    else {
        board *position = (board *)malloc(sizeof(board));
        my_time *time_ctrl = (my_time *)malloc(sizeof(my_time));

        uciProtocol(argc, argv, position, time_ctrl);
        free(position);
        free(time_ctrl);
    }

    
    return 0;
}
