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
extern const int isolated_pawn_penalty_opening;
extern const int isolated_pawn_penalty_endgame;
extern const int passed_pawn_bonus_middle[64];
extern const int passed_pawn_bonus_endgame[64];

// File and Mobility Scores
extern const int semi_open_file_score;
extern const int open_file_score;
extern const int rook_open_file;
extern const int bishop_unit;
extern const int queen_unit;

// Mobility Bonuses
extern const int bishop_mobility_middlegame;
extern const int bishop_mobility_endgame;
extern const int queen_mobility_middlegame;
extern const int queen_mobility_endgame;

// King's Bonuses
extern const int king_shield_bonus;
extern const int king_distance_bonus;

// Game Phase Scores
extern const int opening_phase_score;
extern const int endgame_phase_score;

// Safe Check Bonuses
extern const int queenSafeCheckBonus;
extern const int rookSafeCheckBonus;


int get_game_phase_score(board* position);
int evaluate(board* position);
void clearStaticEvaluationHistory(board* position);

#endif //POTENTIAL_EVALUATION_H
