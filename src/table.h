//
// Created by erena on 29.05.2024.
//
#pragma once

#ifndef U64
#define U64 unsigned long long
#endif

#include "board.h"
#include "board_constants.h"
#include "bit_manipulation.h"
#include <stdlib.h>

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

// number hash table entries
int hash_entries = 0;

// transposition table data structure
typedef struct {
    U64 hashKey;     // "almost" unique chess position identifier
    int depth;       // current search depth
    int flag;       // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    int score;       // score (alpha/beta/PV)
    int bestMove;
} tt;                 // transposition table (TT aka hash table)

// define TT insance
tt *hashTable = NULL;

// random side key
U64 sideKey;

// generate "almost" unique position ID aka hash key from scratch
inline U64 generateHashKey(board* position) {
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

inline void writeHashEntry(int score, int bestMove, int depth, int hashFlag, board* position) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[position->hashKey % hash_entries];

    // store score independent from the actual path
    // from root node (position) to current node (position)
    if (score < -mateScore) score -= position->ply;
    if (score > mateScore) score += position->ply;


    hashEntry->hashKey = position->hashKey;
    hashEntry->score = score;
    hashEntry->flag = hashFlag;
    hashEntry->depth = depth;
    hashEntry->bestMove = bestMove;
}

// read hash entry data
inline int readHashEntry(int alpha, int beta, int *bestMove, int depth, board* position) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[position->hashKey % hash_entries];

    // make sure we're dealing with the exact position we need
    if (hashEntry->hashKey == position->hashKey) {

        // make sure that we watch the exact depth our search is now at
        if (hashEntry->depth >= depth) {

            // extract stored score from TT entry
            int score = hashEntry->score;

            // retrieve score independent from the actual path
            // from root node (position) to current node (position)
            if (score < -mateScore) score -= position->ply;
            if (score > mateScore) score += position->ply;


            // match the exact (PV node) score
            if (hashEntry->flag == hashFlagExact) {
                // return exact (PV node) score
                return score;
            }
            // match alpha (fail-low node) score
            if ((hashEntry->flag == hashFlagAlpha) && (score <= alpha)) {
                // return alpha (fail-low node) score
                return alpha;
            }
            if ((hashEntry->flag == hashFlagBeta) && (score >= beta)) {
                // return beta (fail-high node) score
                return beta;
            }
        }
        // store best move
        *bestMove = hashEntry->bestMove;

    }
    // if hash entry doesn't exist
    return noHashEntry;
}
inline int readHashFlag(board* position) {
    int noHashFlag = hashFlagNone;

    tt *hashEntry = &hashTable[position->hashKey % hash_entries];
    if (hashEntry->hashKey == position->hashKey && (hashEntry->flag == hashFlagBeta || hashEntry->flag == hashFlagAlpha || hashEntry->flag == hashFlagExact)) {
        return hashEntry->flag;
    }

    return noHashFlag;
}

inline void clearHashTable() {
    // init hash table entry pointer
    tt *hash_entry;

    // loop over TT elements
    for (hash_entry = hashTable; hash_entry < hashTable + hash_entries; hash_entry++)
    {
        // reset TT inner fields
        hash_entry->hashKey = 0;
        hash_entry->depth = 0;
        hash_entry->flag = 0;
        hash_entry->score = 0;
        hash_entry->bestMove = 0;
    }
}

// dynamically allocate memory for hash table
static inline void init_hash_table(int mb)
{
    // init hash size
    int hash_size = 0x100000 * mb;

    // init number of hash entries
    hash_entries =  hash_size / sizeof(tt);

    // free hash table if not empty
    if (hashTable != NULL)
    {
        printf("Clearing hash memory...\n");

        // free hash table dynamic memory
        free(hashTable);
    }

    // allocate memory
    hashTable = (tt *) malloc(hash_entries * sizeof(tt));

    // if allocation has failed
    if (hashTable == NULL)
    {
        printf("Couldn't allocate memory for hash table, trying %dMB...", mb / 2);

        // try to allocate with half size
        init_hash_table(mb / 2);
    }

        // if allocation succeeded
    else
    {
        // clear hash table
        clearHashTable();

        printf("Hash table is initialied with %d entries\n", hash_entries);
    }


}
