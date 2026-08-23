#include "nnue.h"
#include "simd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bit_manipulation.h"

#include "incbin.h"
#include "structs.h"
#include "utils.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

INCBIN(Net, STR(EVALFILE));

const struct Weights *const weights = (const struct Weights *) gNetData;

const int king_bucket_layout[64] = {
    3, 3, 3, 3, 3, 3, 3, 3, // Rank 8 (a8..h8 -> index 0..7)
    3, 3, 3, 3, 3, 3, 3, 3, // Rank 7 
    3, 3, 3, 3, 3, 3, 3, 3, // Rank 6 
    3, 3, 3, 3, 3, 3, 3, 3, // Rank 5 
    3, 3, 3, 3, 3, 3, 3, 3, // Rank 4 
    3, 3, 3, 3, 3, 3, 3, 3, // Rank 3 
    2, 2, 2, 2, 2, 2, 2, 2, // Rank 2 
    0, 0, 1, 1, 1, 1, 0, 0  // Rank 1 (a1..h1 -> index 56..63)
};

int king_bucket(int perspective, int square) {
    return king_bucket_layout[square ^ 0b111000 * perspective];
}

[[gnu::always_inline]]
static inline int32_t forward_screlu(const v16u *accum, const v16u *weights) {
    #define FORWARD_UNROLL 4
    v32 sums[FORWARD_UNROLL] = {};

    #pragma GCC unroll
    for (int i = 0; i < HIDDEN_VECS; i += FORWARD_UNROLL) {
        #pragma GCC unroll
        for (int j = 0; j < FORWARD_UNROLL; ++j) {
            v16 a = accum[i + j];
            v16 w = weights[i + j];
            v16 c = crelu(a);
            sums[j] += madd(c * w, c);
        }
    }

    for (int i = 1; i < FORWARD_UNROLL; ++i) {
        sums[0] += sums[i];
    }

    int32_t result = 0;
    for (size_t j = 0; j < VEC_ELEMENTS(int32_t); ++j) {
        result += sums[0][j];
    }

    return result;
}

static inline void add_weights(v16u *restrict accum, const v16u *restrict add) {
    for (int i = 0; i < HIDDEN_VECS; ++i) {
        accum[i] += add[i];
    }
}

static inline void sub_weights(v16u *restrict accum, const v16u *restrict sub) {
    for (int i = 0; i < HIDDEN_VECS; ++i) {
        accum[i] -= sub[i];
    }
}


/* FUSED UPDATES */
void get_features(board *pos, int piece, int square, const v16u **w_feat, const v16u **b_feat);

static inline void add_sub_weights(v16u *restrict accum, const v16u *restrict add, const v16u *restrict sub) {
    for (int i = 0; i < HIDDEN_VECS; ++i) {
        accum[i] += add[i] - sub[i];
    }
}

static inline void add_sub_sub_weights(v16u *restrict accum, const v16u *restrict add, const v16u *restrict sub1, const v16u *restrict sub2) {
    for (int i = 0; i < HIDDEN_VECS; ++i) {
        accum[i] += add[i] - sub1[i] - sub2[i];
    }
}

static inline void add_add_sub_sub_weights(v16u *restrict accum, const v16u *restrict add1, const v16u *restrict add2, const v16u *restrict sub1, const v16u *restrict sub2) {
    for (int i = 0; i < HIDDEN_VECS; ++i) {
        accum[i] += add1[i] + add2[i] - sub1[i] - sub2[i];
    }
}

void nnue_update_add_sub(board *pos, int add_piece, int add_sq, int sub_piece, int sub_sq) {    
    const v16u *w_add, *b_add, *w_sub, *b_sub;
    get_features(pos, add_piece, add_sq, &w_add, &b_add);
    get_features(pos, sub_piece, sub_sq, &w_sub, &b_sub);
    add_sub_weights(pos->accum_white, w_add, w_sub);
    add_sub_weights(pos->accum_black, b_add, b_sub);
}

void nnue_update_add_sub_sub(board *pos, int add_piece, int add_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq) {
    const v16u *w_add, *b_add, *w_sub1, *b_sub1, *w_sub2, *b_sub2;
    get_features(pos, add_piece, add_sq, &w_add, &b_add);
    get_features(pos, sub1_piece, sub1_sq, &w_sub1, &b_sub1);
    get_features(pos, sub2_piece, sub2_sq, &w_sub2, &b_sub2);
    add_sub_sub_weights(pos->accum_white, w_add, w_sub1, w_sub2);
    add_sub_sub_weights(pos->accum_black, b_add, b_sub1, b_sub2);
}

void nnue_update_add_add_sub_sub(board *pos, int add1_piece, int add1_sq, int add2_piece, int add2_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq) {    
    const v16u *w_add1, *b_add1, *w_add2, *b_add2, *w_sub1, *b_sub1, *w_sub2, *b_sub2;
    get_features(pos, add1_piece, add1_sq, &w_add1, &b_add1);
    get_features(pos, add2_piece, add2_sq, &w_add2, &b_add2);
    get_features(pos, sub1_piece, sub1_sq, &w_sub1, &b_sub1);
    get_features(pos, sub2_piece, sub2_sq, &w_sub2, &b_sub2);
    add_add_sub_sub_weights(pos->accum_white, w_add1, w_add2, w_sub1, w_sub2);
    add_add_sub_sub_weights(pos->accum_black, b_add1, b_add2, b_sub1, b_sub2);
}

int nnue_evaluate_pos(board *pos) {

    int32_t sum = 0;
    v16u *accum_stm  = (pos->side == white) ? pos->accum_white : pos->accum_black;
    v16u *accum_nstm = (pos->side == white) ? pos->accum_black : pos->accum_white;

    int piece_count = countBits(pos->occupancies[both]);
    int bucket = (piece_count - 2) / 4;

    sum += forward_screlu(accum_stm, weights->l1w[bucket][0]);
    sum += forward_screlu(accum_nstm, weights->l1w[bucket][1]);

    int32_t out = (sum / QA) + weights->l1b[bucket];
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

void get_features(board *pos, int piece, int square, const v16u **w_feat, const v16u **b_feat) {
    int w_king_sq = getLS1BIndex(pos->bitboards[K]);
    if (piece == K) w_king_sq = square;

    int b_king_sq = getLS1BIndex(pos->bitboards[k]);
    if (piece == k) b_king_sq = square;

    int w_sq = ((w_king_sq % 8) > 3) ? (square ^ 0b000111) : square;
    int b_sq = ((b_king_sq % 8) > 3) ? (square ^ 0b000111) : square;

    int w_bucket = king_bucket(white, w_king_sq);
    int b_bucket = king_bucket(black, b_king_sq);

    *w_feat = weights->ftw[w_bucket][piece][w_sq ^ 0b111000];
    *b_feat = weights->ftw[b_bucket][(piece+6)%12][b_sq];
}

void nnue_add_feature(board *pos, int piece, int square) {
    const v16u *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    add_weights(pos->accum_white, w_feat);
    add_weights(pos->accum_black, b_feat);
}

void nnue_remove_feature(board *pos, int piece, int square) {
    const v16u *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    sub_weights(pos->accum_white, w_feat);
    sub_weights(pos->accum_black, b_feat);
}

void nnue_add_feature_white(board *pos, int piece, int square) {    
    const v16u *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    add_weights(pos->accum_white, w_feat);
}

void nnue_add_feature_black(board *pos, int piece, int square) {    
    const v16u *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    add_weights(pos->accum_black, b_feat);
}

void nnue_sub_feature_white(board *pos, int piece, int square) {    
    const v16u *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    sub_weights(pos->accum_white, w_feat);
}

void nnue_sub_feature_black(board *pos, int piece, int square) {    
    const v16u *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    sub_weights(pos->accum_black, b_feat);
}

void nnue_refresh_accumulator(board *pos) {
    memcpy(pos->accum_white, weights->ftb, HIDDEN_SIZE * sizeof(int16_t));
    memcpy(pos->accum_black, weights->ftb, HIDDEN_SIZE * sizeof(int16_t));
    uint64_t occ = pos->occupancies[both];
    while (occ) {
        int square = getLS1BIndex(occ);
        occ &= occ - 1;

        int piece = pos->mailbox[square];
        nnue_add_feature(pos, piece, square);
    }
}

void nnue_update_finny(ThreadData *t, board *pos, int side) {
    int king_sq = (side == white) ? getLS1BIndex(pos->bitboards[K]) : getLS1BIndex(pos->bitboards[k]);

    int bucket = king_bucket(side, king_sq);
    int mirrored = (king_sq % 8) > 3 ? 1 : 0;

    FinnyEntry *entry = &t->finny_table[side][bucket][mirrored];
    memcpy(side == white ? pos->accum_white : pos->accum_black, entry->accum, HIDDEN_SIZE * sizeof(int16_t));
    
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = pos->bitboards[piece];
        if (bitboard != entry->bitboard[piece]) {
            U64 diff = bitboard ^ entry->bitboard[piece];
            while (diff) {
                int square = getLS1BIndex(diff);

                if (getBit(bitboard, square)) {
                    if (side == white) {
                        nnue_add_feature_white(pos, piece, square);
                    } else {
                        nnue_add_feature_black(pos, piece, square);
                    }
                } else {
                    if (side == white) {
                        nnue_sub_feature_white(pos, piece, square);
                    } else {
                        nnue_sub_feature_black(pos, piece, square);
                    }
                }

                diff &= diff - 1;
            }
            entry->bitboard[piece] = bitboard;
        }
    }

    memcpy(entry->accum, side == white ? pos->accum_white : pos->accum_black, HIDDEN_SIZE * sizeof(int16_t));
    memcpy(entry->bitboard, pos->bitboards, sizeof(uint64_t) * 12);
}

void reset_finny_table(void) {
    for (int i = 0; i < thread_pool.thread_count; i++) {
        for (int side = 0; side < 2; side++) {
            for (int bucket = 0; bucket < 4; bucket++) {
                for (int mirrored = 0; mirrored < 2; mirrored++) {
                    FinnyEntry *entry = &thread_pool.threads[i]->finny_table[side][bucket][mirrored];
                    memset(entry->bitboard, 0, sizeof(uint64_t) * 12);
                    memcpy(entry->accum, weights->ftb, sizeof(int16_t) * HIDDEN_SIZE);
                }
            }
        }   
    }    
}
