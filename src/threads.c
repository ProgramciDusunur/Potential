#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "threads.h"
#include "search.h"

#if defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#include <sched.h>
#include <pthread.h>
#endif

// Helper function to pin a thread to a specific core
static inline void set_thread_affinity(int core_id) {
    if (core_id < 0) return;

#if defined(__linux__) || defined(__gnu_linux__) || defined(linux)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#endif
}



ThreadPool thread_pool;

void setup_main_thread(board *board) {
    if (thread_pool.threads[0] == NULL) {
        printf("info string main thread is NULL!");
        exit(1);
    }
    memcpy(&thread_pool.threads[0]->pos, board, sizeof(*board));

    // Pin main thread to core 0 (or thread's assigned ID)
    set_thread_affinity(thread_pool.threads[0]->id);
}

static void free_threads(void) {
    for (int i = 0; i < thread_pool.thread_count; i++) {
        if (thread_pool.threads[i] != NULL) {
            free(thread_pool.threads[i]);
            thread_pool.threads[i] = NULL;
        }
    }
    thread_pool.thread_count = 0;
}

void init_threads(int requested_count) {
    if (requested_count < 1) requested_count = 1;
    if (requested_count > MAX_THREADS) requested_count = MAX_THREADS;

    // Free existing threads first
    free_threads();

    thread_pool.thread_count = requested_count;
    store_rlx(thread_pool.stop, false);

    for (int i = 0; i < requested_count; i++) {
        thread_pool.threads[i] = (ThreadData *)malloc(sizeof(ThreadData));
        
        if (thread_pool.threads[i] == NULL) {
            printf("FATAL ERROR: Memory allocation failed for thread %d\n", i);
            exit(1);
        }

        memset(thread_pool.threads[i], 0, sizeof(ThreadData));
        thread_pool.threads[i]->id = i;
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

    // Pin helper thread to its designated core
    set_thread_affinity(t->id);

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