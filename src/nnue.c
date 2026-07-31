#include "nnue.h"
#include <stdio.h>
#include "bit_manipulation.h"

#include "incbin.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

INCBIN(Net, STR(EVALFILE));

#define QA 255
#define QB 64
#define SCALE 316

#define FT_SIZE (768 * HIDDEN_SIZE * 2)
#define FB_SIZE (HIDDEN_SIZE * 2)
#define OW_SIZE (2 * HIDDEN_SIZE * 2)
#define OB_SIZE 2
#define EXPECTED_NET_SIZE (FT_SIZE + FB_SIZE + OW_SIZE + OB_SIZE)

const int16_t *feature_weights;
const int16_t *feature_biases;
const int16_t *output_weights;
const int16_t *output_bias;

bool is_nnue_loaded = false;

int clamp_255(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
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
    if (!is_nnue_loaded) return 0;
    
    int32_t sum = 0;
    int16_t *accum_stm  = (pos->side == white) ? pos->accum_white : pos->accum_black;
    int16_t *accum_nstm = (pos->side == white) ? pos->accum_black : pos->accum_white;
    
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        int act_stm = clamp_255(accum_stm[i]);
        act_stm *= act_stm;
        sum += act_stm * output_weights[i];
        
        int act_nstm = clamp_255(accum_nstm[i]);
        act_nstm *= act_nstm;
        sum += act_nstm * output_weights[i + HIDDEN_SIZE];
    }
    
    int32_t out = (sum / QA) + output_bias[0];
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
    if (!is_nnue_loaded) return;
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
    
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        pos->accum_white[i] += feature_weights[w_idx * HIDDEN_SIZE + i];
        pos->accum_black[i] += feature_weights[b_idx * HIDDEN_SIZE + i];
    }
}

void nnue_remove_feature(board *pos, int piece, int square) {
    if (!is_nnue_loaded) return;
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
    
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        pos->accum_white[i] -= feature_weights[w_idx * HIDDEN_SIZE + i];
        pos->accum_black[i] -= feature_weights[b_idx * HIDDEN_SIZE + i];
    }
}

void nnue_refresh_accumulator(board *pos) {
    if (!is_nnue_loaded) return;
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        pos->accum_white[i] = feature_biases[i];
        pos->accum_black[i] = feature_biases[i];
    }
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            nnue_add_feature(pos, piece, square);
        }
    }
}
