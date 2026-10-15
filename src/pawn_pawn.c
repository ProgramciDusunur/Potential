#include "pawn_pawn.h"
#include "bit_manipulation.h"

static uint64_t PAWN_PAWN_MASKS[8];

static inline uint64_t pp_file_bb(int file) {
    return 0x0101010101010101ULL << file;
}

void init_pawn_pawn_masks(void) {
    for (int f = 0; f < 8; f++) {
        uint64_t m = 0;
        for (int g = f - 1; g <= f + 1; g++) {
            if (g >= 0 && g <= 7)
                m |= pp_file_bb(g);
        }
        PAWN_PAWN_MASKS[f] = m;
    }
}

static inline int pawn_compressed_index(int perspective, int king_sq, int pawn_colour, int sq) {
    int flip = (perspective == white ? 56 : 0) ^ (((king_sq % 8) >= 4) ? 7 : 0);
    sq ^= flip;
    int offset = (perspective != pawn_colour) ? 48 : 0;
    return offset + (sq - 8);
}

static inline int get_pawn_pawn_index(int perspective, int king_sq, int a_colour, int a_sq, int b_colour, int b_sq) {
    int a_idx = pawn_compressed_index(perspective, king_sq, a_colour, a_sq);
    int b_idx = pawn_compressed_index(perspective, king_sq, b_colour, b_sq);
    int hi = (a_idx > b_idx) ? a_idx : b_idx;
    int lo = (a_idx > b_idx) ? b_idx : a_idx;
    return (hi * (hi - 1)) / 2 + lo;
}

void get_all_pawn_pawn_features(const board *pos, int *w_feats, int *w_count, int *b_feats, int *b_count) {
    uint64_t white_pawns = pos->bitboards[P];
    uint64_t black_pawns = pos->bitboards[p];

    int w_ksq = getLS1BIndex(pos->bitboards[K]);
    int b_ksq = getLS1BIndex(pos->bitboards[k]);

    int wc = 0, bc = 0;

    uint64_t remaining_w = white_pawns;
    while (remaining_w) {
        int a = getLS1BIndex(remaining_w);
        popBit(remaining_w, a);

        uint64_t mask = PAWN_PAWN_MASKS[a % 8];

        uint64_t ww = remaining_w & mask;
        while (ww) {
            int b = getLS1BIndex(ww);
            popBit(ww, b);

            w_feats[wc++] = get_pawn_pawn_index(white, w_ksq, white, a, white, b);
            b_feats[bc++] = get_pawn_pawn_index(black, b_ksq, white, a, white, b);
        }

        uint64_t wb = black_pawns & mask;
        while (wb) {
            int b = getLS1BIndex(wb);
            popBit(wb, b);

            w_feats[wc++] = get_pawn_pawn_index(white, w_ksq, white, a, black, b);
            b_feats[bc++] = get_pawn_pawn_index(black, b_ksq, white, a, black, b);
        }
    }

    uint64_t remaining_b = black_pawns;
    while (remaining_b) {
        int a = getLS1BIndex(remaining_b);
        popBit(remaining_b, a);

        uint64_t mask = PAWN_PAWN_MASKS[a % 8];

        uint64_t bb = remaining_b & mask;
        while (bb) {
            int b = getLS1BIndex(bb);
            popBit(bb, b);

            w_feats[wc++] = get_pawn_pawn_index(white, w_ksq, black, a, black, b);
            b_feats[bc++] = get_pawn_pawn_index(black, b_ksq, black, a, black, b);
        }
    }

    *w_count = wc;
    *b_count = bc;
}
