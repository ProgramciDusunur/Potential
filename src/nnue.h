#ifndef POTENTIAL_NNUE_H
#define POTENTIAL_NNUE_H

#include <stdbool.h>
#include "structs.h"
#include "board_constants.h"

extern bool is_nnue_loaded;

bool nnue_load(const char* file_path);
int nnue_evaluate_pos(board *pos);
void test_nnue_indicies(board *pos);

void nnue_add_feature(board *pos, int piece, int square);
void nnue_remove_feature(board *pos, int piece, int square);
void nnue_refresh_accumulator(board *pos);

#endif //POTENTIAL_NNUE_H
