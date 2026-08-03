#ifndef POTENTIAL_NNUE_H
#define POTENTIAL_NNUE_H

#include <stdbool.h>
#include "structs.h"
#include "board_constants.h"

extern bool is_nnue_loaded;
extern const int white_king_bucket_layout[64];
extern const int black_king_bucket_layout[64];

bool nnue_load(const char* file_path);
int nnue_evaluate_pos(board *pos);
void test_nnue_indicies(board *pos);

void nnue_add_feature(board *pos, int piece, int square);
void nnue_remove_feature(board *pos, int piece, int square);
void nnue_refresh_accumulator(board *pos);

#endif //POTENTIAL_NNUE_H
