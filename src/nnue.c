#include "nnue.h"
#include <stdio.h>

#include "incbin.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef EVALFILE
#define EVALFILE beans.bin
#endif

INCBIN(Net, STR(EVALFILE));

#define QA 255
#define QB 64
#define SCALE 400

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
    // 768*64*2 + 64*2 + 128*2 + 1*2 = 98690 bytes
    // File size is actually 98752 bytes (contains 62 bytes of padding/header/footer)
    if (gNetSize >= 98690) {
        feature_weights = (const int16_t *)gNetData;
        feature_biases  = (const int16_t *)(gNetData + 98304);
        output_weights  = (const int16_t *)(gNetData + 98432);
        output_bias     = (const int16_t *)(gNetData + 98688);
        is_nnue_loaded = true;
        return true;
    }
    return false;
}

int nnue_evaluate_pos(board *pos) {
    if (!is_nnue_loaded) return 0;
    
    int16_t accum_white[64];
    int16_t accum_black[64];
    
    for (int i = 0; i < 64; i++) {
        accum_white[i] = feature_biases[i];
        accum_black[i] = feature_biases[i];
    }
    
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            int piece_color = (piece >= 6) ? 1 : 0;
            int piece_type  = piece % 6;
            
            int std_sq = square ^ 56;
            
            int w_idx = (piece_color * 384) + (piece_type * 64) + std_sq;
            int b_idx = ((1 - piece_color) * 384) + (piece_type * 64) + (std_sq ^ 56);
            
            for (int i = 0; i < 64; i++) {
                accum_white[i] += feature_weights[w_idx * 64 + i];
                accum_black[i] += feature_weights[b_idx * 64 + i];
            }
        }
    }
    
    int64_t sum = 0;
    int16_t *accum_stm  = (pos->side == white) ? accum_white : accum_black;
    int16_t *accum_nstm = (pos->side == white) ? accum_black : accum_white;
    
    for (int i = 0; i < 64; i++) {
        int act_stm = clamp_255(accum_stm[i]);
        act_stm *= act_stm;
        sum += act_stm * output_weights[i];
        
        int act_nstm = clamp_255(accum_nstm[i]);
        act_nstm *= act_nstm;
        sum += act_nstm * output_weights[i + 64];
    }
    
    int64_t out = (sum / QA) + output_bias[0];
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
