//
// Created by erena on 29.05.2024.
//
#pragma once

#ifndef CHESSENGINE_TABLE_H
#define CHESSENGINE_TABLE_H

#endif //CHESSENGINE_TABLE_H

#ifndef U64
#define U64 unsigned long long
#endif

#include "board.h"
#include "board_constants.h"
#include "bit_manipulation.h"

/**********************************\
 ==================================

         Transposition Table

 ==================================
\**********************************/

// hash table size
#define hashSize 0x400000

// no hash entry found constant
#define noHashEntry 100000

#define hashFlagExact 0
#define hashFlagAlpha 1
#define hashFlagBeta  2

// transposition table data structure
typedef struct {
    U64 hashKey;     // "almost" unique chess position identifier
    int depth;       // current search depth
    int flag;       // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    int score;       // score (alpha/beta/PV)
    int bestMove;
} tt;                 // transposition table (TT aka hash table)

// define TT insance
tt hashTable[hashSize];

// random side key
U64 sideKey;

// generate "almost" unique position ID aka hash key from scratch
U64 generateHashKey(board* position) {
    // final hash key
    U64 finalKey = 0ULL;

    // temp piece bitboard copy
    U64 bitboard;


    // loop over piece bitboards
    for (int piece = P; piece <= k; piece++) {
        // init piece bitboard copy
        bitboard = position->bitboards[piece];

        // loop over the pieces within a bitboard
        while (bitboard) {
            // init square occupied by the piece
            int square = getLS1BIndex(bitboard);

            // hash piece
            finalKey ^= pieceKeys[piece][square];

            // pop LS1B
            popBit(bitboard, square);
        }
    }

    if (position->enpassant != no_sq) {
        // hash enpassant
        finalKey ^= enpassantKeys[position->enpassant];
    }
    // hash castling rights
    finalKey ^= castleKeys[position->castle];

    // hash the side only if black is to move
    if (position->side == black) { finalKey ^= sideKey; }

    // return generated hash key
    return finalKey;
}
