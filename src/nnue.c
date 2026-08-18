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

const struct Weights *const weights = (const struct Weights *) gNetData;

const int king_bucket_layout[64] = {
    7, 7, 7, 7, 7, 7, 7, 7, // Rank 8 (a8..h8 -> index 0..7)
    7, 7, 7, 7, 7, 7, 7, 7, // Rank 7 
    7, 7, 7, 7, 7, 7, 7, 7, // Rank 6 
    7, 7, 7, 7, 7, 7, 7, 7, // Rank 5 
    6, 6, 6, 6, 6, 6, 6, 6, // Rank 4 
    6, 6, 6, 6, 6, 6, 6, 6, // Rank 3 
    4, 4, 5, 5, 5, 5, 4, 4, // Rank 2 
    0, 1, 2, 3, 3, 2, 1, 0  // Rank 1 (a1..h1 -> index 56..63)
};

int king_bucket(int perspective, int square) {
    return king_bucket_layout[square ^ 0b111000 * perspective];
}


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
        for (size_t j = 0; j < ELEMENTS; ++j) {
            a[j] = accum[i + j];
            w[j] = weights[i + j];
        }
        barrier();

        int16_t c[ELEMENTS];
        for (size_t j = 0; j < ELEMENTS; ++j) {
            int16_t v = a[j];
            c[j] = v < 0 ? 0 : (v > 255 ? 255 : v);
        }

        int16_t intermediate[ELEMENTS];
        for (size_t j = 0; j < ELEMENTS; ++j) {
            intermediate[j] = (int16_t)(c[j] * w[j]);
        }

        for (size_t j = 0; j < ELEMENTS; ++j) {
            sum[j] += intermediate[j] * c[j];
        }
    }

    int32_t result = 0;
    for (size_t j = 0; j < ELEMENTS; ++j) result += sum[j];

    return result;
}

static inline void add_weights(int16_t *restrict accum, const int16_t *restrict weights) {
    for (size_t i = 0; i < HIDDEN_SIZE; i += ELEMENTS) {
        int16_t a[ELEMENTS], w[ELEMENTS];
        for (size_t j = 0; j < ELEMENTS; ++j) {
            a[j] = accum[i + j];
            w[j] = weights[i + j];
        }
        barrier();

        int16_t c[ELEMENTS];
        for (size_t j = 0; j < ELEMENTS; ++j) {
            c[j] = a[j] + w[j];
        }

        for (size_t j = 0; j < ELEMENTS; ++j) {
            accum[i + j] = c[j];
        }
    }
}

static inline void sub_weights(int16_t *restrict accum, const int16_t *restrict weights) {
    for (size_t i = 0; i < HIDDEN_SIZE; i += ELEMENTS) {
        int16_t a[ELEMENTS], w[ELEMENTS];
        for (size_t j = 0; j < ELEMENTS; ++j) {
            a[j] = accum[i + j];
            w[j] = weights[i + j];
        }
        barrier();

        int16_t c[ELEMENTS];
        for (size_t j = 0; j < ELEMENTS; ++j) {
            c[j] = a[j] - w[j];
        }

        for (size_t j = 0; j < ELEMENTS; ++j) {
            accum[i + j] = c[j];
        }
    }
}


/* FUSED UPDATES */
void get_features(board *pos, int piece, int square, const int16_t **w_feat, const int16_t **b_feat);

static inline void add_sub_weights(int16_t *restrict accum, const int16_t *restrict add, const int16_t *restrict sub) {
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        accum[i] = accum[i] + add[i] - sub[i];
    }
}

static inline void add_sub_sub_weights(int16_t *restrict accum, const int16_t *restrict add, const int16_t *restrict sub1, const int16_t *restrict sub2) {
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        accum[i] = accum[i] + add[i] - sub1[i] - sub2[i];
    }
}

static inline void add_add_sub_sub_weights(int16_t *restrict accum, const int16_t *restrict add1, const int16_t *restrict add2, const int16_t *restrict sub1, const int16_t *restrict sub2) {
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        accum[i] = accum[i] + add1[i] + add2[i] - sub1[i] - sub2[i];
    }
}

void nnue_update_add_sub(board *pos, int add_piece, int add_sq, int sub_piece, int sub_sq) {    
    const int16_t *w_add, *b_add, *w_sub, *b_sub;
    get_features(pos, add_piece, add_sq, &w_add, &b_add);
    get_features(pos, sub_piece, sub_sq, &w_sub, &b_sub);
    add_sub_weights(pos->accum_white, w_add, w_sub);
    add_sub_weights(pos->accum_black, b_add, b_sub);
}

void nnue_update_add_sub_sub(board *pos, int add_piece, int add_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq) {
    const int16_t *w_add, *b_add, *w_sub1, *b_sub1, *w_sub2, *b_sub2;
    get_features(pos, add_piece, add_sq, &w_add, &b_add);
    get_features(pos, sub1_piece, sub1_sq, &w_sub1, &b_sub1);
    get_features(pos, sub2_piece, sub2_sq, &w_sub2, &b_sub2);
    add_sub_sub_weights(pos->accum_white, w_add, w_sub1, w_sub2);
    add_sub_sub_weights(pos->accum_black, b_add, b_sub1, b_sub2);
}

void nnue_update_add_add_sub_sub(board *pos, int add1_piece, int add1_sq, int add2_piece, int add2_sq, int sub1_piece, int sub1_sq, int sub2_piece, int sub2_sq) {    
    const int16_t *w_add1, *b_add1, *w_add2, *b_add2, *w_sub1, *b_sub1, *w_sub2, *b_sub2;
    get_features(pos, add1_piece, add1_sq, &w_add1, &b_add1);
    get_features(pos, add2_piece, add2_sq, &w_add2, &b_add2);
    get_features(pos, sub1_piece, sub1_sq, &w_sub1, &b_sub1);
    get_features(pos, sub2_piece, sub2_sq, &w_sub2, &b_sub2);
    add_add_sub_sub_weights(pos->accum_white, w_add1, w_add2, w_sub1, w_sub2);
    add_add_sub_sub_weights(pos->accum_black, b_add1, b_add2, b_sub1, b_sub2);
}

int nnue_evaluate_pos(board *pos) {

    int32_t sum = 0;
    int16_t *accum_stm  = (pos->side == white) ? pos->accum_white : pos->accum_black;
    int16_t *accum_nstm = (pos->side == white) ? pos->accum_black : pos->accum_white;

    int piece_count = countBits(pos->occupancies[both]);
    int bucket = (piece_count - 2) / 4;
    int offset = bucket * 2 * HIDDEN_SIZE;

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

void get_features(board *pos, int piece, int square, const int16_t **w_feat, const int16_t **b_feat) {
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
    const int16_t *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    add_weights(pos->accum_white, w_feat);
    add_weights(pos->accum_black, b_feat);
}

void nnue_remove_feature(board *pos, int piece, int square) {
    const int16_t *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    sub_weights(pos->accum_white, w_feat);
    sub_weights(pos->accum_black, b_feat);
}

void nnue_add_feature_white(board *pos, int piece, int square) {    
    const int16_t *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    add_weights(pos->accum_white, w_feat);
}

void nnue_add_feature_black(board *pos, int piece, int square) {    
    const int16_t *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    add_weights(pos->accum_black, b_feat);
}

void nnue_sub_feature_white(board *pos, int piece, int square) {    
    const int16_t *w_feat, *b_feat;
    get_features(pos, piece, square, &w_feat, &b_feat);
    sub_weights(pos->accum_white, w_feat);
}

void nnue_sub_feature_black(board *pos, int piece, int square) {    
    const int16_t *w_feat, *b_feat;
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
            for (int bucket = 0; bucket < INPUT_BUCKETS; bucket++) {
                for (int mirrored = 0; mirrored < 2; mirrored++) {
                    FinnyEntry *entry = &thread_pool.threads[i]->finny_table[side][bucket][mirrored];
                    memset(entry->bitboard, 0, sizeof(uint64_t) * 12);
                    memcpy(entry->accum, weights->ftb, sizeof(int16_t) * HIDDEN_SIZE);
                }
            }
        }   
    }    
}
