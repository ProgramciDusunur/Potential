//
// Created by erena on 31.05.2024.
//
#pragma once

#include "mask.h"
#include "bit_manipulation.h"
#include <string.h>
#include "stdio.h"


static const int bishopRelevantBits[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};
static const int rookRelevantBits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12,
};

// rookMagics[square]
U64 rookMagic[64];
// bishopMagics[square]
U64 bishopMagic[64];

// pseudo random number state
static unsigned int state = 1804289383;

inline static unsigned int getRandom32BitNumber() {
    //get current state
    unsigned int number = state;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // update random number state
    state = number;

    return number;
}

inline static U64 getRandom64Numbers() {
    U64 n1, n2, n3, n4;

    n1 = (U64) (getRandom32BitNumber()) & 0xFFFF;
    n2 = (U64) (getRandom32BitNumber()) & 0xFFFF;
    n3 = (U64) (getRandom32BitNumber()) & 0xFFFF;
    n4 = (U64) (getRandom32BitNumber()) & 0xFFFF;

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

inline static U64 generateMagicNumber() {
    return getRandom64Numbers() & getRandom64Numbers() & getRandom64Numbers();
}

inline static U64 findMagicNumber(int square, int relevantBits, int bishop) {
    // init occupancies
    U64 magicOccupancies[4096];

    // init attack tables
    U64 attacks[4096];

    // init used attacks
    U64 usedAttacks[4096];

    // init attack mask for a current piece
    U64 attackMask = bishop ? maskBishopAttacks(square) : maskRookAttacks(square);

    // init occupancy indicies
    int occupancyIndicies = 1 << relevantBits;

    // loop over occupancy indicies
    for (int index = 0; index < occupancyIndicies; index++) {
        // init occupancies
        magicOccupancies[index] = setOccupancy(index, relevantBits, attackMask);

        // init attacks
        attacks[index] = bishop ? bishopAttack(square, magicOccupancies[index]) :
                         rookAttack(square, magicOccupancies[index]);
    }
    // test magic numbers loop
    for (int randomCount = 0; randomCount < 100000000; randomCount++) {

        // generate magic number candidate
        U64 magicNumber = generateMagicNumber();

        // skip inappropriate magic numbers
        if (countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6) continue;

        // init used attacks
        memset(usedAttacks, 0ULL, sizeof(usedAttacks));

        // init index & fail flag
        int index, fail;

        // test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancyIndicies; index++) {
            // init magic index
            int magicIndex = (int) ((magicOccupancies[index] * magicNumber) >> (64 - relevantBits));

            // if magic index works
            if (usedAttacks[magicIndex] == 0ULL) {
                // init used attacks
                usedAttacks[magicIndex] = attacks[index];
            } else if (usedAttacks[magicIndex] != attacks[index]) {
                // magic index doesn't work
                fail = 1;
            }
        }
        // if magic number works
        if (!fail) {
            return magicNumber;
        }
    }
    // if magic number doesn't work
    printf("  Magic number fails!\n");
    return 0ULL;
}

// init magic numbers
inline static U64 initMagicNumbers() {
    // loop over 64 board squares
    for (int square = 0; square < 64; square++) {
        // init rook magic numbers
        rookMagic[square] = findMagicNumber(square, rookRelevantBits[square], rook);
    }
    printf("\n");
    for (int square = 0; square < 64; square++) {
        // init bishop magic numbers
        bishopMagic[square] = findMagicNumber(square, bishopRelevantBits[square], bishop);
    }
    return 0ULL;
}

