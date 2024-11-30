//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_TABLE_H
#define POTENTIAL_TABLE_H

#pragma once

#include "bit_manipulation.h"
#include "structs.h"
#include "board_constants.h"
#include <stdlib.h>
#include "magic.h"

// no hash entry found constant
#define noHashEntry 100000

#define hashFlagNone (-1)
#define hashFlagExact 0
#define hashFlagAlpha 1
#define hashFlagBeta  2


// random side key
extern U64 sideKey;
extern U64 hash_entries;
extern tt *hashTable;


U64 generateHashKey(board* position);
void writeHashEntry(int score, int bestMove, int depth, int hashFlag, board* position);
int readHashEntry(int alpha, int beta, int *bestMove, int depth, board* position);
uint64_t getHasIndex(uint64_t hash);
int readHashFlag(board* position);
void clearHashTable(void);
void init_hash_table(int mb);
void initRandomKeys(void);






#endif //POTENTIAL_TABLE_H

