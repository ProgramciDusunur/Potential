//
// Created by erena on 29.05.2024.
//
#pragma once

#include "board.h"

// file masks [square]
extern U64 fileMasks[64];

// rank masks [square]
extern U64 rankMasks[64];

// isolated pawn masks [square]
extern U64 isolatedMasks[64];

// white passed pawn masks [square]
extern U64 whitePassedMasks[64];

// black passed pawn masks [square]
extern U64 blackPassedMasks[64];


// Rook attack masks rookMask[square]
extern U64 rookMask[64];
// BishopMask[square]
extern U64 bishopMask[64];

extern U64 maskPawnAttacks(int square, int side);
extern U64 maskKnightAttacks(int square);
extern U64 maskKingAttacks(int square);
extern U64 maskBishopAttacks(int square);
extern U64 maskRookAttacks(int square);

extern U64 bishopAttack(int square, U64 block);
extern U64 rookAttack(int square, U64 block);

extern U64 get_attackers(const board *pos, int square, int side);

extern void initEvaluationMasks();
