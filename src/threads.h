#include "structs.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

extern ThreadPool thread_pool;

void setup_main_thread(board *board);
void init_threads(int requested_count);
uint64_t total_nodes(ThreadData *threads, int thread_count);
void stop_threads(ThreadData *threats, int thread_count);
