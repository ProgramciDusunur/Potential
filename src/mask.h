//
// Created by erena on 29.05.2024.
//
#ifndef MASK_H
#define MASK_H

#pragma once
#include "board_constants.c"
#include "bit_manipulation.h"
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

void initEvaluationMasks();

U64 maskPawnAttacks(int isWhite, int square);
U64 maskKnightAttacks(int square);
U64 maskKingAttacks(int square);
U64 maskBishopAttacks(int square);
U64 bishopAttack(int square, U64 block);
U64 maskRookAttacks(int square);
U64 rookAttack(int square, U64 block);
U64 get_attackers(const board *pos, int square, int side);
U64 setFileRankMask(int file_number, int rank_number);

#endif // MASK_H
