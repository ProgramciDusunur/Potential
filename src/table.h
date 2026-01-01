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
#include <stdio.h>
#include <stdlib.h>


#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
        
    #if !defined(MAP_ANONYMOUS)
        #if defined(MAP_ANON)
            #define MAP_ANONYMOUS MAP_ANON
        #else
            #define MAP_ANONYMOUS 0x20
        #endif
    #endif

    /* Fix for missing MAP_HUGETLB on older kernels */
    #ifndef MAP_HUGETLB
        #define MAP_HUGETLB 0x40000
    #endif
#endif


#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
#endif

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
void writeHashEntry(uint64_t key, int16_t score, uint16_t bestMove, uint8_t depth, uint8_t hashFlag, bool ttPv, board* position);
int readHashEntry(board *position, uint16_t *move, int16_t *tt_score,
                  uint8_t *tt_depth, uint8_t *tt_flag, bool *tt_pv);
U64 generatePawnKey(board* position);
U64 generateMinorKey(board *position);
U64 generateMajorKey(board *position);
U64 generate_white_np_hash_key(board *position);
U64 generate_black_np_hash_key(board *position);
U64 generate_krp_key(board *position);
void clearHashTable(void);
void init_hash_table(int mb);
void initRandomKeys(void);






#endif //POTENTIAL_TABLE_H

