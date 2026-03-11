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
    { 80, 373, 420, 564, 1167, 0, -80, -373, -420, -564, -1167, 0 },
    { 136, 413, 447, 742, 1398, 0, -136, -413, -447, -742, -1398, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -77, -77, -77, -77, -77, -77, -77, -77,
            104, 92, 88, 76, 69, 24, 68, 96,
            12, 50, 49, 66, 74, 23, 45, 5,
            -19, 25, 24, 41, 41, 39, 47, -16,
            -26, 9, 2, 29, 34, 24, 21, -17,
            -27, 1, 12, 10, 22, 19, 43, -14,
            -41, 4, -3, -23, -10, 32, 43, -31,
            -77, -77, -77, -77, -77, -77, -77, -77,
        },
        {   // Knight
            -240, -153, -44, 98, -84, -238, -228, -188,
            6, -7, 78, 36, -14, 106, -68, -13,
            3, 90, 91, 128, 135, 115, 85, 3,
            46, 31, 67, 80, 55, 78, 34, 42,
            -7, 26, 63, 37, 41, 56, 32, 6,
            -16, -1, 24, 48, 35, 23, 22, -18,
            -23, -8, 26, 4, 16, 30, -40, -17,
            -156, -42, -54, -21, -5, -15, -30, -167,
        },
        {   // Bishop
            -37, -57, -103, -56, -80, -245, -160, -2,
            -42, 26, 34, 7, 24, -41, 7, -9,
            -6, 30, 21, 69, 34, 83, 31, -2,
            4, 9, 70, 39, 49, 42, 29, -7,
            -14, 28, 14, 51, 44, 34, 1, -4,
            14, 19, 20, 8, 22, 11, 25, 13,
            -7, 24, 27, 6, 13, 29, 34, -32,
            -14, -22, -15, 4, -21, -25, -55, 8,
        },
        {   // Rook
            17, 48, 89, 57, 101, -29, 42, 106,
            50, 13, 72, 80, 90, 41, -21, 83,
            -6, 9, 61, 75, 122, 57, 88, 37,
            -37, -15, 31, 34, 30, 32, -35, -20,
            -68, -51, -18, -5, -1, -31, -48, -23,
            -67, -51, -78, -56, -16, -60, -45, -60,
            -63, -66, -42, -21, -28, -5, -70, -80,
            -47, -38, -28, -9, 5, -24, -54, -52,
        },
        {   // Queen
            -36, 126, 89, 75, -59, 28, 4, -81,
            0, -8, 6, 58, 24, 24, 36, 27,
            -10, -4, -32, 19, 41, 50, 15, 28,
            -42, -31, 32, -13, 3, 2, 5, -10,
            -34, -25, -9, -6, 8, -5, -5, 5,
            -35, -10, -14, -13, -13, -13, 22, 4,
            -30, -6, 5, -7, -4, 29, -10, -42,
            -59, -49, -32, -8, -6, -12, 36, -32,
        },
        {   // King
            255, 80, 66, 323, 67, -288, 408, -214,
            428, -133, 87, 231, -182, -246, 36, 309,
            -93, 137, 74, -135, -12, 71, -54, 32,
            -109, -185, 45, 68, -154, 17, -119, -79,
            -143, -69, -61, -107, -167, -81, -62, -121,
            -69, -28, -32, -70, -75, -90, -15, -15,
            21, 21, 21, -41, -30, 17, 58, 62,
            33, 70, 46, -15, 32, 21, 74, 86,
        }
    },
    {   // Endgame
        {   // Pawn
            -186, -186, -186, -186, -186, -186, -186, -186,
            209, 208, 209, 168, 132, 188, 200, 205,
            96, 116, 88, 51, 46, 75, 107, 102,
            68, 74, 12, 10, 12, 7, 29, 46,
            43, 41, 16, 13, -5, 20, 28, 21,
            30, 40, 0, 9, 26, 20, 15, 13,
            36, 37, 15, 37, 39, 12, 3, 11,
            -186, -186, -186, -186, -186, -186, -186, -186,
        },
        {   // Knight
            -68, 35, 28, -85, 38, 58, 55, -140,
            -50, -4, -23, 44, 63, -30, 7, -134,
            -6, -15, 51, 15, -10, 15, 23, 3,
            -16, 28, 33, 51, 78, 38, 11, -12,
            12, 15, 40, 55, 60, 30, 70, 3,
            4, 15, 23, 43, 32, 18, 18, -31,
            -59, -46, -30, 14, 1, 1, 47, -35,
            3, -51, -4, -28, -32, -59, -39, -175,
        },
        {   // Bishop
            34, -22, 45, -11, 51, 61, 26, -86,
            25, 7, -5, 25, 30, 10, -11, -19,
            10, 18, 18, 13, 49, 37, 7, -12,
            -10, 34, 23, 37, 31, 25, 16, -16,
            13, -1, 38, 38, 17, 10, 30, 2,
            -36, 7, 32, 38, 32, 32, 9, -41,
            -45, -5, -16, -6, 3, -25, -21, -154,
            -34, -81, -52, -58, -21, -31, -63, -52,
        },
        {   // Rook
            38, 20, -13, 9, -2, 26, -4, -6,
            6, 35, 7, 20, 14, 11, 44, 9,
            31, 25, 4, -14, -8, 3, -8, 5,
            24, 23, 21, 1, 18, 6, 43, 12,
            12, 11, 11, 4, -5, 19, 1, -5,
            -15, -19, 9, -4, -24, 12, 20, -49,
            -38, -34, -22, -32, -28, -38, -12, -32,
            -20, -8, -4, -22, -20, -28, -11, -29,
        },
        {   // Queen
            -19, -96, 40, 9, 87, 29, 133, 119,
            40, 18, 82, 18, 82, 28, 69, 62,
            12, 49, 92, 8, 126, 82, 75, 18,
            61, 58, 39, 95, 114, 126, 46, 109,
            45, 10, 29, 66, 53, 63, 14, -19,
            -80, -45, 42, 5, 30, 37, -51, -119,
            -88, -120, -112, -44, -73, -188, -67, -120,
            -50, 2, -116, -143, -234, -178, -351, -11,
        },
        {   // King
            -272, -71, -56, -86, -67, 105, -96, -190,
            -77, 71, 7, -71, 91, 114, 26, -21,
            54, 35, 29, 92, 50, 33, 73, 38,
            53, 61, 42, 17, 90, 42, 59, 56,
            19, 52, 38, 45, 76, 53, 34, 31,
            -17, 10, 28, 36, 49, 43, 8, -13,
            -61, -27, -14, 26, 8, -13, -26, -60,
            -87, -59, -47, -56, -71, -61, -62, -112
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
