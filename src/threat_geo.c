#include "threat_geo.h"
#include <stdio.h>
#include "bit_manipulation.h"
#include "move.h"

int pawn_geo[64][64];
int knight_geo[64][64];
int bishop_geo[64][64];
int rook_geo[64][64];
int queen_geo[64][64];

const int8_t ti_target_ids[5][12] = {
    // Pawn
    { 0,  1, -1,  2, -1, -1,   3,  4, -1,  5, -1, -1},
    // Knight
    { 0,  1,  2,  3,  4, -1,   5,  6,  7,  8,  9, -1},
    // Bishop
    { 0,  1,  2,  3, -1, -1,   4,  5,  6,  7, -1, -1},
    // Rook
    { 0,  1,  2,  3, -1, -1,   4,  5,  6,  7, -1, -1},
    // Queen
    { 0,  1,  2,  3,  4, -1,   5,  6,  7,  8,  9, -1}
};

const int ti_max_geo[5] = { 84, 336, 560, 896, 1456 };

const int ti_type_offset[5] = {
    TI_OFFSET_WHITE_PAWN,
    TI_OFFSET_WHITE_KNIGHT,
    TI_OFFSET_WHITE_BISHOP,
    TI_OFFSET_WHITE_ROOK,
    TI_OFFSET_WHITE_QUEEN
};

const int (*const ti_geo[5])[64] = {
    pawn_geo, knight_geo, bishop_geo, rook_geo, queen_geo
};



void init_pawn_geo() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            pawn_geo[i][j] = -1;
        }
    }

    for (int square = 8; square <= 55; square++) {
        int file = square % 8;
        int rank_id = (square / 8) - 1;
        
        if (file > 0) {
            int geo = (rank_id * 14) + (2 * file) - 1;
            pawn_geo[square ^ 56][(square + 7) ^ 56] = geo;
            pawn_geo[square ^ 56][(square - 9) ^ 56] = geo;
        }
        if (file < 7) {
            int geo = (rank_id * 14) + (2 * file);
            pawn_geo[square ^ 56][(square + 9) ^ 56] = geo;
            pawn_geo[square ^ 56][(square - 7) ^ 56] = geo;
        }
    }
}

void init_knight_geo() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            knight_geo[i][j] = -1;
        }
    }

    int count = 0;
    for (int sq_a1 = 0; sq_a1 < 64; sq_a1++) {
        int sq_a8 = sq_a1 ^ 56;
        uint64_t attacks = knightAttacks[sq_a8];
        
        for (int dest_a1 = 0; dest_a1 < 64; dest_a1++) {
            int dest_a8 = dest_a1 ^ 56;
            if (getBit(attacks, dest_a8)) {
                knight_geo[sq_a8][dest_a8] = count++;
            }
        }
    }
}

void init_bishop_geo() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            bishop_geo[i][j] = -1;
        }
    }

    int count = 0;
    for (int sq_a1 = 0; sq_a1 < 64; sq_a1++) {
        int sq_a8 = sq_a1 ^ 56;
        uint64_t attacks = getBishopAttacks(sq_a8, 0ULL); // Empty board
        
        for (int dest_a1 = 0; dest_a1 < 64; dest_a1++) {
            int dest_a8 = dest_a1 ^ 56;
            if (getBit(attacks, dest_a8)) {
                bishop_geo[sq_a8][dest_a8] = count++;
            }
        }
    }
}

void init_rook_geo() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            rook_geo[i][j] = -1;
        }
    }

    int count = 0;
    for (int sq_a1 = 0; sq_a1 < 64; sq_a1++) {
        int sq_a8 = sq_a1 ^ 56;
        uint64_t attacks = getRookAttacks(sq_a8, 0ULL); // Empty board
        
        for (int dest_a1 = 0; dest_a1 < 64; dest_a1++) {
            int dest_a8 = dest_a1 ^ 56;
            if (getBit(attacks, dest_a8)) {
                rook_geo[sq_a8][dest_a8] = count++;
            }
        }
    }
}

void init_queen_geo() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            queen_geo[i][j] = -1;
        }
    }

    int count = 0;
    for (int sq_a1 = 0; sq_a1 < 64; sq_a1++) {
        int sq_a8 = sq_a1 ^ 56;
        uint64_t attacks = getQueenAttacks(sq_a8, 0ULL); // Empty board
        
        for (int dest_a1 = 0; dest_a1 < 64; dest_a1++) {
            int dest_a8 = dest_a1 ^ 56;
            if (getBit(attacks, dest_a8)) {
                queen_geo[sq_a8][dest_a8] = count++;
            }
        }
    }
}

void init_threat_geometry() {
    init_pawn_geo();
    init_knight_geo();
    init_bishop_geo();
    init_rook_geo();
    init_queen_geo();
}