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

size_t how_many_mb(size_t mb) {
    return (mb * 1024ULL * 1024ULL) / sizeof(uint32_t);
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
            _fseeki64(f, 0, SEEK_END);
            
            int64_t file_size = _ftelli64(f);
            
            _fseeki64(f, 0, SEEK_SET);

            printf("Total file size: %lld bytes\n", file_size);
            fflush(stdout); // Ensure the output is printed immediately
            
            uint32_t BUFFER_SIZE = how_many_mb(10);
            uint32_t *buffer = (uint32_t *)malloc(BUFFER_SIZE * sizeof(uint32_t));
            uint64_t game_count = 0;
            uint64_t total_bytes_read = 0;
            int header_skip = 8; // Skip the first 8 bytes (occupancy)
            int first_threshold = 10;
            size_t number_of_items_read;
            clock_t start = clock();
            while ((number_of_items_read = fread(buffer, sizeof(uint32_t), BUFFER_SIZE, f)) > 0) {
                total_bytes_read += number_of_items_read * sizeof(uint32_t);
                for (size_t i = 0; i < number_of_items_read; i++) {                    
                    if (header_skip > 0) {header_skip--;}
                    // Null terminator indicates the end of a game in the viriformat
                    if (buffer[i] == 0 && header_skip == 0) {
                        game_count++;
                        header_skip = 8; // Reset header skip for the next occupancy
                    }
                    int current_threshold = (int)((total_bytes_read * 100ULL) / file_size);

                    if (current_threshold >= first_threshold) {
                        printf("[%d%%]:  Progress\n", current_threshold);
                        fflush(stdout); // Ensure the output is printed immediately
                        first_threshold += 10; // Update the next threshold

                    }
                }
                
            }

            printf("Total number of games in the file: %llu\n", game_count);
            clock_t end = clock();
            int total_mb = (int)(total_bytes_read / (1024 * 1024));
            double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("Total consumed time: %.2f second | Speed: %.2f MB/s\n", elapsed_time, total_mb / elapsed_time);

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
