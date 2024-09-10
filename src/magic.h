//
// Created by erena on 31.05.2024.
//
#pragma once

#include "mask.h"
#include "bit_manipulation.h"
#include <string.h>
#include "stdio.h"


extern const int bishopRelevantBits[64];
extern const int rookRelevantBits[64];

// rookMagics[square]
extern U64 rookMagic[64];
// bishopMagics[square]
extern U64 bishopMagic[64];

// pseudo random number state
extern unsigned int state;



