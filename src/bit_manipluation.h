//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_BIT_MANIPLUATION_H
#define POTENTIAL_BIT_MANIPLUATION_H

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

int countBits(U64 bitboard);
int getLS1BIndex(U64 bitboard);
void printBitboard(U64 bitboard);
U64 setOccupancy(int index, int bitsInMask, U64 attackMask);




#endif //POTENTIAL_BIT_MANIPLUATION_H
