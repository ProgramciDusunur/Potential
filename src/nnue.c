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

// Feature Weights (Layer 0) -> Layer 1 quantization clamp
static inline uint8_t screlu_255(int32_t value) {
    value = clamp(value, 0, Q0);
    return (uint8_t)((value * value) >> 8);
}

// Layer 1 -> Layer 2 quantization clamp
static inline int32_t screlu_l1(int64_t value) {
    int64_t clamped = clamp(value, 0, 16384);
    return (int32_t)((clamped * clamped) >> 16);
}

// Layer 2 -> Layer 3 quantization clamp
static inline int32_t crelu_l3(int64_t value) {
    return (int32_t)clamp(value, 0, 262144);
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

void print_features() {
    printf("feature layer bias:");

    for (int i = 0; i < 8; i++) {
        printf(" %d", weights->ftb[i]);
    }

    printf("\n");

    printf("l1 bias:");
    for (int i = 0; i < 8; i++) {
        printf(" %d", weights->l1b[i]);
    }    

    printf("\n");

    printf("l3 bias: %d\n", weights->l3b[0]);
        
    fflush(stdout);
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
    int b_king_sq = getLS1BIndex(pos->bitboards[k]);

    int w_bucket = king_bucket(white, w_king_sq);
    int w_mirrored = (w_king_sq % 8) > 3;

    int b_bucket = king_bucket(black, b_king_sq);
    int b_mirrored = (b_king_sq % 8) > 3;

    int w_sq = square ^ 56;
    if (w_mirrored) w_sq ^= 7;

    int b_sq = square;
    if (b_mirrored) b_sq ^= 7;

    *w_feat = (const v16u *) weights->ftw[w_bucket][piece][w_sq];
    *b_feat = (const v16u *) weights->ftw[b_bucket][(piece + 6) % 12][b_sq];
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

// L0 -> L1
static inline void propagate_l0_to_l1(const int16_t *stm, const int16_t *nstm, uint8_t *output) {
#if defined(USE_AVX512)
    const __m512i zero = _mm512_setzero_si512();
    const __m512i k255 = _mm512_set1_epi16(255);
    const __m512i perm_idx = _mm512_setr_epi64(0, 2, 4, 6, 1, 3, 5, 7);

    for (int i = 0; i < L1; i += 64) {
        __m512i v0 = _mm512_loadu_si512((const __m512i *)&stm[i]);
        __m512i v1 = _mm512_loadu_si512((const __m512i *)&stm[i + 32]);
        __m512i c0 = _mm512_max_epi16(_mm512_min_epi16(v0, k255), zero);
        __m512i c1 = _mm512_max_epi16(_mm512_min_epi16(v1, k255), zero);
        __m512i res0 = _mm512_srli_epi16(_mm512_mullo_epi16(c0, c0), 8);
        __m512i res1 = _mm512_srli_epi16(_mm512_mullo_epi16(c1, c1), 8);
        __m512i pack = _mm512_packus_epi16(res0, res1);
        __m512i perm = _mm512_permutexvar_epi64(perm_idx, pack);
        _mm512_storeu_si512((__m512i *)&output[i], perm);
    }

    for (int i = 0; i < L1; i += 64) {
        __m512i v0 = _mm512_loadu_si512((const __m512i *)&nstm[i]);
        __m512i v1 = _mm512_loadu_si512((const __m512i *)&nstm[i + 32]);
        __m512i c0 = _mm512_max_epi16(_mm512_min_epi16(v0, k255), zero);
        __m512i c1 = _mm512_max_epi16(_mm512_min_epi16(v1, k255), zero);
        __m512i res0 = _mm512_srli_epi16(_mm512_mullo_epi16(c0, c0), 8);
        __m512i res1 = _mm512_srli_epi16(_mm512_mullo_epi16(c1, c1), 8);
        __m512i pack = _mm512_packus_epi16(res0, res1);
        __m512i perm = _mm512_permutexvar_epi64(perm_idx, pack);
        _mm512_storeu_si512((__m512i *)&output[i + L1], perm);
    }
#elif defined(USE_AVX2)
    const __m256i zero = _mm256_setzero_si256();
    const __m256i k255 = _mm256_set1_epi16(255);

    for (int i = 0; i < L1; i += 32) {
        __m256i v0 = _mm256_loadu_si256((const __m256i *)&stm[i]);
        __m256i v1 = _mm256_loadu_si256((const __m256i *)&stm[i + 16]);
        __m256i c0 = _mm256_max_epi16(_mm256_min_epi16(v0, k255), zero);
        __m256i c1 = _mm256_max_epi16(_mm256_min_epi16(v1, k255), zero);
        __m256i res0 = _mm256_srli_epi16(_mm256_mullo_epi16(c0, c0), 8);
        __m256i res1 = _mm256_srli_epi16(_mm256_mullo_epi16(c1, c1), 8);
        __m256i pack = _mm256_packus_epi16(res0, res1);
        __m256i perm = _mm256_permute4x64_epi64(pack, 0xd8);
        _mm256_storeu_si256((__m256i *)&output[i], perm);
    }

    for (int i = 0; i < L1; i += 32) {
        __m256i v0 = _mm256_loadu_si256((const __m256i *)&nstm[i]);
        __m256i v1 = _mm256_loadu_si256((const __m256i *)&nstm[i + 16]);
        __m256i c0 = _mm256_max_epi16(_mm256_min_epi16(v0, k255), zero);
        __m256i c1 = _mm256_max_epi16(_mm256_min_epi16(v1, k255), zero);
        __m256i res0 = _mm256_srli_epi16(_mm256_mullo_epi16(c0, c0), 8);
        __m256i res1 = _mm256_srli_epi16(_mm256_mullo_epi16(c1, c1), 8);
        __m256i pack = _mm256_packus_epi16(res0, res1);
        __m256i perm = _mm256_permute4x64_epi64(pack, 0xd8);
        _mm256_storeu_si256((__m256i *)&output[i + L1], perm);
    }
#else
    for (int i = 0; i < L1; i++) {
        output[i] = screlu_255(stm[i]);
        output[i + L1] = screlu_255(nstm[i]);
    }
#endif
}

// L1 -> L2
static inline void propagate_l1_to_l2(const uint8_t *input, int out_bucket, int32_t *output) {
    int32_t sums[L2] = {0};
    int l1_offset = out_bucket * L2;

    for (int i = 0; i < 2 * L1; i++) {
        uint8_t in_val = input[i];
        if (!in_val) continue;

        const int8_t *w_row = &weights->l1w[i][l1_offset];
        for (int j = 0; j < L2; j++) {
            sums[j] += (int32_t)in_val * w_row[j];
        }
    }

    for (int j = 0; j < L2; j++) {
        int32_t sum = (sums[j] >> 1) + weights->l1b[l1_offset + j];
        output[j] = screlu_l1(sum);
    }
}

// L2 -> L3
static inline void propagate_l2_to_l3(const int32_t *input, int out_bucket, int32_t *output) {
    int64_t sums[L3];
    int l3_offset = out_bucket * L3;

    for (int i = 0; i < L3; i++) {
        sums[i] = weights->l2b[l3_offset + i];
    }

    for (int j = 0; j < L2; j++) {
        int32_t act = input[j];
        if (!act) continue;

        const int32_t *w_row = &weights->l2w[j][l3_offset];
        for (int i = 0; i < L3; i++) {
            sums[i] += (int64_t)act * w_row[i];
        }
    }

    for (int i = 0; i < L3; i++) {
        output[i] = crelu_l3(sums[i]);
    }
}

// L3 -> Output
static inline int propagate_l3_to_out(const int32_t *input, int out_bucket) {
    int64_t out_sum = weights->l3b[out_bucket];

    for (int i = 0; i < L3; i++) {
        int32_t act = input[i];
        if (!act) continue;
        out_sum += (int64_t)act * weights->l3w[i][out_bucket];
    }

    return (int)((out_sum * SCALE) / 16777216);
}

int nnue_evaluate_pos(board *pos) {
    v16u *accum_stm = (pos->side == white) ? pos->accum_white : pos->accum_black;
    v16u *accum_nstm = (pos->side == white) ? pos->accum_black : pos->accum_white;

    int piece_count = __builtin_popcountll(pos->occupancies[both]);
    int out_bucket = (piece_count - 2) / 4;
    if (out_bucket < 0) out_bucket = 0;
    if (out_bucket > OUTPUT_BUCKETS - 1) out_bucket = OUTPUT_BUCKETS - 1;

    __attribute__((aligned(64))) uint8_t l1_out[2 * L1];
    int32_t l2_out[L2];
    int32_t l3_out[L3];

    propagate_l0_to_l1((const int16_t *)accum_stm, (const int16_t *)accum_nstm, l1_out);
    propagate_l1_to_l2(l1_out, out_bucket, l2_out);
    propagate_l2_to_l3(l2_out, out_bucket, l3_out);
    return propagate_l3_to_out(l3_out, out_bucket);
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
