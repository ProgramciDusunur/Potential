#include "nnue.h"
#include <stdio.h>

#define QA 255
#define QB 64
#define SCALE 400

int16_t feature_weights[768 * 64];
int16_t feature_biases[64];
int16_t output_weights[128];
int16_t output_bias[1];

int clamp_255(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

bool nnue_load(const char* file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) return false;

    size_t read_fw = fread(feature_weights, sizeof(int16_t), 768 * 64, f);
    size_t read_fb = fread(feature_biases, sizeof(int16_t), 64, f);
    size_t read_ow = fread(output_weights, sizeof(int16_t), 128, f);
    size_t read_ob = fread(output_bias, sizeof(int16_t), 1, f);

    fclose(f);

    return (read_fw == 768 * 64) && (read_fb == 64) && (read_ow == 128) && (read_ob == 1);
}

int nnue_evaluate_pos(board *pos) {
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
