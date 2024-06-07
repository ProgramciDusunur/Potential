//
// Created by erena on 20.04.2024.
//
#pragma once
#ifndef CHESSENGINE_BITMANIPULATION_H
#define CHESSENGINE_BITMANIPULATION_H

#endif //CHESSENGINE_BITMANIPULATION_H

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define abs(x) ((x) < 0 ? -(x) : (x))


#define U64 unsigned long long

#define popBit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define setBit(bitboard, square) bitboard |= (1ULL << square)
#define getBit(bitboard, square) bitboard & (1ULL << square)


int countBits(U64 bitboard) {
    return __builtin_popcountll(bitboard);
}

// get least significant 1st bit index
int getLS1BIndex(U64 bitboard) {
    return __builtin_ctzll(bitboard);
}

U64 setOccupancy(int index, int bitsInMask, U64 attackMask);


void pBitboard(U64 bitboard);