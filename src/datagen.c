#include "datagen.h"
#include "generate_fen.h"
#include "search.h"
#include "timeman.h"
#include "threads.h"
#include "perft.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#define MAX_GAME_PLYS 400

_Atomic uint64_t target_fens_limit = 0;

void datagen_worker(int thread_id, uint64_t target_games, int nodes_limit, int use_book) {
    char out_filename[256];
    char illegal_filename[256];
    snprintf(out_filename, sizeof(out_filename), "datagen/datagen_%d.txt", thread_id);
    snprintf(illegal_filename, sizeof(illegal_filename), "datagen/illegal_%d.txt", thread_id);

    FILE *out_file = fopen(out_filename, "a");
    FILE *illegal_file = fopen(illegal_filename, "a");

    if (!out_file || !illegal_file) {
        if (out_file) fclose(out_file);
        if (illegal_file) fclose(illegal_file);
        return;
    }

    ThreadData *t = thread_pool.threads[thread_id];

    while (1) {
        // Check game limit
        uint64_t current_game = atomic_load_explicit(&games_played_count, memory_order_relaxed);
        if (target_games > 0 && current_game >= target_games) {
            break;
        }

        // Check FEN limit
        uint64_t current_fens = atomic_load_explicit(&total_fens_generated, memory_order_relaxed);
        uint64_t f_limit = atomic_load_explicit(&target_fens_limit, memory_order_relaxed);
        if (f_limit > 0 && current_fens >= f_limit) {
            break;
        }

        atomic_fetch_add_explicit(&games_played_count, 1, memory_order_relaxed);

        uint64_t new_fens = play_selfgen_game(out_file, illegal_file, nodes_limit, use_book, t);
        uint64_t current_total = (uint64_t)atomic_fetch_add_explicit(&total_fens_generated, new_fens, memory_order_relaxed) + new_fens;
        
        if ((current_game + 1) % 100 == 0) {
            uint64_t elapsed = getTimeMiliSecond() - global_start_time;
            if (elapsed == 0) elapsed = 1;
            printf("Played %" PRIu64 " Games... (%" PRIu64 " FENs, %" PRIu64 " FEN/s)\n", current_game + 1, current_total, current_total * 1000 / elapsed);
        }
    }

    fclose(out_file);
    fclose(illegal_file);
}

struct datagen_args {
    int id;
    uint64_t target;
    int nodes;
    int book;
};

void* datagen_worker_proxy(void* arg) {
    struct datagen_args *args = (struct datagen_args *)arg;
    datagen_worker(args->id, args->target, args->nodes, args->book);
    return NULL;
}
