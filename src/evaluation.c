//
// Created by erena on 29.05.2024.
//

#include "evaluation.h"
#include "bit_manipulation.h"

// get game phase score
static inline int get_game_phase_score(board* position) {
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

// position evaluation
static inline int evaluate(board* position) {
    // get game phase score
    int game_phase_score = get_game_phase_score(position);

    // game phase (opening, middle game, endgame)
    int game_phase = -1;

    // pick up game phase based on game phase score
    if (game_phase_score > opening_phase_score) game_phase = opening;
    else if (game_phase_score < endgame_phase_score) game_phase = endgame;
    else game_phase = middlegame;

    position->gamePhase = game_phase;


    // static evaluation score
    int score = 0;

    // current pieces bitboard copy
    U64 bitboard;

    // init piece & square
    int piece, square;

    // penalties
    int double_pawns = 0;

    // loop over piece bitboards
    for (int bb_piece = P; bb_piece <= k; bb_piece++) {
        // init piece bitboard copy
        bitboard = position->bitboards[bb_piece];

        // loop over pieces within a bitboard
        while (bitboard) {
            // init piece
            piece = bb_piece;

            // init square
            square = getLS1BIndex(bitboard);

            /*
                Now in order to calculate interpolated score
                for a given game phase we use this formula
                (same for material and positional scores):

                (
                  score_opening * game_phase_score +
                  score_endgame * (opening_phase_score - game_phase_score)
                ) / opening_phase_score

                E.g. the score for pawn on d4 at phase say 5000 would be
                interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
            */

            // interpolate scores in middle_game
            if (game_phase == middlegame)
                score += (
                                 material_score[opening][piece] * game_phase_score +
                                 material_score[endgame][piece] * (opening_phase_score - game_phase_score)
                         ) / opening_phase_score;

                // score material weights with pure scores in opening or endgame
            else score += material_score[game_phase][piece];

            // score positional piece scores
            switch (piece) {
                // evaluate white pawns
                case P:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][PAWN][square] * game_phase_score +
                                         positional_score[endgame][PAWN][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][PAWN][square];

                    // double pawn penalty
                    /*double_pawns = countBits(position->bitboards[P] & fileMasks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1) {
                        if (game_phase == opening) {
                            score += double_pawns * double_pawn_penalty_opening;
                        } else if (game_phase == endgame) {
                            score += double_pawns * double_pawn_penalty_endgame;
                        }

                    }*/

                    /*
                    // on isolated pawn
                    if ((bitboards[P] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score += isolated_pawn_penalty;
                    */
                    // on passed pawn
                    /*if ((whitePassedMasks[square] & position->bitboards[p]) == 0) {
                        // give passed pawn bonus
                        if (game_phase == middlegame) {
                            score += passed_pawn_bonus_middle[get_rank[square]];
                        } else if (game_phase == endgame) {
                            score += passed_pawn_bonus_endgame[get_rank[square]];
                        }
                    }*/


                    break;

                    // evaluate white knights
                case N:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][KNIGHT][square] * game_phase_score +
                                         positional_score[endgame][KNIGHT][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][KNIGHT][square];

                    break;

                    // evaluate white bishops
                case B:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][BISHOP][square] * game_phase_score +
                                         positional_score[endgame][BISHOP][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][BISHOP][square];

                    // mobility
                    //score += count_bits(get_bishop_attacks(square, occupancies[both]));

                    break;

                    // evaluate white rooks
                case R:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][ROOK][square] * game_phase_score +
                                         positional_score[endgame][ROOK][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][ROOK][square];

                    /* semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += open_file_score;
                    */
                    break;

                    // evaluate white queens
                case Q:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][QUEEN][square] * game_phase_score +
                                         positional_score[endgame][QUEEN][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][QUEEN][square];

                    // mobility
                    //score += count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate white king
                case K:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][KING][square] * game_phase_score +
                                         positional_score[endgame][KING][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][KING][square];

                    /* semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= open_file_score;
                    */
                    // king safety bonus
                    //score += countBits(kingAttacks[square] & position->occupancies[white]) * king_shield_bonus;

                    break;

                    // evaluate black pawns
                case p:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][PAWN][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][PAWN][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][PAWN][mirrorScore[square]];

                    // double pawn penalty
                    /*double_pawns = countBits(position->bitboards[p] & fileMasks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1) {
                        if (game_phase == opening) {
                            score += double_pawns * double_pawn_penalty_opening;
                        } else if (game_phase == endgame) {
                            score += double_pawns * double_pawn_penalty_endgame;
                        }

                    }*/

                    // on isolated pawnd
                    /*if ((bitboards[p] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score -= isolated_pawn_penalty;
                    */
                    // on passed pawn
                    /*if ((blackPassedMasks[square] & position->bitboards[P]) == 0) {
                        // give passed pawn bonus
                        if (game_phase == middlegame) {
                            score += passed_pawn_bonus_middle[get_rank[square]];
                        } else if (game_phase == endgame) {
                            score += passed_pawn_bonus_endgame[get_rank[square]];
                        }
                    }*/


                    break;

                    // evaluate black knights
                case n:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][KNIGHT][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][KNIGHT][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][KNIGHT][mirrorScore[square]];

                    break;

                    // evaluate black bishops
                case b:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][BISHOP][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][BISHOP][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][BISHOP][mirrorScore[square]];

                    // mobility
                    //score -= count_bits(get_bishop_attacks(square, occupancies[both]));
                    break;

                    // evaluate black rooks
                case r:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][ROOK][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][ROOK][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][ROOK][mirrorScore[square]];

                    /* semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= open_file_score;
                    */
                    break;

                    // evaluate black queens
                case q:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][QUEEN][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][QUEEN][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][QUEEN][mirrorScore[square]];

                    // mobility
                    //score -= count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate black king
                case k:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][KING][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][KING][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][KING][mirrorScore[square]];

                    /* semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += open_file_score;
                    */
                    // king safety bonus
                    //score -= countBits(kingAttacks[square] & position->occupancies[black]) * king_shield_bonus;

                    break;
            }

            // pop ls1b
            popBit(bitboard, square);
        }
    }

    // return final evaluation based on side
    return (position->side == white) ? score : -score;
}

