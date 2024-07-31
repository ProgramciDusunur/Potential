//
// Created by erena on 20.04.2024.
//
#pragma once

#include <stdio.h>


#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define myAbs(x) ((x) < 0 ? -(x) : (x))


#define U64 unsigned long long

#define popBit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define setBit(bitboard, square) bitboard |= (1ULL << square)
#define getBit(bitboard, square) bitboard & (1ULL << square)


inline int countBits(U64 bitboard) {
    return __builtin_popcountll(bitboard);
}

// get least significant 1st bit index
inline int getLS1BIndex(U64 bitboard) {
    return __builtin_ctzll(bitboard);
}

static inline void printBitboard(U64 bitboard) {
    printf("\n");
    for (int y = 0, coordinate = 8; y < 8; y++, coordinate--) {
        printf("%d  ", coordinate);
        for (int x = 0; x < 8; x++) {
            int index = y * 8 + x;
            printf("%d ", getBit(bitboard, index) ? 1 : 0);
        }
        printf(" \n");
    }
    printf("\n   a b c d e f g h \n");
    printf("\n   Bitboard: %llu decimal\n\n", bitboard);
}

inline U64 setOccupancy(int index, int bitsInMask, U64 attackMask) {
    U64 occupancy = 0ULL;
    for (int count = 0; count < bitsInMask; count++) {
        int square = getLS1BIndex(attackMask);
        popBit(attackMask, square);
        if (index & (1 << count)) {
            occupancy |= (1ULL << square);
        }
    }
    return occupancy;
}
