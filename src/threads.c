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

void free_threads(void) {
    for (int i = 0; i < thread_pool.thread_count; i++) {
        if (thread_pool.threads[i] != NULL) {
            free(thread_pool.threads[i]);
            thread_pool.threads[i] = NULL;
        }
    }
    thread_pool.thread_count = 0;

    for (int i = 0; i < thread_pool.shared_history_count; i++) {
        if (thread_pool.shared_histories[i] != NULL) {
            SharedHistory *sh = thread_pool.shared_histories[i];
            free(sh->pawnHistory);
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

uint64_t total_nodes(void) {
    uint64_t nodes = 0;
    for (int i = 0; i < thread_pool.thread_count; i++) {        
        nodes += load_rlx(thread_pool.threads[i]->search_i.nodes_searched);
    }
    return nodes;
}

void *thread_entry(void *arg) {
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

    while (true) {
        pthread_mutex_lock(&thread_pool.mutex);
        while (t->generation == thread_pool.search_generation
               && thread_pool.threads_alive) {
            pthread_cond_wait(&thread_pool.start_cond, &thread_pool.mutex);
        }

        if (!thread_pool.threads_alive) {
            pthread_mutex_unlock(&thread_pool.mutex);
            break;
        }

        t->generation = thread_pool.search_generation;
        pthread_mutex_unlock(&thread_pool.mutex);

        searchPosition(t->search_depth, false, t, t->time);

        pthread_mutex_lock(&thread_pool.mutex);
        thread_pool.helpers_running--;
        if (thread_pool.helpers_running == 0) {
            pthread_cond_signal(&thread_pool.done_cond);
        }
        pthread_mutex_unlock(&thread_pool.mutex);
    }
    return NULL;
}

void destroy_threads(void) {
    if (thread_pool.thread_count <= 0) return;

    if (thread_pool.thread_count > 1) {
        store_rlx(thread_pool.stop, true);

        pthread_mutex_lock(&thread_pool.mutex);
        thread_pool.threads_alive = false;
        pthread_cond_broadcast(&thread_pool.start_cond);
        pthread_mutex_unlock(&thread_pool.mutex);

        for (int i = 1; i < thread_pool.thread_count; i++) {
            pthread_join(thread_pool.threads[i]->native_handle, NULL);
        }
    }

    free_threads();

    pthread_mutex_destroy(&thread_pool.mutex);
    pthread_cond_destroy(&thread_pool.start_cond);
    pthread_cond_destroy(&thread_pool.done_cond);
}

void init_threads(int requested_count) {
    if (requested_count < 1) requested_count = 1;
    if (requested_count > MAX_THREADS) requested_count = MAX_THREADS;

    destroy_threads();

    thread_pool.thread_count = requested_count;
    store_rlx(thread_pool.stop, false);

    pthread_mutex_init(&thread_pool.mutex, NULL);
    pthread_cond_init(&thread_pool.start_cond, NULL);
    pthread_cond_init(&thread_pool.done_cond, NULL);
    thread_pool.threads_alive = true;
    thread_pool.helpers_running = 0;
    thread_pool.search_generation = 0;

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

        int pawn_hist_size = BASE_PAWNHIST_SIZE * scale;
        sh->pawn_history_mask = pawn_hist_size - 1;
        sh->pawnHistory = (int16_t (*)[12][64])calloc(pawn_hist_size, sizeof(int16_t[12][64]));
        
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
        thread_pool.threads[i]->generation = 0;
        clearStaticEvaluationHistory(thread_pool.threads[i]->ss);
    }

    for (int i = 1; i < requested_count; i++) {
        pthread_create(&thread_pool.threads[i]->native_handle, NULL, thread_entry, thread_pool.threads[i]);
    }
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
    }

    if (thread_pool.thread_count > 1) {
        pthread_mutex_lock(&thread_pool.mutex);
        thread_pool.helpers_running = thread_pool.thread_count - 1;
        thread_pool.search_generation++;
        pthread_cond_broadcast(&thread_pool.start_cond);
        pthread_mutex_unlock(&thread_pool.mutex);
    }
}

void wait_helpers(void) {
    // Signal all helpers to stop
    store_rlx(thread_pool.stop, true);

    if (thread_pool.thread_count > 1) {
        pthread_mutex_lock(&thread_pool.mutex);
        while (thread_pool.helpers_running > 0) {
            pthread_cond_wait(&thread_pool.done_cond, &thread_pool.mutex);
        }
        pthread_mutex_unlock(&thread_pool.mutex);
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