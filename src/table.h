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
#include <stdint.h>

// no hash entry found constant
#define noHashEntry 100000

#define hashFlagNone 0
#define hashFlagExact 1
#define hashFlagAlpha 2
#define hashFlagBeta  3


// random side key
extern U64 sideKey;
extern U64 hash_entries;
extern tt *hashTable;


U64 generateHashKey(board* position);
void writeHashEntry(int score, int bestMove, int depth, int hashFlag, board* position);
int readHashEntry(board *position, int *move, int16_t *tt_score,
                  uint8_t *tt_depth, uint8_t *tt_flag);
U64 generatePawnKey(board* position);
U64 generateMinorKey(board *position);
void clearHashTable(void);
void init_hash_table(int mb);
void initRandomKeys(void);






#endif //POTENTIAL_TABLE_H

