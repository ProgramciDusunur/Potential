//
// Created by erena on 29.05.2024.
//
#pragma once

#include "board_constants.h"
#include "board.h"
#include "values.h"
#include "mask.h"
#include "bit_manipulation.h"
#include "move.h"

/**********************************\
 ==================================

             Evaluation

 ==================================
\**********************************/

// material score

/*
    ♙ =   100   = ♙
    ♘ =   300   = ♙ * 3
    ♗ =   350   = ♙ * 3 + ♙ * 0.5
    ♖ =   500   = ♙ * 5
    ♕ =   1000  = ♙ * 10
    ♔ =   10000 = ♙ * 100

*/

// material score [game phase][piece]
extern const int material_score[2][12];

extern const int seeMaterial[12];

// game phase scores
extern const int opening_phase_score;
extern const int endgame_phase_score;

// game phases
enum {
    opening, endgame, middlegame
};

// piece types
enum {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

// positional piece scores [game phase][piece][square]
extern const int positional_score[2][6][64];

// mirror positional score tables for opposite side
extern const int mirrorScore[128];

// double pawns penalty
extern const int double_pawn_penalty_opening;
extern const int double_pawn_penalty_endgame;

// isolated pawn penalty
extern const int isolated_pawn_penalty_opening;
extern const int isolated_pawn_penalty_endgame;

// passed pawn bonus
extern const int passed_pawn_bonus_middle[8];
extern const int passed_pawn_bonus_endgame[8];


// semi open file score
extern const int semi_open_file_score;

// open file score
extern const int open_file_score;

// mobility units (values from engine Fruit reloaded)
extern const int bishop_unit;
extern const int queen_unit;

// mobility bonuses (values from engine Fruit reloaded)
extern const int bishop_mobility_opening;
extern const int bishop_mobility_endgame;
extern const int queen_mobility_opening;
extern const int queen_mobility_endgame;

// king's shield bonus
extern const int king_shield_bonus;

// get game phase score
inline int get_game_phase_score(board* position) {
    /*
        The game phase score of the game is derived from the pieces
        (not counting pawns and kings) that are still on the board.
        The full material starting position game phase score is:

        4 * knight material score in the opening +
        4 * bishop material score in the opening +
        4 * rook material score in the opening +
        2 * queen material score in the opening
    */

    // white & black game phase scores
    int white_piece_scores = 0, black_piece_scores = 0;

    // loop over white pieces
    for (int piece = N; piece <= Q; piece++)
        white_piece_scores += countBits(position->bitboards[piece]) * material_score[opening][piece];


    // loop over white pieces
    for (int piece = n; piece <= q; piece++)
        black_piece_scores += countBits(position->bitboards[piece]) * -material_score[opening][piece];



    // return game phase score
    return white_piece_scores + black_piece_scores;
}


int evaluate(board* position);
void clearStaticEvaluationHistory(board* position);
