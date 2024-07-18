//
// Created by erena on 31.05.2024.
//
#pragma once


#ifndef U64
#define U64 unsigned long long
#endif

extern const int bishopRelevantBits[64];
extern const int rookRelevantBits[64];

// rookMagics[square]
extern U64 rookMagic[64];
// bishopMagics[square]
extern U64 bishopMagic[64];
