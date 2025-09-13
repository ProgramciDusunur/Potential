//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_TABLE_H
#define POTENTIAL_TABLE_H

#pragma once

#include "bit_manipulation.h"
#include "structs.h"
#include "values.h"
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

int hash_full(void);
U64 generateHashKey(board* position);
uint64_t get_hash_index(uint64_t hash);
uint32_t get_hash_low_bits(uint64_t hash);
void prefetch_hash_entry(uint64_t hash_key);
void writeHashEntry(uint64_t key, int16_t score, int bestMove, uint8_t depth, uint8_t hashFlag, bool ttPv, board* position);
int readHashEntry(board *position, int *move, int16_t *tt_score,
                  uint8_t *tt_depth, uint8_t *tt_flag, bool *tt_pv);
U64 generatePawnKey(board* position);
U64 generateMinorKey(board *position);
U64 generateMajorKey(board *position);
U64 generate_white_np_hash_key(board *position);
U64 generate_black_np_hash_key(board *position);
U64 murmur_hash_3(uint64_t key);
void clearHashTable(void);
void init_hash_table(int mb);
void initRandomKeys(void);






#endif //POTENTIAL_TABLE_H

