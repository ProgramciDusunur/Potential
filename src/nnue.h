#ifndef POTENTIAL_NNUE_H
#define POTENTIAL_NNUE_H

#include <stdbool.h>
#include "structs.h"
#include "board_constants.h"
#include "threads.h"

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

void reset_finny_table(void);
void nnue_update_finny(ThreadData *t, board *pos, int side);
void reset_finny_table(void);

bool nnue_load(const char* file_path);
int nnue_evaluate_pos(board *pos);
void test_nnue_indicies(board *pos);

void nnue_add_feature(board *pos, int piece, int square);
void nnue_remove_feature(board *pos, int piece, int square);
void nnue_refresh_accumulator(board *pos);

void nnue_update_add_sub(board *pos, int add_piece, int add_sq, int sub_piece, int sub_sq);
void nnue_update_add_sub_sub(board *pos, int add_piece, int add_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq);
void nnue_update_add_add_sub_sub(board *pos, int add1_piece, int add1_sq, int add2_piece, int add2_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq);

#endif //POTENTIAL_NNUE_H
