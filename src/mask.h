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

// file masks [square]
U64 fileMasks[64];

// rank masks [square]
U64 rankMasks[64];

// isolated pawn masks [square]
U64 isolatedMasks[64];

// white passed pawn masks [square]
U64 whitePassedMasks[64];

// black passed pawn masks [square]
U64 blackPassedMasks[64];

// double pawns penalty
const int double_pawn_penalty_opening = -5;
const int double_pawn_penalty_endgame = -10;

// isolated pawn penalty
const int isolated_pawn_penalty_opening = -5;
const int isolated_pawn_penalty_endgame = -10;

// passed pawn bonus
const int passed_pawn_bonus_middle[8] = { 0, 0, 0, 5, 10, 15, 20, 25};
const int passed_pawn_bonus_endgame[8] = { -5, 0, 0, 10, 15, 20, 35, 70};

// semi open file score
const int semi_open_file_score = 10;

// open file score
const int open_file_score = 15;

// mobility units (values from engine Fruit reloaded)
static const int bishop_unit = 4;
static const int queen_unit = 9;

// mobility bonuses (values from engine Fruit reloaded)
static const int bishop_mobility_opening = 5;
static const int bishop_mobility_endgame = 5;
static const int queen_mobility_opening = 1;
static const int queen_mobility_endgame = 2;

// king's shield bonus
const int king_shield_bonus = 5;



extern U64 maskPawnAttacks(int square, int side);
extern U64 maskKnightAttacks(int square);
U64 maskKingAttacks(int square);
U64 maskBishopAttacks(int square);
U64 maskRookAttacks(int square);

U64 bishopAttack(int square, U64 block);
U64 rookAttack(int square, U64 block);

U64 get_attackers(const board *pos, int square, int side);

void initEvaluationMasks();
