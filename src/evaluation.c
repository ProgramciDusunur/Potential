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
    { 73, 359, 406, 534, 1116, 0, -73, -359, -406, -534, -1116, 0 },
    { 107, 381, 416, 668, 1292, 0, -107, -381, -416, -668, -1292, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -75, -75, -75, -75, -75, -75, -75, -75,
            131, 66, 79, 83, 66, 19, -11, 69,
            6, 30, 39, 60, 69, 48, 39, 17,
            -4, 23, 21, 42, 48, 30, 31, -7,
            -17, 13, 11, 35, 32, 23, 20, -16,
            -17, 14, 12, 11, 20, 16, 43, -14,
            -32, 12, -1, -16, 0, 31, 43, -23,
            -75, -75, -75, -75, -75, -75, -75, -75,
        },
        {   // Knight
            -193, -191, -164, 0, 64, -303, -244, -172,
            -34, -7, 63, 15, 19, 97, 4, -28,
            -16, 84, 70, 106, 134, 111, 90, -4,
            24, 27, 61, 78, 64, 76, 32, 50,
            -3, 10, 42, 34, 46, 42, 25, 1,
            -11, 11, 20, 33, 39, 23, 26, -13,
            -18, -25, 1, 7, 8, 20, -9, -8,
            -54, -31, -32, 0, -14, -15, -22, -45,
        },
        {   // Bishop
            -67, -98, -78, -75, -17, -164, -92, -12,
            -35, 22, 11, -5, -1, -6, 31, -23,
            -23, 17, 34, 62, 57, 72, 23, 21,
            -7, 15, 34, 44, 37, 39, 16, 5,
            -11, 3, 18, 34, 34, 14, 10, -14,
            -4, 11, 19, 15, 12, 22, 10, 18,
            3, 12, 18, 0, 11, 30, 29, -8,
            -34, -4, -19, -9, -27, -18, -23, 9,
        },
        {   // Rook
            73, 78, 104, 135, 137, 135, 73, 93,
            26, 17, 54, 86, 110, 76, 21, 28,
            -16, 17, 21, 56, 58, 39, 49, 4,
            -35, -29, 2, 16, 21, 11, -7, -28,
            -58, -67, -43, -12, -18, -30, -40, -51,
            -70, -50, -54, -50, -32, -48, -31, -73,
            -68, -45, -36, -40, -27, -16, -42, -99,
            -54, -46, -35, -19, -11, -35, -65, -59,
        },
        {   // Queen
            -66, -17, 99, 60, 36, 116, -37, -31,
            -16, -16, 8, 16, 31, 23, 4, 16,
            -29, -18, -4, 22, 40, 46, 37, 28,
            -34, -22, -8, 0, 11, 7, -7, 9,
            -13, -21, -9, -9, 0, 4, -14, -13,
            -27, -3, -6, -8, -1, -2, 6, -9,
            -37, -5, 12, 0, 4, 11, 6, -34,
            -21, -27, -3, -1, -7, -25, -33, -18,
        },
        {   // King
            140, 167, 232, 136, 227, -11, -105, -207,
            295, 218, 96, 169, 32, 18, -80, 169,
            -6, 28, -30, 3, 13, -12, -2, -68,
            -88, -32, -88, -55, -36, -69, -65, -76,
            -93, -50, -110, -127, -116, -123, -71, -80,
            -35, -40, -84, -99, -100, -71, -20, -28,
            43, 11, 3, -53, -40, 14, 39, 45,
            45, 50, 30, -30, 23, 3, 74, 74,
        }
    },
    {   // Endgame
        {   // Pawn
            -176, -176, -176, -176, -176, -176, -176, -176,
            149, 197, 143, 118, 116, 145, 183, 162,
            102, 103, 79, 61, 51, 58, 84, 86,
            60, 54, 33, 22, 18, 24, 39, 50,
            41, 42, 23, 16, 18, 21, 37, 33,
            32, 40, 18, 24, 25, 22, 27, 23,
            39, 36, 32, 27, 27, 24, 26, 25,
            -176, -176, -176, -176, -176, -176, -176, -176,
        },
        {   // Knight
            -99, 42, 23, 3, -24, 138, 69, -142,
            -4, 4, -4, 72, 42, -14, -3, -12,
            3, -1, 35, 29, 13, 14, 15, -6,
            1, 24, 38, 43, 46, 33, 27, -3,
            -2, 22, 35, 42, 44, 37, 21, -12,
            -25, 3, 14, 27, 16, 11, -5, -23,
            -57, -14, -6, 8, -2, -10, -26, -65,
            -115, -57, -41, -49, -31, -26, -53, -66,
        },
        {   // Bishop
            32, 38, 23, 23, 11, 33, 20, -22,
            14, 1, 13, 14, 13, 8, -3, -3,
            0, 12, 12, 2, 9, 11, 18, -12,
            -4, 25, 17, 25, 25, 24, 22, -8,
            -18, 17, 22, 15, 20, 19, 11, -9,
            -9, 3, 13, 17, 19, 4, -4, -26,
            -39, -22, -18, -1, -12, -16, -22, -42,
            -32, -33, -40, -29, -20, -40, -56, -67,
        },
        {   // Rook
            -4, -2, -16, -31, -31, -37, -14, -13,
            20, 26, 22, 13, -6, 4, 18, 10,
            30, 18, 22, 15, 5, 17, 8, 15,
            24, 29, 30, 17, 9, 21, 10, 18,
            6, 20, 20, 11, 5, 9, 7, -5,
            -17, -1, 8, 1, -8, 7, -17, -20,
            -27, -23, -19, -10, -20, -23, -37, -24,
            -13, -4, 0, -6, -19, -13, -2, -32,
        },
        {   // Queen
            47, 44, -41, 3, 28, -31, 73, 29,
            5, 24, 49, 41, 59, 54, 40, 1,
            -6, 55, 58, 77, 58, 79, 37, 26,
            10, 39, 81, 76, 100, 75, 75, 46,
            -45, 18, 36, 85, 44, 47, 65, 17,
            -37, -41, 10, 6, 12, 8, -26, -38,
            -49, -67, -95, -47, -64, -88, -109, -56,
            -83, -114, -134, -103, -101, -115, -87, -161,
        },
        {   // King
            -249, -68, -69, -61, -60, 8, 29, -114,
            -100, -26, -2, -31, 42, 56, 77, -49,
            28, 49, 53, 39, 45, 70, 70, 42,
            21, 51, 53, 37, 45, 62, 64, 38,
            6, 32, 44, 55, 53, 52, 39, 14,
            -4, 12, 30, 42, 42, 29, 11, -7,
            -42, -20, 2, 17, 15, -5, -24, -45,
            -85, -59, -46, -39, -56, -53, -65, -99
        }
    }
};

// File and Mobility Scores
const int semi_open_file_score = 10;
const int open_file_score = 15;
const int king_semi_open_file_score = 10;
const int king_open_file_score = 20;
const int rook_open_file = 10;

// King's Bonuses
const int king_shield_bonus_middlegame = 6;
const int king_shield_bonus_endgame = 2;
const int king_distance_bonus = 2;

// Game Phase Scores
const int opening_phase_score = 7740;
const int endgame_phase_score = 518;

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
    
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = position->bitboards[piece];
        while (bitboard) {
            const int square = getLS1BIndex(bitboard);
            score += packed_table[piece][square];

            switch (piece) {
                case R:
                    if ((position->bitboards[P] & fileMasks[square]) == 0) score += S(semi_open_file_score, semi_open_file_score);                    
                    break;
                case r:
                    if ((position->bitboards[p] & fileMasks[square]) == 0) score -= S(semi_open_file_score, semi_open_file_score);                    
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
