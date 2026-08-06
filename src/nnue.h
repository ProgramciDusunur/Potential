#ifndef POTENTIAL_NNUE_H
#define POTENTIAL_NNUE_H

#include <stdbool.h>
#include "structs.h"
#include "board_constants.h"

#define QA 255
#define QB 64
#define SCALE 315

#define OUTPUT_BUCKETS 8
#define INPUT_BUCKETS 4

struct Weights {
    int16_t ftw[INPUT_BUCKETS][2][6][64][HIDDEN_SIZE];
    int16_t ftb[HIDDEN_SIZE];
    int16_t l1w[OUTPUT_BUCKETS][2][HIDDEN_SIZE];
    int16_t l1b[OUTPUT_BUCKETS];
};

int king_bucket(int perspective, int square);

int nnue_evaluate_pos(board *pos);
void test_nnue_indicies(board *pos);

void nnue_add_feature(board *pos, int piece, int square);
void nnue_remove_feature(board *pos, int piece, int square);
void nnue_refresh_accumulator(board *pos);

#endif //POTENTIAL_NNUE_H
