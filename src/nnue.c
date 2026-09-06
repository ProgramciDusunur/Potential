#include "nnue.h"
#include "simd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bit_manipulation.h"

#include "incbin.h"
#include "structs.h"
#include "utils.h"
#include "threats.h"
#include "threat_geo.h"

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

void add_all_threat_inputs(const board *pos, v16u *acc_white, v16u *acc_black) {
    int w_ksq = getLS1BIndex(pos->bitboards[K]);
    int b_ksq = getLS1BIndex(pos->bitboards[k]);

    int w_flip_mask = ((w_ksq % 8) > 3) ? 7 : 0;
    int b_flip_mask = (((b_ksq % 8) > 3) ? 7 : 0) ^ 56;

    U64 occ = pos->occupancies[both];

    for (int piece = P; piece <= q; piece++) {
        if (piece == K || piece == k) continue;

        U64 pieces = pos->bitboards[piece];
        if (!pieces) continue;

        int base_piece = piece % 6;
        int max_geo = ti_max_geo[base_piece];
        int type_offset = ti_type_offset[base_piece];
        const int (*const geo_tab)[64] = ti_geo[base_piece];

        while (pieces) {
            int sq = getLS1BIndex(pieces);
            popBit(pieces, sq);

            U64 attacks = 0;
            switch(piece) {
                case P: attacks = getPawnAttacks(0, sq); break;
                case p: attacks = getPawnAttacks(1, sq); break;
                case N: case n: attacks = getKnightAttacks(sq); break;
                case B: case b: attacks = getBishopAttacks(sq, occ); break;
                case R: case r: attacks = getRookAttacks(sq, occ); break;
                case Q: case q: attacks = getQueenAttacks(sq, occ); break;
            }
            attacks &= occ;

            while (attacks) {
                int target_sq = getLS1BIndex(attacks);
                popBit(attacks, target_sq);

                int target_piece = pos->mailbox[target_sq];
                if (target_piece >= 12) continue;

                int w_rel_target = target_piece;
                int w_target_id = ti_target_ids[base_piece][w_rel_target];
                if (w_target_id != -1) {
                    int w_mapped_sq = sq ^ w_flip_mask;
                    int w_mapped_target = target_sq ^ w_flip_mask;
                    int is_cross_color = (w_rel_target >= 6);

                    if (!(is_cross_color && ((w_rel_target % 6) == base_piece) && ((w_mapped_target ^ 56) > (w_mapped_sq ^ 56)))) {
                        int geo = geo_tab[w_mapped_sq][w_mapped_target];
                        if (geo != -1) {
                            int offset = (piece >= 6) ? BLACK_TI_SIZE : 0;
                            int feat = offset + type_offset + (w_target_id * max_geo) + geo;
                            add_weights(acc_white, weights->ft_threat_weights[feat]);
                        }
                    }
                }

                int b_rel_target = (target_piece + 6) % 12;
                int b_target_id = ti_target_ids[base_piece][b_rel_target];
                if (b_target_id != -1) {
                    int b_mapped_sq = sq ^ b_flip_mask;
                    int b_mapped_target = target_sq ^ b_flip_mask;
                    int is_cross_color = (b_rel_target >= 6);

                    if (!(is_cross_color && ((b_rel_target % 6) == base_piece) && ((b_mapped_target ^ 56) > (b_mapped_sq ^ 56)))) {
                        int geo = geo_tab[b_mapped_sq][b_mapped_target];
                        if (geo != -1) {
                            int offset = (piece < 6) ? BLACK_TI_SIZE : 0;
                            int feat = offset + type_offset + (b_target_id * max_geo) + geo;
                            add_weights(acc_black, weights->ft_threat_weights[feat]);
                        }
                    }
                }
            }
        }
    }
}

int nnue_evaluate_pos(board *pos) {
    int32_t sum = 0;

    v16 temp_white[HIDDEN_VECS];
    v16 temp_black[HIDDEN_VECS];
    memcpy(temp_white, pos->accum_white, sizeof(temp_white));
    memcpy(temp_black, pos->accum_black, sizeof(temp_black));

    add_all_threat_inputs(pos, (v16u*)temp_white, (v16u*)temp_black);

    v16 *accum_stm  = (pos->side == white) ? temp_white : temp_black;
    v16 *accum_nstm = (pos->side == white) ? temp_black : temp_white;

    int piece_count = countBits(pos->occupancies[both]);
    int bucket = (piece_count - 2) / 4;
    if (bucket > 7) bucket = 7;

    sum += forward_screlu((v16u*)accum_stm, weights->l1w[bucket][0]);
    sum += forward_screlu((v16u*)accum_nstm, weights->l1w[bucket][1]);

    int32_t out = (sum / QA) + weights->l1b[bucket];
    int final_eval = (int)((out * SCALE) / (QA * QB));

    return final_eval;
}



void test_nnue_indicies(board *pos) {
    int w_ksq = __builtin_ctzll(pos->bitboards[5]) ^ 56;
    int b_ksq = __builtin_ctzll(pos->bitboards[11]) ^ 56;
    
    int w_mirror_mask = ((w_ksq % 8) > 3) ? 7 : 0;
    int b_mirror_mask = ((b_ksq % 8) > 3) ? 7 : 0;

    printf("White psq indices:");
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            int piece_color = (piece >= 6) ? 1 : 0;
            int piece_type  = piece % 6;
                        
            int std_sq = square ^ 56;
                        
            int mapped_sq_w = std_sq ^ w_mirror_mask;
            
            int w_idx = (piece_color * 384) + (piece_type * 64) + mapped_sq_w;
            printf(" %d", w_idx);
        }
    }
    
    printf("\nBlack psq indices:");
    for (int square = 0; square < 64; square++) {
        int piece = pos->mailbox[square];
        if (piece < 12) {
            int piece_color = (piece >= 6) ? 1 : 0;
            int piece_type  = piece % 6;
                        
            int std_sq = square ^ 56;
                        
            int mapped_sq_b = std_sq ^ 56 ^ b_mirror_mask;
            
            int b_idx = ((1 - piece_color) * 384) + (piece_type * 64) + mapped_sq_b;
            printf(" %d", b_idx);
        }
    }
    printf("\n"); 
}

void add_threat_inputs(board *pos, v16u *acc, int perspective) {
    int king_sq = getLS1BIndex(pos->bitboards[perspective == 0 ? K : k]);
    int flip_mask = (king_sq % 8 > 3) ? 7 : 0;
    if (perspective == 1) flip_mask ^= 56;

    for (int piece = P; piece <= q; piece++) {
        if (piece == K || piece == k) continue;

        U64 pieces = pos->bitboards[piece];
        while (pieces) {
            int sq = getLS1BIndex(pieces);
            popBit(pieces, sq);

            U64 attacks = 0;
            switch(piece) {
                case P: attacks = getPawnAttacks(0, sq); break;
                case p: attacks = getPawnAttacks(1, sq); break;
                case N: case n: attacks = getKnightAttacks(sq); break;
                case B: case b: attacks = getBishopAttacks(sq, pos->occupancies[both]); break;
                case R: case r: attacks = getRookAttacks(sq, pos->occupancies[both]); break;
                case Q: case q: attacks = getQueenAttacks(sq, pos->occupancies[both]); break;
            }
            attacks &= pos->occupancies[both];

            while (attacks) {
                int target_sq = getLS1BIndex(attacks);
                popBit(attacks, target_sq);

                int target_piece = pos->mailbox[target_sq];
                if (target_piece == 12) continue;

                int is_enemy = (perspective == 0) ? (piece >= 6) : (piece < 6);
                int base_piece = piece % 6;
                int offset = is_enemy ? BLACK_TI_SIZE : 0;
                int relative_target = (perspective == 0) ? target_piece : (target_piece + 6) % 12;

                int target_id = -1;
                switch(base_piece) {
                    case 0: target_id = get_white_pawn_target_id(relative_target); break;
                    case 1: target_id = get_white_knight_target_id(relative_target); break;
                    case 2: target_id = get_white_bishop_target_id(relative_target); break;
                    case 3: target_id = get_white_rook_target_id(relative_target); break;
                    case 4: target_id = get_white_queen_target_id(relative_target); break;
                }

                if (target_id != -1) {
                    int mapped_sq = sq ^ flip_mask;
                    int mapped_target = target_sq ^ flip_mask;
                    
                    int is_cross_color = (relative_target >= 6);
                    if (is_cross_color && ((relative_target % 6) == base_piece) && ((mapped_target ^ 56) > (mapped_sq ^ 56))) continue;

                    int geo = -1;
                    int type_offset = 0;
                    switch(base_piece) {
                        case 0: geo = pawn_geo[mapped_sq][mapped_target];   type_offset = TI_OFFSET_WHITE_PAWN; break;
                        case 1: geo = knight_geo[mapped_sq][mapped_target]; type_offset = TI_OFFSET_WHITE_KNIGHT; break;
                        case 2: geo = bishop_geo[mapped_sq][mapped_target]; type_offset = TI_OFFSET_WHITE_BISHOP; break;
                        case 3: geo = rook_geo[mapped_sq][mapped_target];   type_offset = TI_OFFSET_WHITE_ROOK; break;
                        case 4: geo = queen_geo[mapped_sq][mapped_target];  type_offset = TI_OFFSET_WHITE_QUEEN; break;
                    }

                    if (geo != -1) {
                        int max_geo = 0;
                        switch(base_piece) {
                            case 0: max_geo = 84; break;
                            case 1: max_geo = 336; break;
                            case 2: max_geo = 560; break;
                            case 3: max_geo = 896; break;
                            case 4: max_geo = 1456; break;
                        }
                        int feature_index = offset + type_offset + (target_id * max_geo) + geo;
                        add_weights(acc, weights->ft_threat_weights[feature_index]);
                    }
                }
            }
        }
    }
}

void sort_features(int *arr, int count) {
    for (int i = 1; i < count; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void test_threat_indices(board *pos) {
    int w_threat_list[256], b_threat_list[256];
    int w_threat_count = 0, b_threat_count = 0;

    for (int perspective = 0; perspective <= 1; perspective++) {
        int king_sq = getLS1BIndex(pos->bitboards[perspective == 0 ? K : k]);
        int flip_mask = (king_sq % 8 > 3) ? 7 : 0;
        if (perspective == 1) flip_mask ^= 56;

        for (int piece = P; piece <= q; piece++) {
            if (piece == K || piece == k) continue;

            U64 pieces = pos->bitboards[piece];
            while (pieces) {
                int sq = getLS1BIndex(pieces);
                popBit(pieces, sq);

                U64 attacks = 0;
                switch(piece) {
                    case P: attacks = getPawnAttacks(0, sq); break;
                    case p: attacks = getPawnAttacks(1, sq); break;
                    case N: case n: attacks = getKnightAttacks(sq); break;
                    case B: case b: attacks = getBishopAttacks(sq, pos->occupancies[both]); break;
                    case R: case r: attacks = getRookAttacks(sq, pos->occupancies[both]); break;
                    case Q: case q: attacks = getQueenAttacks(sq, pos->occupancies[both]); break;
                }
                attacks &= pos->occupancies[both];

                while (attacks) {
                    int target_sq = getLS1BIndex(attacks);
                    popBit(attacks, target_sq);

                    int target_piece = pos->mailbox[target_sq];
                    if (target_piece == 12) continue;

                    int is_enemy = (perspective == 0) ? (piece >= 6) : (piece < 6);
                    int base_piece = piece % 6;
                    int offset = is_enemy ? BLACK_TI_SIZE : 0;
                    int relative_target = (perspective == 0) ? target_piece : (target_piece + 6) % 12;

                    int target_id = -1;
                    switch(base_piece) {
                        case 0: target_id = get_white_pawn_target_id(relative_target); break;
                        case 1: target_id = get_white_knight_target_id(relative_target); break;
                        case 2: target_id = get_white_bishop_target_id(relative_target); break;
                        case 3: target_id = get_white_rook_target_id(relative_target); break;
                        case 4: target_id = get_white_queen_target_id(relative_target); break;
                    }

                    if (target_id != -1) {
                        int mapped_sq = sq ^ flip_mask;
                        int mapped_target = target_sq ^ flip_mask;
                        
                        int is_cross_color = (relative_target >= 6);
                        if (is_cross_color && ((relative_target % 6) == base_piece) && ((mapped_target ^ 56) > (mapped_sq ^ 56))) continue;

                        int geo = -1;
                        int max_geo = 0;
                        int type_offset = 0;
                        switch(base_piece) {
                            case 0: geo = pawn_geo[mapped_sq][mapped_target];   max_geo = 84;   type_offset = TI_OFFSET_WHITE_PAWN; break;
                            case 1: geo = knight_geo[mapped_sq][mapped_target]; max_geo = 336;  type_offset = TI_OFFSET_WHITE_KNIGHT; break;
                            case 2: geo = bishop_geo[mapped_sq][mapped_target]; max_geo = 560;  type_offset = TI_OFFSET_WHITE_BISHOP; break;
                            case 3: geo = rook_geo[mapped_sq][mapped_target];   max_geo = 896;  type_offset = TI_OFFSET_WHITE_ROOK; break;
                            case 4: geo = queen_geo[mapped_sq][mapped_target];  max_geo = 1456; type_offset = TI_OFFSET_WHITE_QUEEN; break;
                        }

                        if (geo != -1) {
                            int feature_index = offset + type_offset + (target_id * max_geo) + geo;
                            if (perspective == 0) w_threat_list[w_threat_count++] = feature_index;
                            else b_threat_list[b_threat_count++] = feature_index;
                        }
                    }
                }
            }
        }
    }

    sort_features(w_threat_list, w_threat_count);
    sort_features(b_threat_list, b_threat_count);

    printf("\nwhite threat features:");
    for(int i=0; i<w_threat_count; i++) printf(" %d", w_threat_list[i]);

    printf("\n\nblack threat features:");
    for(int i=0; i<b_threat_count; i++) printf(" %d", b_threat_list[i]);
    printf("\n\n");
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
