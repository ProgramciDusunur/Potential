//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_EVALUATION_H
#define POTENTIAL_EVALUATION_H

#pragma once

#include "bit_manipulation.h"
#include "mask.h"
#include "move.h"
#include "values.h"
#include <stdbool.h>


/**********************************\
 ==================================

             Evaluation

 ==================================
\**********************************/


/*
    ♙ =   100   = ♙
    ♘ =   300   = ♙ * 3
    ♗ =   350   = ♙ * 3 + ♙ * 0.5
    ♖ =   500   = ♙ * 5
    ♕ =   1000  = ♙ * 10
    ♔ =   10000 = ♙ * 100

*/


// game phases
enum {
    opening, endgame, middlegame
};

// piece types
enum {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

// Mirror Score Array
extern const int mirrorScore[128];

// Material Scores
extern const int material_score[2][12];

// SEE Material Array
extern const int seeMaterial[12];

// Positional Piece Scores
extern const int positional_score[2][6][64];

// Pawn Penalties and Bonuses
extern const int double_pawn_penalty_opening;
extern const int double_pawn_penalty_endgame;
extern const int isolated_pawn_penalty_midgame;
extern const int isolated_pawn_penalty_endgame;
extern const int passed_pawn_bonus_middle[64];
extern const int passed_pawn_bonus_endgame[64];

// File and Mobility Scores
extern const int semi_open_file_score;
extern const int open_file_score;
extern const int king_semi_open_file_score;
extern const int king_open_file_score;
extern const int rook_file_score;
extern const int bishop_unit;
extern const int queen_unit;

// Mobility Bonuses
extern const int bishop_mobility_middlegame;
extern const int bishop_mobility_endgame;
extern const int queen_mobility_middlegame;
extern const int queen_mobility_endgame;

// King's Bonuses
extern const int king_shield_bonus_middlegame ;
extern const int king_shield_bonus_endgame;
extern const int king_distance_bonus;

// Game Phase Scores
extern const int opening_phase_score;
extern const int endgame_phase_score;

// Knight Outpost Bonus
extern const int knightOutpost[2][64];

// Pawn Hole Bonus
extern const int pawnHoleBonus[64];
extern const bool pawnHoleSquareCheck[64];

// Passed Can Move Bonus
extern const int passedCanMoveBonus;

// Bishop Pair Bonus
extern const int bishop_pair_bonus_midgame;
extern const int bishop_pair_bonus_endgame;
extern const int bishop_pair_bonus[];

extern int mg_table[12][64]; // [piece][square] -> midgame score
extern int eg_table[12][64]; // [piece][square] -> endgame score


int get_game_phase_score(const board* position);
void init_tables();
int evaluate(const board* position);
void clearStaticEvaluationHistory(board* position);

#endif //POTENTIAL_EVALUATION_H
