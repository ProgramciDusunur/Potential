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

// Tuned Material scores ÔÇö paste into evaluation.c
const int material_score[2][12] = {
    { 79, 361, 405, 540, 1105, 0, -79, -361, -405, -540, -1105, 0 },
    { 105, 385, 418, 675, 1297, 0, -105, -385, -418, -675, -1297, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -75, -75, -75, -75, -75, -75, -75, -75,
            124, 148, 53, 112, 105, 26, -2, 34,
            14, 25, 30, 58, 57, 41, 42, 14,
            -12, 20, 22, 41, 44, 25, 26, -13,
            -21, 9, 12, 36, 28, 25, 18, -16,
            -20, 11, 10, 11, 19, 15, 42, -12,
            -32, 7, 1, -19, -3, 32, 42, -23,
            -75, -75, -75, -75, -75, -75, -75, -75,
        },
        {   // Knight
            -203, -219, -144, -105, 69, -248, -134, -161,
            -52, -25, 63, 27, 39, 100, -37, -20,
            15, 74, 66, 104, 128, 87, 72, 21,
            40, 34, 54, 84, 61, 75, 36, 40,
            -6, 6, 43, 33, 45, 47, 19, 2,
            -14, 11, 21, 33, 36, 24, 22, -2,
            -20, 3, -4, 8, 6, 23, -13, -15,
            -34, -31, -36, -16, -11, -20, -23, -77,
        },
        {   // Bishop
            -65, -115, -66, -108, -114, -214, -81, -1,
            -50, 25, 14, -50, 18, -7, 26, -43,
            -23, 21, 45, 70, 47, 51, 36, 21,
            -3, 20, 38, 42, 39, 51, 25, 11,
            -3, 8, 20, 35, 32, 20, 19, -9,
            5, 19, 22, 21, 19, 23, 23, 13,
            9, 16, 26, 6, 15, 26, 34, 6,
            -23, 3, -15, -4, -15, -17, -5, 12,
        },
        {   // Rook
            56, 61, 65, 144, 141, 149, 77, 113,
            25, 27, 62, 92, 78, 96, 8, 47,
            -10, 9, 9, 59, 85, 2, 30, 19,
            -23, -31, -6, 5, 20, 6, -8, -11,
            -54, -57, -50, -16, -24, -39, -46, -47,
            -72, -55, -44, -28, -38, -56, -32, -65,
            -70, -44, -31, -26, -33, -25, -45, -99,
            -54, -42, -33, -13, -9, -32, -62, -56,
        },
        {   // Queen
            -95, -31, 37, 45, 102, 72, -3, -14,
            -22, -10, 15, -7, -16, 8, 18, 10,
            -4, -3, -9, 20, 38, 44, 26, 24,
            -19, -12, -4, 1, -5, 2, -6, 6,
            -13, -30, -7, 1, -4, 2, -10, -13,
            -24, 2, -2, -6, -6, -3, 11, -21,
            -28, 1, 10, 2, 4, 17, 16, -36,
            -7, -22, -6, 0, -9, -33, -14, 16,
        },
        {   // King
            51, 94, 311, 156, 135, 154, 89, -88,
            33, 50, 174, 147, -9, 33, 56, 172,
            -116, 9, -83, -49, -39, -72, -17, 34,
            -31, -73, -77, -127, -37, -55, -55, -42,
            -77, -37, -114, -113, -134, -68, -80, -107,
            -33, -21, -69, -84, -88, -74, -19, -43,
            61, 20, 1, -48, -32, 21, 46, 60,
            39, 56, 30, -24, 30, 7, 84, 83,
        }
    },
    {   // Endgame
        {   // Pawn
            -175, -175, -175, -175, -175, -175, -175, -175,
            161, 149, 148, 122, 91, 114, 183, 172,
            103, 110, 85, 60, 63, 55, 75, 79,
            71, 56, 32, 17, 20, 22, 37, 49,
            43, 42, 28, 16, 16, 23, 38, 34,
            37, 41, 23, 24, 26, 28, 26, 22,
            43, 39, 33, 37, 37, 25, 21, 24,
            -175, -175, -175, -175, -175, -175, -175, -175,
        },
        {   // Knight
            -83, 35, 15, 36, -13, 106, 0, -139,
            -26, 7, -8, 63, 21, -1, 28, -14,
            -15, 1, 38, 22, 11, 25, 6, -1,
            -8, 21, 33, 37, 43, 42, 28, -13,
            -13, 20, 43, 46, 38, 31, 25, 0,
            -24, -5, 11, 20, 28, 11, 2, -35,
            -73, -11, -3, 8, 5, -16, -16, -30,
            -92, -48, -24, -25, -40, -29, -44, -58,
        },
        {   // Bishop
            15, 44, 24, 30, 21, 48, 31, -67,
            6, 3, 10, 29, 2, 15, 0, -20,
            2, 27, 15, -4, 10, 21, 16, -2,
            9, 25, 13, 25, 26, 8, 11, -10,
            2, 16, 18, 23, 26, 15, 13, -26,
            -16, 0, 16, 18, 18, 13, -19, -9,
            -42, -24, -18, 0, -2, -17, -20, -33,
            -46, -71, -43, -30, -23, -29, -35, -55,
        },
        {   // Rook
            6, -1, -7, -28, -31, -34, -11, -15,
            19, 21, 21, 10, 10, -4, 22, 10,
            25, 23, 25, 7, -6, 25, 17, 4,
            20, 34, 26, 22, 13, 10, 21, 4,
            6, 22, 25, 11, 12, 16, 6, -7,
            -13, -7, 0, -8, -5, 2, -19, -23,
            -38, -18, -7, -17, -17, -21, -19, -34,
            -14, -8, -3, -10, -15, -12, -5, -35,
        },
        {   // Queen
            79, 47, 26, 8, -29, 16, 59, 23,
            -10, 9, 35, 94, 108, 72, 38, 11,
            -36, -17, 58, 89, 77, 57, 85, 26,
            4, 24, 48, 85, 121, 121, 94, 13,
            -48, 31, 33, 54, 63, 67, 34, 30,
            -51, -44, 6, 15, 14, 22, -26, -28,
            -101, -72, -84, -51, -50, -79, -120, -89,
            -118, -88, -134, -106, -83, -100, -164, -170,
        },
        {   // King
            -209, -79, -106, -72, -49, -27, -10, -91,
            -56, 20, -23, -17, 50, 41, 44, -47,
            35, 50, 68, 54, 53, 88, 75, 25,
            12, 62, 61, 61, 49, 57, 60, 23,
            9, 25, 53, 49, 62, 41, 35, 14,
            -21, 17, 28, 40, 41, 32, 8, -1,
            -55, -24, 2, 16, 15, -8, -26, -46,
            -74, -67, -47, -38, -66, -54, -68, -93
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
