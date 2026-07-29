#ifndef POTENTIAL_NNUE_H
#define POTENTIAL_NNUE_H

#include <stdbool.h>
#include "structs.h"
#include "board_constants.h"


bool nnue_load(const char* file_path);
int nnue_evaluate_pos(board *pos);
void test_nnue_indicies(board *pos);

#endif //POTENTIAL_NNUE_H
