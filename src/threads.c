#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "threads.h"
#include "search.h"

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

static void free_threads(void) {
    for (int i = 0; i < thread_pool.thread_count; i++) {
        if (thread_pool.threads[i] != NULL) {
            free(thread_pool.threads[i]);
            thread_pool.threads[i] = NULL;
        }
    }
    thread_pool.thread_count = 0;

    for (int i = 0; i < thread_pool.shared_history_count; i++) {
        if (thread_pool.shared_histories[i] != NULL) {
            free(thread_pool.shared_histories[i]);
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
        // Note: calloc allocates zeroed memory lazily. The physical RAM pages will only be
        // allocated when the threads first write to them during search, perfectly assigning
        // the memory to their local NUMA node!
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

    searchPosition(t->search_depth, false, t, t->time);
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

        // Launch thread
        pthread_create(&t->native_handle, NULL, thread_entry, t);
    }
}

void wait_helpers(void) {
    // Signal all helpers to stop
    store_rlx(thread_pool.stop, true);

    for (int i = 1; i < thread_pool.thread_count; i++) {
        pthread_join(thread_pool.threads[i]->native_handle, NULL);
    }
}

int select_thread(void) {
    int how_many_threads = thread_pool.thread_count;

    // If only one thread, main thread is best
    if (how_many_threads == 1) return 0;

    int best_thread = 0;
    int best_voting_score = -1000000;

    for (int i = 0; i < how_many_threads; i++) {
        ThreadData *td = thread_pool.threads[i];
        int depth = td->search_i.depthCompleted;

        // Skip threads that haven't completed any depth
        if (depth == 0) continue;

        int score = td->search_i.score;

        // Stormphrax-style voting: weight = depth * 100 + score
        // Higher completed depth is strongly preferred, score breaks ties
        int voting_score = depth * 100 + score;

        if (voting_score > best_voting_score) {
            best_voting_score = voting_score;
            best_thread = i;
        }
    }

    return best_thread;
}