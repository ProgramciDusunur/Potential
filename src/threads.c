#include "threads.h"

ThreadPool thread_pool;

void setup_main_thread(board *board) {
    if (thread_pool.threads[0] == NULL) {
        printf("info string main thread is NULL!");
        exit(1);
    }
    memcpy(&thread_pool.threads[0]->pos, board, sizeof(*board));
}

void init_threads(int requested_count) {
    if (requested_count < 1) requested_count = 1;
    if (requested_count > MAX_THREADS) requested_count = MAX_THREADS;

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

        if (i > 0) {
            // pthread_mutex_init(&thread_pool.threads[i]->mutex, NULL);
            // pthread_cond_init(&thread_pool.threads[i]->sleep_cond, NULL);
            // thread_pool.threads[i]->searching = false;
            // pthread_create(&thread_pool.threads[i]->native_handle, NULL, ThreadLoop, thread_pool.threads[i]);
        }
    }
}

uint64_t total_nodes(ThreadData *threads, int thread_count) {
    uint64_t total_nodes = 0;
    for (int i = 0;i < thread_count;i++) {        
        total_nodes += load_rlx(threads->search_i.nodes_searched);
    }
    return total_nodes;
}

void stop_threads(ThreadData *threats, int thread_count) {
    for (int i = 0;i < thread_count;i++) {
        threats->search_i.stopped = true;
    }
}