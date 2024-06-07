//
// Created by erena on 29.05.2024.
//
#pragma once

#include "board.h"

#ifndef CHESSENGINE_MASK_H
#define CHESSENGINE_MASK_H

#endif //CHESSENGINE_MASK_H

#ifndef U64
#define U64 unsigned long long
#endif

extern U64 maskPawnAttacks(int square, int side);
extern U64 maskPawnAttacksSEE(int square, int side);
extern U64 maskKnightAttacks(int square);
U64 maskKingAttacks(int square);
U64 maskBishopAttacks(int square);
U64 maskRookAttacks(int square);

U64 bishopAttack(int square, U64 block);
U64 rookAttack(int square, U64 block);

U64 get_attackers(const board *pos, int square);
