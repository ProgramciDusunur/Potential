#ifndef POTENTIAL_NNUE_H
#define POTENTIAL_NNUE_H

#include <stdbool.h>
#include "structs.h"
#include "board_constants.h"
#include "threads.h"

/* Quantization Constants */
// L0 -> L1 quantization
#define Q0 255
// L1 -> L2 quantization
#define Q1 128
// L2 -> L3 quantization
#define Q2 64

// Hidden Layer 1 Size
#define L1 64
// Hidden Layer 2 Size
#define L2 16
// Hidden Layer 3 Size
#define L3 32


#define OUTPUT_BUCKETS 1
#define INPUT_BUCKETS 1

#define SCALE 315

struct Weights {
    int16_t ftw[INPUT_BUCKETS][12][64][L1];           // (feature weights)
    int16_t ftb[L1];                                  // (feature bias)
    int8_t  l1w[2 * L1][L2];                          // (layer 1 weights)
    int32_t l1b[L2];                                  // (layer 1 bias)
    int32_t l2w[L2][L3];                              // (layer 2 weights)
    int32_t l2b[L3];                                  // (layer 2 bias)
    int32_t l3w[OUTPUT_BUCKETS][L3];                  // (layer 3 weights)
    int32_t l3b[OUTPUT_BUCKETS];                      // (layer 3 bias)
};

int king_bucket(int perspective, int square);
void get_features(board *pos, int piece, int square, const v16u **w_feat, const v16u **b_feat);

void reset_finny_table(void);
void nnue_update_finny(ThreadData *t, board *pos, int side);
void reset_finny_table(void);

int nnue_evaluate_pos(board *pos);
void test_nnue_indicies(board *pos);

void nnue_add_feature(board *pos, int piece, int square);
void nnue_remove_feature(board *pos, int piece, int square);
void nnue_refresh_accumulator(board *pos);


void nnue_update_add_sub(board *pos, int add_piece, int add_sq, int sub_piece, int sub_sq);
void nnue_update_add_sub_sub(board *pos, int add_piece, int add_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq);
void nnue_update_add_add_sub_sub(board *pos, int add1_piece, int add1_sq, int add2_piece, int add2_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq);

#endif
 //POTENTIAL_NNUE_H
