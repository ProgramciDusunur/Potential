#include "nnue.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bit_manipulation.h"

#include "incbin.h"
#include "structs.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

INCBIN(Net, STR(EVALFILE));

#define QA 255
#define QB 64
#define SCALE 283

#define OUTPUT_BUCKETS 8

#define FT_SIZE (768 * HIDDEN_SIZE * 2)
#define FB_SIZE (HIDDEN_SIZE * 2)
#define OW_SIZE (2 * HIDDEN_SIZE * 2 * OUTPUT_BUCKETS)
#define OB_SIZE (2 * OUTPUT_BUCKETS)
#define EXPECTED_NET_SIZE (FT_SIZE + FB_SIZE + OW_SIZE + OB_SIZE)

const int16_t *feature_weights;
const int16_t *feature_biases;
const int16_t *output_weights;
const int16_t *output_bias;

bool is_nnue_loaded = false;

#if defined(__AVX512F__)
#define VEC_BYTES 64
#elif defined(__AVX2__)
#define VEC_BYTES 32
#else
#define VEC_BYTES 16
#endif
#define VEC_ELEMENTS (VEC_BYTES / sizeof(int16_t))
#define ELEMENTS (VEC_ELEMENTS < HIDDEN_SIZE ? VEC_ELEMENTS : HIDDEN_SIZE)

static inline void barrier(void) { __asm__ volatile(""); }

static inline int32_t forward_screlu(const int16_t *accum, const int16_t *weights) {
    int32_t sum[ELEMENTS] = {0};

    for (size_t i = 0; i < HIDDEN_SIZE; i += ELEMENTS) {
        int16_t a[ELEMENTS], w[ELEMENTS];
        for (int j = 0; j < ELEMENTS; ++j) {
            a[j] = accum[i + j];
            w[j] = weights[i + j];
        }
        barrier();

        int16_t c[ELEMENTS];
        for (int j = 0; j < ELEMENTS; ++j) {
            int16_t v = a[j];
            c[j] = v < 0 ? 0 : (v > 255 ? 255 : v);
        }

        int16_t intermediate[ELEMENTS];
        for (int j = 0; j < ELEMENTS; ++j) {
            intermediate[j] = (int16_t)(c[j] * w[j]);
        }

        for (int j = 0; j < ELEMENTS; ++j) {
            sum[j] += intermediate[j] * c[j];
        }
    }

    int32_t result = 0;
    for (int j = 0; j < ELEMENTS; ++j) result += sum[j];

    return result;
}

static inline void add_weights(int16_t *restrict accum, const int16_t *restrict weights) {
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        accum[i] += weights[i];
    }
}

static inline void sub_weights(int16_t *restrict accum, const int16_t *restrict weights) {
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        accum[i] -= weights[i];
    }
}

bool nnue_load(const char* file_path) {
    (void)file_path; // unused parameter when embedding
    if (gNetSize >= EXPECTED_NET_SIZE) {
        feature_weights = (const int16_t *)gNetData;
        feature_biases  = (const int16_t *)(gNetData + FT_SIZE);
        output_weights  = (const int16_t *)(gNetData + FT_SIZE + FB_SIZE);
        output_bias     = (const int16_t *)(gNetData + FT_SIZE + FB_SIZE + OW_SIZE);
        is_nnue_loaded = true;
        return true;
    }
    return false;
}

int nnue_evaluate_pos(board *pos) {
    assert(is_nnue_loaded);
    
    int32_t sum = 0;
    int16_t *accum_stm  = (pos->side == white) ? pos->accum_white : pos->accum_black;
    int16_t *accum_nstm = (pos->side == white) ? pos->accum_black : pos->accum_white;

    int piece_count = countBits(pos->occupancies[both]);
    int bucket = (piece_count - 2) / 4;
    int offset = bucket * 2 * HIDDEN_SIZE;

    sum += forward_screlu(accum_stm, output_weights + offset);
    sum += forward_screlu(accum_nstm, output_weights + HIDDEN_SIZE + offset);

    int32_t out = (sum / QA) + output_bias[bucket];
    int final_eval = (int)((out * SCALE) / (QA * QB));

    return final_eval;
}

void test_nnue_indicies(board *pos) {
    printf("White indices:");
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            int piece_color = (piece >= 6) ? 1 : 0;
            int piece_type  = piece % 6;
            int std_sq = square ^ 56;
            int w_idx = (piece_color * 384) + (piece_type * 64) + std_sq;
            printf(" %d", w_idx);
        }
    }
    printf("\nBlack indices:");
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            int piece_color = (piece >= 6) ? 1 : 0;
            int piece_type  = piece % 6;
            int std_sq = square ^ 56;
            int b_idx = ((1 - piece_color) * 384) + (piece_type * 64) + (std_sq ^ 56);
            printf(" %d", b_idx);
        }
    }
    printf("\n");
}

void nnue_add_feature(board *pos, int piece, int square) {
    assert(is_nnue_loaded);
    int piece_color = (piece >= 6) ? 1 : 0;
    int piece_type  = piece % 6;
    
    int w_king_sq = (piece == K) ? square : getLS1BIndex(pos->bitboards[K]);
    int b_king_sq = (piece == k) ? square : getLS1BIndex(pos->bitboards[k]);
    
    int w_sq = ((w_king_sq % 8) > 3) ? (square ^ 7) : square;
    int b_sq = ((b_king_sq % 8) > 3) ? (square ^ 7) : square;
    
    int w_std_sq = w_sq ^ 56;
    int b_std_sq = b_sq ^ 56;
    
    int w_idx = (piece_color * 384) + (piece_type * 64) + w_std_sq;
    int b_idx = ((1 - piece_color) * 384) + (piece_type * 64) + (b_std_sq ^ 56);
    
    add_weights(pos->accum_white, feature_weights + w_idx * HIDDEN_SIZE);
    add_weights(pos->accum_black, feature_weights + b_idx * HIDDEN_SIZE);
}

void nnue_remove_feature(board *pos, int piece, int square) {
    assert(is_nnue_loaded);
    int piece_color = (piece >= 6) ? 1 : 0;
    int piece_type  = piece % 6;
    
    int w_king_sq = (piece == K) ? square : getLS1BIndex(pos->bitboards[K]);
    int b_king_sq = (piece == k) ? square : getLS1BIndex(pos->bitboards[k]);
    
    int w_sq = ((w_king_sq % 8) > 3) ? (square ^ 7) : square;
    int b_sq = ((b_king_sq % 8) > 3) ? (square ^ 7) : square;
    
    int w_std_sq = w_sq ^ 56;
    int b_std_sq = b_sq ^ 56;
    
    int w_idx = (piece_color * 384) + (piece_type * 64) + w_std_sq;
    int b_idx = ((1 - piece_color) * 384) + (piece_type * 64) + (b_std_sq ^ 56);
    
    sub_weights(pos->accum_white, feature_weights + w_idx * HIDDEN_SIZE);
    sub_weights(pos->accum_black, feature_weights + b_idx * HIDDEN_SIZE);
}

void nnue_refresh_accumulator(board *pos) {
    assert(is_nnue_loaded);
    memcpy(pos->accum_white, feature_biases, HIDDEN_SIZE * sizeof(int16_t));
    memcpy(pos->accum_black, feature_biases, HIDDEN_SIZE * sizeof(int16_t));
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            nnue_add_feature(pos, piece, square);
        }
    }
}
