#include "threads.h"


ThreadData *init_threads(int thread_count) {
    ThreadData *threads;

    return threads;
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