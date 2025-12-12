//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_MAGIC_H
#define POTENTIAL_MAGIC_H

#pragma once

#include "bit_manipulation.h"
#include "mask.h"
#include <string.h>

extern const int bishopRelevantBits[64];
extern const int rookRelevantBits[64];

// rookMagics[square]
extern U64 rookMagic[64];
// bishopMagics[square]
extern U64 bishopMagic[64];

// SplitMix64 PRNG for generating random hash keys
extern uint64_t sm64_state;
extern const int castlingRights[64];

uint64_t get_random_uint64_number(void);
U64 generateMagicNumber(void);
U64 findMagicNumber(int square, int relevantBits, int bishop);
U64 initMagicNumbers(void);


#endif //POTENTIAL_MAGIC_H
