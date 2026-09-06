#include "threat_geo.h"
#include <stdio.h>
#include "bit_manipulation.h"
#include "move.h"

int pawn_geo[64][64];
int knight_geo[64][64];
int bishop_geo[64][64];
int rook_geo[64][64];
int queen_geo[64][64];



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