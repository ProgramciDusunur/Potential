#include "structs.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

extern ThreadPool thread_pool;

void setup_main_thread(board *board);
void init_threads(int requested_count);
uint64_t total_nodes(void);
void start_helpers(board *root_pos, int depth, my_time *time);
void wait_helpers(void);
