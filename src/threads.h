#include "structs.h"
#include "utils.h"


ThreadData *init_threads(int thread_count);
uint64_t total_nodes(ThreadData *threads, int thread_count);
void stop_threads(ThreadData *threats, int thread_count);
