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

// pseudo random number state
extern unsigned int state;

extern const int castlingRights[64];


unsigned int getRandom32BitNumber();
U64 getRandom64Numbers();
U64 generateMagicNumber();
U64 findMagicNumber(int square, int relevantBits, int bishop);
U64 initMagicNumbers();


#endif //POTENTIAL_MAGIC_H
