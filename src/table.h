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

#define hashFlagNone (-1)
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

U64 generateHashKey(board* position);
static inline int readHashEntry(int alpha, int beta, int *bestMove, int depth, board* position);
void writeHashEntry(int score, int bestMove, int depth, int hashFlag, board* position);
int readHashFlag(board* position);
void clearHashTable();
