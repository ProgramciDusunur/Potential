//
// Created by erena on 13.09.2024.
//

#include "evaluation.h"

#include "utils.h"


// Mirror Score Array
const int mirrorScore[128] =
        {
                a1, b1, c1, d1, e1, f1, g1, h1,
                a2, b2, c2, d2, e2, f2, g2, h2,
                a3, b3, c3, d3, e3, f3, g3, h3,
                a4, b4, c4, d4, e4, f4, g4, h4,
                a5, b5, c5, d5, e5, f5, g5, h5,
                a6, b6, c6, d6, e6, f6, g6, h6,
                a7, b7, c7, d7, e7, f7, g7, h7,
                a8, b8, c8, d8, e8, f8, g8, h8
        };


// Piece scores for incremental evaluation
const int piece_scores[13] = {0, 337, 365, 477, 1025, 0, 0, -337, -365, -477, -1025, 0, 0};

// SEE Material Array
const int seeMaterial[12] = {100, 300, 300, 500, 900, 12000, -100, -300, -300, -500, -900, -12000};

// Tuned Material scores — paste into evaluation.c
const int material_score[2][12] = {
    { 8, -4, -9, 36, 25, 0, -8, 4, 9, -36, -25, 0 },
    { -10, -35, -14, -40, -95, 0, 10, 35, 14, 40, 95, 0 }
};

// Tuned PSQT tables — paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            20, 20, 20, 20, 20, 20, 20, 20,
            66, 154, 18, 55, -5, 120, 49, -44,
            -37, -39, -64, -8, -20, -23, 23, -71,
            -20, -13, -33, -15, -14, -11, -2, -10,
            -17, -27, -21, -19, -25, -27, -3, -29,
            -15, -25, -29, -37, -34, -23, -16, -26,
            -14, -21, 6, -7, 17, -2, -9, -22,
            20, 20, 20, 20, 20, 20, 20, 20,
        },
        {   // Knight
            -147, -31, 35, -49, 119, -39, 43, -49,
            -15, 17, 124, 88, 30, -20, 21, 41,
            11, 71, -43, -4, -16, 49, 47, 102,
            27, -5, 5, -6, -29, 13, -14, 32,
            -39, 62, -21, -18, -14, -26, 12, -48,
            -8, -8, -1, -14, -11, -3, -16, -35,
            -7, -24, -40, -20, -23, -16, 11, -26,
            -47, 22, -27, -9, -37, -2, -7, 15,
        },
        {   // Bishop
            -157, 63, 8, 22, 34, 17, 66, -169,
            33, -2, 41, 45, 37, 40, 18, 12,
            -29, 74, -30, 18, 30, -27, -35, -29,
            -34, -22, 34, 10, -37, 34, -18, 30,
            10, 11, -11, -31, -26, -24, -6, -30,
            0, 8, -25, -17, -18, -5, -14, -4,
            5, -11, 35, -24, -13, 58, 1, 13,
            21, -2, -23, 3, 0, 20, 38, 13,
        },
        {   // Rook
            43, 53, -29, 62, 36, 20, 42, 54,
            -10, 43, 44, 46, -12, 78, 37, 55,
            6, 30, 20, 47, 16, 25, 72, 27,
            -13, 0, 18, 37, 35, 46, -7, -9,
            -25, -42, -1, 55, 14, -34, 19, -70,
            -42, -40, -16, -21, -25, -33, -14, -54,
            -37, -64, -10, -27, -31, -33, -55, -18,
            -5, -37, -18, -39, -49, -35, -35, -35,
        },
        {   // Queen
            -148, 193, 189, 211, 101, 134, 85, -74,
            96, 98, 14, -8, 98, 51, -43, -88,
            83, 94, 48, 51, 107, -64, -9, 80,
            -36, -25, 15, -23, -38, 26, -25, -29,
            2, -48, -21, -51, -37, -33, -28, -62,
            -81, -7, -37, -47, -54, -29, -58, -93,
            -31, -41, -19, -33, -45, -17, -84, -76,
            44, -108, 47, 70, -39, -75, -71, -58,
        },
        {   // King
            -52, 36, 29, -2, -43, -21, 15, 26,
            42, 12, -7, 6, 5, 9, -25, -16,
            4, 37, 15, -3, -7, 19, 35, -9,
            -4, -7, 1, -14, -17, -12, -1, -23,
            -36, 12, -14, -26, -33, -31, -20, -38,
            -1, 50, -20, -33, -18, 2, 15, -14,
            14, -2, -3, -10, -5, 8, 27, 17,
            -2, 34, -6, -2, 16, 16, 13, 15,
        }
    },
    {   // Endgame
        {   // Pawn
            -17, -17, -17, -17, -17, -17, -17, -17,
            16, 15, 62, 52, 31, 7, 2, 40,
            1, 37, 49, 10, 14, 22, 9, 46,
            -5, 7, 39, 3, -17, -6, -1, -12,
            10, -1, 15, -11, 24, 16, -51, 12,
            6, 18, 20, 38, 24, 6, 11, 5,
            7, 19, -56, -26, -211, -44, -3, 11,
            -17, -17, -17, -17, -17, -17, -17, -17,
        },
        {   // Knight
            -41, -21, 4, -11, -14, -10, -46, -82,
            -8, 9, -8, 15, 8, -6, -7, -35,
            -7, -31, 27, 26, 16, -44, -2, -24,
            0, 20, 39, 39, 44, 22, 27, -1,
            -1, -35, 33, 42, 33, 34, 21, -1,
            -15, 14, 16, 10, 27, 14, -3, -97,
            -25, -3, 7, 56, 44, -3, -6, -27,
            -12, -89, 52, 2, 31, -1, 4, -47,
        },
        {   // Bishop
            -9, -16, 24, -3, -2, -4, -12, 21,
            -3, 1, 12, -7, 2, -8, 1, -9,
            30, -3, 41, -14, -16, 11, 5, 40,
            2, 14, -27, -11, 14, -33, 6, 7,
            -1, 8, 17, 1, 14, 19, 2, -4,
            13, 3, 13, 18, 8, 8, -2, -10,
            -9, 23, -2, -27, 16, -4, -1, -22,
            -18, 20, 34, 0, -4, -175, 0, -12,
        },
        {   // Rook
            18, 13, 2, 20, 17, 15, 13, 10,
            -3, -1, -11, -42, -38, 2, -3, -4,
            7, 4, -16, -13, -41, -14, -20, 2,
            3, -8, 6, -22, -14, -12, -15, 2,
            8, 10, 13, 9, -41, -11, -3, 6,
            1, 5, -1, 2, -2, -7, -4, -11,
            30, -2, -23, -12, -36, -4, -9, 17,
            -21, 37, -15, 34, 42, 27, 43, 42,
        },
        {   // Queen
            82, -55, 15, 21, 29, 18, 30, 125,
            3, -11, 17, -26, 13, 45, 50, 20,
            0, -6, 29, -5, -29, 88, 39, 29,
            23, 42, 44, 76, 87, 66, 96, 105,
            32, 48, 59, 95, 94, 54, 93, 74,
            4, 19, 35, 64, 51, 48, 30, 25,
            -2, -3, 39, 66, 68, -3, -16, -12,
            -13, -373, -631, -866, 75, -12, -181, -21,
        },
        {   // King
            -83, -44, -27, -27, -20, 6, -5, -26,
            -21, 8, 5, 8, 8, 29, 14, 2,
            1, 8, 14, 6, 11, 23, 35, 4,
            -17, 13, 15, 18, 17, 24, 17, -6,
            -11, -13, 22, 15, 18, 14, 12, -20,
            -17, -3, 18, 30, 16, 14, -2, -14,
            -33, -4, 12, 21, 21, 16, 7, -26,
            -62, -2, 21, 29, -25, 2, 16, -52
        }
    }
};






// Pawn Penalties and Bonuses
const int double_pawn_penalty_opening = -5;
const int double_pawn_penalty_endgame = -10;
const int isolated_pawn_penalty_opening = -5;
const int isolated_pawn_penalty_endgame = -10;

// passed pawn bonus
const int passed_pawn_bonus_middle[64] = { 0, 0, 0, 0, 0, 0, 0, 0,
                                                  36, 42, 42, 42, 42, 42, 42, 36,
                                                  14, 17, 17, 17, 17, 17, 17, 14,
                                                  5, 7, 7, 7, 7, 7, 7, 5,
                                                  0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0,};

const int passed_pawn_bonus_endgame[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                  80, 85, 90, 103, 103, 90, 85, 80,
                                                  20, 27, 30, 34, 34, 30, 27, 20,
                                                  10, 12, 15, 20, 20, 15, 12, 10,
                                                  0, 10, 10, 10, 10, 10, 10, 0,
                                                  -3, 0, 0, 0, 0, 0, 0, 0,
                                                  -2, 0, 0, 0, 0, 0, -1, -2,
                                                  0, 0, 0, 0, 0, 0, 0, 0};

// Pawn Hole Bonus [square]
const int pawnHoleBonus[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 2, 2, 1, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
};

// Pawn hole knight check [square]
const bool pawnHoleSquareCheck[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
};


// Knight Evaluation
const int knightOutpost[2][64] = {
        {   0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 1, 1, 1, 1, 0, 0,
                0, 1, 3, 3, 3, 3, 1, 0,
                0, 2, 1, 6, 3, 4, 2, 0,
                0, 0, 3, 0, 3, 3, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
        },
        {   0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 3, 0, 0, 0, 0,
                0, 2, 2, 3, 6, 1, 2, 0,
                0, 1, 0, 3, 2, 0, 1, 0,
                0, 0, 1, 0, 1, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
        },
};

// File and Mobility Scores
const int semi_open_file_score = 10;
const int open_file_score = 15;
const int king_semi_open_file_score = 10;
const int king_open_file_score = 20;
const int rook_open_file = 10;
const int bishop_unit = 4;
const int queen_unit = 9;

// Mobility Bonuses
const int bishop_mobility_middlegame = 5;
const int bishop_mobility_endgame = 10;
const int queen_mobility_middlegame = 1;
const int queen_mobility_endgame = 2;

// King's Bonuses
const int king_shield_bonus_middlegame = 6;
const int king_shield_bonus_endgame = 2;
const int king_distance_bonus = 2;

// Game Phase Scores
const int opening_phase_score = 7740;
const int endgame_phase_score = 518;

// Passed Can Move Bonus
const int passedCanMoveBonus = 5;

// Bishop Pair Bonus
const int bishop_pair_bonus_midgame = 8;
const int bishop_pair_bonus_endgame = 48;

const int bishop_pair_bonus[] = {0, 8, 15, 23, 30, 38};

#include <stdint.h>

typedef int64_t Score;

#define S(mg, eg) make_score(mg, eg)

inline Score make_score(int mg, int eg) {
    return (Score)((uint64_t)eg << 32) + (int32_t)mg;
}

inline int mg_of(Score s) { 
    return (int)(int32_t)(s & 0xFFFFFFFFLL); 
}

inline int eg_of(Score s) {     
    return (int)(int32_t)((s + 0x80000000LL) >> 32); 
}

// Pre-interpolated tables
int mg_table[12][64]; // [piece][square] -> midgame score
int eg_table[12][64]; // [piece][square] -> endgame score

Score packed_table[12][64];

void init_tables() {    
    for (int piece = P; piece <= K; piece++) {
        for (int square = 0; square < 64; square++) {
            int mg = material_score[opening][piece] + positional_score[opening][piece][square];
            int eg = material_score[endgame][piece] + positional_score[endgame][piece][square];
            packed_table[piece][square] = S(mg, eg);
        }
    }
    
    for (int piece = p; piece <= k; piece++) {
        int piece_type = piece - p;
        for (int square = 0; square < 64; square++) {
            int mirrored_sq = mirrorScore[square];
            int mg = material_score[opening][piece] - positional_score[opening][piece_type][mirrored_sq];
            int eg = material_score[endgame][piece] - positional_score[endgame][piece_type][mirrored_sq];
            packed_table[piece][square] = S(mg, eg);
        }
    }
}

void get_threats(int side, board* pos) {
    uint64_t bb;

    uint64_t knightBB;
    uint64_t bishopBB;
    uint64_t rookBB;
    uint64_t queenBB;
    uint64_t pawnBB;
    uint64_t kingBB;

    // init piece bitboards
    knightBB = pos->bitboards[side == white ? N : n];
    bishopBB = pos->bitboards[side == white ? B : b];
    rookBB = pos->bitboards[side == white ? R : r];
    queenBB = pos->bitboards[side == white ? Q : q];     
    pawnBB = pos->bitboards[side == white ? P : p];
    kingBB = pos->bitboards[side == white ? K : k];

    // Calculate Knight attacks
    pos->pieceThreats.knightThreats |= knight_threats(knightBB);
    pos->pieceThreats.stmThreats[pos->side] |= pos->pieceThreats.knightThreats;

    // Calculate Bishop attacks
    while (bishopBB) {
        int bishopSquare = getLS1BIndex(bishopBB);
        pos->pieceThreats.bishopThreats |= getBishopAttacks(bishopSquare, pos->occupancies[both]);
        pos->pieceThreats.stmThreats[pos->side] |= pos->pieceThreats.bishopThreats;
        popBit(bishopBB, bishopSquare);
    }

    // Calculate Rook attacks
    while (rookBB) {    
        int rookSquare = getLS1BIndex(rookBB);
        pos->pieceThreats.rookThreats |= getRookAttacks(rookSquare, pos->occupancies[both]);
        pos->pieceThreats.stmThreats[pos->side] |= pos->pieceThreats.rookThreats;
        popBit(rookBB, rookSquare);
    }

    // Calculate Pawn attacks
    pos->pieceThreats.pawnThreats |= pawn_threats(pawnBB, side);
    pos->pieceThreats.stmThreats[pos->side] |= pos->pieceThreats.pawnThreats;
    
    // Calculate Queen attacks
    while (queenBB) {
        int queenSquare = getLS1BIndex(queenBB);
        pos->pieceThreats.queenThreats |= getQueenAttacks(queenSquare, pos->occupancies[both]);
        pos->pieceThreats.stmThreats[pos->side] |= pos->pieceThreats.queenThreats;
        popBit(queenBB, queenSquare);
    }
    
    // Calculate King attacks
    pos->pieceThreats.kingThreats |= getKingAttacks(getLS1BIndex(kingBB));
    pos->pieceThreats.stmThreats[pos->side] |= pos->pieceThreats.kingThreats;

}

bool is_square_threatened(board *pos, int square) {
    return (pos->pieceThreats.stmThreats[!pos->side] & (1ULL << square)) != 0;
}

// get game phase score
int get_game_phase_score(const board* position) {
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

int get_piece_phase_score(uint8_t piece) {    
    int val = piece_scores[piece];
    return val < 0 ? -val : val;
}


int evaluate(board* position) {
    const int game_phase_score = position->phase_score;
    Score score = S(0, 0);
    
    position->pieceThreats.pawnThreats = 0;
    position->pieceThreats.knightThreats = 0;
    position->pieceThreats.bishopThreats = 0;
    position->pieceThreats.rookThreats = 0;
    position->pieceThreats.queenThreats = 0;
    position->pieceThreats.kingThreats = 0;
    position->pieceThreats.stmThreats[white] = 0;
    position->pieceThreats.stmThreats[black] = 0;

    get_threats(position->side, position);

    const int whiteKingSquare = getLS1BIndex(position->bitboards[K]);
    const int blackKingSquare = getLS1BIndex(position->bitboards[k]);
    int passed_pawn_count = 0;
    
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = position->bitboards[piece];
        while (bitboard) {
            const int square = getLS1BIndex(bitboard);
            score += packed_table[piece][square];

            switch (piece) {
                case P:
                    if ((whitePassedMasks[square] & position->bitboards[p]) == 0) {
                        passed_pawn_count++;
                        if (!(getBit(position->occupancies[both], (square - 8))))
                            score += S(passedCanMoveBonus, passedCanMoveBonus);
                    }
                    break;
                case p:
                    if ((blackPassedMasks[square] & position->bitboards[P]) == 0) {
                        passed_pawn_count--;
                        if (!(getBit(position->occupancies[both], (square + 8))))
                            score -= S(passedCanMoveBonus, passedCanMoveBonus);
                    }
                    break;
                case B: score += S(countBits(getBishopAttacks(square, position->occupancies[both])), countBits(getBishopAttacks(square, position->occupancies[both]))); break;
                case b: score -= S(countBits(getBishopAttacks(square, position->occupancies[both])), countBits(getBishopAttacks(square, position->occupancies[both]))); break;
                case R:
                    if ((position->bitboards[P] & fileMasks[square]) == 0) score += S(semi_open_file_score, semi_open_file_score);
                    if (((position->bitboards[P] | position->bitboards[p]) & fileMasks[square]) == 0) score += S(rook_open_file, rook_open_file);
                    break;
                case r:
                    if ((position->bitboards[p] & fileMasks[square]) == 0) score -= S(semi_open_file_score, semi_open_file_score);
                    if (((position->bitboards[P] | position->bitboards[p]) & fileMasks[square]) == 0) score -= S(rook_open_file, rook_open_file);
                    break;
            }
            popBit(bitboard, square);
        }
    }
    
    uint64_t bKingRing = kingAttacks[blackKingSquare];
    uint64_t wKingRing = kingAttacks[whiteKingSquare];

    int wThreats = ((position->pieceThreats.pawnThreats & bKingRing) != 0) * 3 +
                   ((position->pieceThreats.knightThreats & bKingRing) != 0) * 8 +
                   ((position->pieceThreats.bishopThreats & bKingRing) != 0) * 8 +
                   ((position->pieceThreats.rookThreats & bKingRing) != 0) * 12 +
                   ((position->pieceThreats.queenThreats & bKingRing) != 0) * 25;
    if (wThreats > 25) wThreats += (wThreats / 8 * 4);

    int bThreats = ((position->pieceThreats.pawnThreats & wKingRing) != 0) * 3 +
                   ((position->pieceThreats.knightThreats & wKingRing) != 0) * 8 +
                   ((position->pieceThreats.bishopThreats & wKingRing) != 0) * 8 +
                   ((position->pieceThreats.rookThreats & wKingRing) != 0) * 12 +
                   ((position->pieceThreats.queenThreats & wKingRing) != 0) * 25;
    if (bThreats > 25) bThreats += (bThreats / 8 * 4);

    score += (position->side == white) ? S(wThreats, wThreats) : -S(bThreats, bThreats);
    
    int w_shield = countBits(kingAttacks[whiteKingSquare] & position->occupancies[white]);
    score += S(w_shield * king_shield_bonus_middlegame, w_shield * king_shield_bonus_endgame);
    
    int b_shield = countBits(kingAttacks[blackKingSquare] & position->occupancies[black]);
    score -= S(b_shield * king_shield_bonus_middlegame, b_shield * king_shield_bonus_endgame);
    
    if ((position->bitboards[P] & fileMasks[whiteKingSquare]) == 0) score -= S(king_semi_open_file_score, king_semi_open_file_score);
    if (((position->bitboards[P] | position->bitboards[p]) & fileMasks[whiteKingSquare]) == 0) score -= S(king_open_file_score, king_open_file_score);
    if ((position->bitboards[p] & fileMasks[blackKingSquare]) == 0) score += S(king_semi_open_file_score, king_semi_open_file_score);
    if (((position->bitboards[P] | position->bitboards[p]) & fileMasks[blackKingSquare]) == 0) score += S(king_open_file_score, king_open_file_score);
    
    if (countBits(position->bitboards[B]) == 2) score += S(bishop_pair_bonus_midgame, bishop_pair_bonus_endgame);
    if (countBits(position->bitboards[b]) == 2) score -= S(bishop_pair_bonus_midgame, bishop_pair_bonus_endgame);

    // Winnable Score
    int winnable = 6 * passed_pawn_count + 8 * (countBits(position->bitboards[P]) - countBits(position->bitboards[p]));
    score += S(winnable, winnable);

    // Unpack ve Final Interpolation
    int mg = mg_of(score);
    int eg = eg_of(score);

    int final_score;
    if (game_phase_score > opening_phase_score) final_score = mg;
    else if (game_phase_score < endgame_phase_score) final_score = eg;
    else final_score = (mg * game_phase_score + eg * (opening_phase_score - game_phase_score)) / opening_phase_score;

    return (position->side == white) ? final_score : -final_score;
}


void clearStaticEvaluationHistory(SearchStack* ss) {
    for (int i = -STACK_OFFSET;i < maxPly + STACK_OFFSET;i++) {        
        (ss + i)->staticEval = noEval;
    }
}
