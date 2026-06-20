#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "threads.h"
#include "search.h"
#include "utils.h"
#include "history.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sched.h>
#include <pthread.h>
#endif

ThreadPool thread_pool;

void setup_main_thread(board *board) {
    if (thread_pool.threads[0] == NULL) {
        printf("info string main thread is NULL!");
        exit(1);
    }
    memcpy(&thread_pool.threads[0]->pos, board, sizeof(*board));
}

static void *thread_entry(void *arg);

static void free_threads(void) {
    // Wake up sleeping threads and join them
    for (int i = 1; i < thread_pool.thread_count; i++) {
        if (thread_pool.threads[i] != NULL) {
            ThreadData *t = thread_pool.threads[i];
            pthread_mutex_lock(&t->mutex);
            t->exit_thread = true;
            pthread_cond_signal(&t->sleep_cond);
            pthread_mutex_unlock(&t->mutex);
            pthread_join(t->native_handle, NULL);
        }
    }

    for (int i = 0; i < thread_pool.thread_count; i++) {
        if (thread_pool.threads[i] != NULL) {
            if (i > 0) {
                pthread_mutex_destroy(&thread_pool.threads[i]->mutex);
                pthread_cond_destroy(&thread_pool.threads[i]->sleep_cond);
                pthread_cond_destroy(&thread_pool.threads[i]->finished_cond);
            }
            free(thread_pool.threads[i]);
            thread_pool.threads[i] = NULL;
        }
    }
    thread_pool.thread_count = 0;

    for (int i = 0; i < thread_pool.shared_history_count; i++) {
        if (thread_pool.shared_histories[i] != NULL) {
            SharedHistory *sh = thread_pool.shared_histories[i];
            for (int c = 0; c < 2; c++) {
                free(sh->pawn_corrhist[c]);
                free(sh->minor_corrhist[c]);
                free(sh->major_corrhist[c]);
                free(sh->krp_corrhist[c]);
                for (int c2 = 0; c2 < 2; c2++) {
                    free(sh->non_pawn_corrhist[c][c2]);
                }
            }
            free(sh);
            thread_pool.shared_histories[i] = NULL;
        }
    }
    if (thread_pool.shared_histories != NULL) {
        free(thread_pool.shared_histories);
        thread_pool.shared_histories = NULL;
    }
    thread_pool.shared_history_count = 0;
}

void init_threads(int requested_count) {
    if (requested_count < 1) requested_count = 1;
    if (requested_count > MAX_THREADS) requested_count = MAX_THREADS;

    // Free existing threads first
    free_threads();

    thread_pool.thread_count = requested_count;
    store_rlx(thread_pool.stop, false);

    // Default to 8 threads per L3 Cache domain
    int threads_per_l3 = 8;
    thread_pool.shared_history_count = (requested_count + threads_per_l3 - 1) / threads_per_l3;
    thread_pool.shared_histories = (SharedHistory **)malloc(thread_pool.shared_history_count * sizeof(SharedHistory *));

    for (int i = 0; i < thread_pool.shared_history_count; i++) {
        thread_pool.shared_histories[i] = (SharedHistory *)calloc(1, sizeof(SharedHistory));
        SharedHistory *sh = thread_pool.shared_histories[i];
        
        int local_threads = requested_count - (i * threads_per_l3);
        if (local_threads > threads_per_l3) local_threads = threads_per_l3;
        
        int scale = next_power_of_2(local_threads);
        int corrhist_size = BASE_CORRHIST_SIZE * scale;
        sh->corrhist_mask = corrhist_size - 1;
        
        for (int c = 0; c < 2; c++) {
            sh->pawn_corrhist[c] = (int16_t *)calloc(corrhist_size, sizeof(int16_t));
            sh->minor_corrhist[c] = (int16_t *)calloc(corrhist_size, sizeof(int16_t));
            sh->major_corrhist[c] = (int16_t *)calloc(corrhist_size, sizeof(int16_t));
            sh->krp_corrhist[c] = (int16_t *)calloc(corrhist_size, sizeof(int16_t));
            for (int c2 = 0; c2 < 2; c2++) {
                sh->non_pawn_corrhist[c][c2] = (int16_t *)calloc(corrhist_size, sizeof(int16_t));
            }
        }
    }

    for (int i = 0; i < requested_count; i++) {
        thread_pool.threads[i] = (ThreadData *)malloc(sizeof(ThreadData));
        
        if (thread_pool.threads[i] == NULL) {
            printf("FATAL ERROR: Memory allocation failed for thread %d\n", i);
            exit(1);
        }

        memset(thread_pool.threads[i], 0, sizeof(ThreadData));
        thread_pool.threads[i]->id = i;
        thread_pool.threads[i]->shared_history = thread_pool.shared_histories[i / threads_per_l3];
        thread_pool.threads[i]->ss = thread_pool.threads[i]->ss_base + STACK_OFFSET;
        clearStaticEvaluationHistory(thread_pool.threads[i]->ss);

        if (i > 0) {
            pthread_mutex_init(&thread_pool.threads[i]->mutex, NULL);
            pthread_cond_init(&thread_pool.threads[i]->sleep_cond, NULL);
            pthread_cond_init(&thread_pool.threads[i]->finished_cond, NULL);
            thread_pool.threads[i]->searching = false;
            thread_pool.threads[i]->exit_thread = false;
            
            // Launch thread once during initialization
            pthread_create(&thread_pool.threads[i]->native_handle, NULL, thread_entry, thread_pool.threads[i]);
        }
    }
}

uint64_t total_nodes(void) {
    uint64_t nodes = 0;
    for (int i = 0; i < thread_pool.thread_count; i++) {        
        nodes += load_rlx(thread_pool.threads[i]->search_i.nodes_searched);
    }
    return nodes;
}

// Thread entry function for helper threads
static void *thread_entry(void *arg) {
    ThreadData *t = (ThreadData *)arg;

#if defined(_WIN32)
    // Pin thread to specific logical processor for NUMA-awareness (Windows)
    HANDLE native_thread = GetCurrentThread();
    GROUP_AFFINITY affinity;
    memset(&affinity, 0, sizeof(GROUP_AFFINITY));
    affinity.Group = t->id / 64;
    affinity.Mask = 1ULL << (t->id % 64);
    SetThreadGroupAffinity(native_thread, &affinity, NULL);
#elif defined(__linux__)
    // Pin thread to specific logical processor for NUMA-awareness (Linux)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(t->id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#endif

    while (1) {
        pthread_mutex_lock(&t->mutex);
        while (!t->searching && !t->exit_thread) {
            pthread_cond_wait(&t->sleep_cond, &t->mutex);
        }
        pthread_mutex_unlock(&t->mutex);

        if (t->exit_thread) {
            break;
        }

        searchPosition(t->search_depth, false, t, t->time);

        pthread_mutex_lock(&t->mutex);
        t->searching = false;
        pthread_cond_signal(&t->finished_cond);
        pthread_mutex_unlock(&t->mutex);
    }
    
    return NULL;
}

void start_helpers(board *root_pos, int depth, my_time *time) {
    store_rlx(thread_pool.stop, false);

    for (int i = 1; i < thread_pool.thread_count; i++) {
        ThreadData *t = thread_pool.threads[i];

        // Copy position from root
        memcpy(&t->pos, root_pos, sizeof(board));
        t->pos.ply = 0;
        t->pos.seldepth = 0;

        // Reset search info
        store_rlx(t->search_i.nodes_searched, 0);
        t->search_i.stopped = false;

        // Clear search stack
        memset(t->ss_base, 0, sizeof(SearchStack) * (maxPly + 20));

        // Set search parameters
        t->search_depth = depth;
        t->time = time;

        // Wake up sleeping thread
        pthread_mutex_lock(&t->mutex);
        t->searching = true;
        pthread_cond_signal(&t->sleep_cond);
        pthread_mutex_unlock(&t->mutex);
    }
}

void wait_helpers(void) {
    // Signal all helpers to stop
    store_rlx(thread_pool.stop, true);

    for (int i = 1; i < thread_pool.thread_count; i++) {
        ThreadData *t = thread_pool.threads[i];
        pthread_mutex_lock(&t->mutex);
        while (t->searching) {
            pthread_cond_wait(&t->finished_cond, &t->mutex);
        }
        pthread_mutex_unlock(&t->mutex);
    }
}

int select_thread(void) {
    int how_many_threads = thread_pool.thread_count;
    if (how_many_threads == 1) return 0;

    int64_t votes[4096];
    memset(votes, 0, sizeof(votes));
    
    int minScore = 999999;
    int first_valid_thread = -1;
    
    for (int i = 0; i < how_many_threads; i++) {
        ThreadData *td = thread_pool.threads[i];
        if (td->search_i.depthCompleted == 0 || td->pos.pvTable[0][0] == 0) continue;
        
        if (first_valid_thread == -1) first_valid_thread = i;
        
        if (td->search_i.score < minScore) {
            minScore = td->search_i.score;
        }
    }

    if (first_valid_thread == -1) return 0;
    
    int best_thread = first_valid_thread;
    
    for (int i = 0; i < how_many_threads; i++) {
        ThreadData *td = thread_pool.threads[i];
        if (td->search_i.depthCompleted == 0 || td->pos.pvTable[0][0] == 0) continue;
        
        uint16_t move = td->pos.pvTable[0][0];
        int moveIdx = (getMoveSource(move) * 64 + getMoveTarget(move)) & 4095;
        
        int64_t weight = (int64_t)(td->search_i.score - minScore + 50) 
                       * (int64_t)td->search_i.depthCompleted;
                       
        votes[moveIdx] += weight;
        
        uint16_t best_move = thread_pool.threads[best_thread]->pos.pvTable[0][0];
        int bestMoveIdx = (getMoveSource(best_move) * 64 + getMoveTarget(best_move)) & 4095;
        
        if (votes[moveIdx] > votes[bestMoveIdx]) {
            best_thread = i;
        }
    }
    
    return best_thread;
}