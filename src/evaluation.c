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
    { 81, 371, 420, 558, 1170, 0, -81, -371, -420, -558, -1170, 0 },
    { 128, 420, 454, 737, 1404, 0, -128, -420, -454, -737, -1404, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -78, -78, -78, -78, -78, -78, -78, -78,
            98, 120, 83, 63, 44, 24, 29, 90,
            12, 44, 45, 68, 76, 42, 31, 12,
            -9, 27, 22, 39, 48, 32, 40, -7,
            -23, 11, 9, 33, 32, 23, 24, -19,
            -21, 13, 11, 11, 21, 19, 45, -15,
            -38, 8, -2, -19, -2, 33, 43, -28,
            -78, -78, -78, -78, -78, -78, -78, -78,
        },
        {   // Knight
            -213, -182, -151, -67, 21, -278, -206, -162,
            -26, 2, 79, -16, -3, 104, -20, -11,
            10, 77, 89, 122, 125, 98, 90, 10,
            30, 34, 65, 86, 65, 85, 38, 41,
            -4, 19, 48, 42, 50, 51, 20, 3,
            -17, 8, 21, 41, 44, 26, 26, -9,
            -25, -22, 6, 5, 10, 13, -6, -3,
            -78, -36, -53, -14, -10, -15, -21, -59,
        },
        {   // Bishop
            -58, -36, -72, -96, -64, -186, -108, -10,
            -27, 25, 13, -35, -20, -11, 6, -45,
            -22, 40, 39, 65, 54, 65, 31, 13,
            7, 21, 43, 46, 45, 42, 22, 0,
            -7, 17, 18, 35, 38, 22, 6, -19,
            6, 14, 19, 16, 19, 23, 17, 8,
            -7, 18, 18, 5, 13, 27, 31, 7,
            -24, -3, -15, -9, -42, -20, -26, 6,
        },
        {   // Rook
            53, 56, 118, 92, 103, 78, 44, 67,
            27, 12, 71, 81, 75, 81, -8, 47,
            -22, 19, 31, 71, 98, 50, 62, 19,
            -25, -16, 19, 17, 30, 5, -10, -31,
            -54, -49, -25, -17, -17, -28, -27, -47,
            -73, -46, -46, -48, -25, -53, -25, -64,
            -70, -53, -35, -35, -25, -21, -49, -89,
            -50, -42, -34, -14, -8, -32, -59, -55,
        },
        {   // Queen
            -64, 65, 88, 77, 73, 98, -9, -53,
            -13, -18, 2, 12, 23, 18, 21, 13,
            -37, -9, -3, 31, 46, 50, 31, 23,
            -41, -27, 3, 0, -6, 14, -10, 5,
            -21, -40, -15, -6, -1, -9, -12, -5,
            -44, -11, -13, -7, -6, -6, 9, -15,
            -37, -7, 3, -7, -4, 13, 5, -14,
            -24, -38, -16, -11, -14, -15, -25, -9,
        },
        {   // King
            121, 96, 252, 200, 24, -155, 186, -90,
            193, 68, 112, 176, -85, -102, 73, 254,
            86, 80, -23, 5, -27, 25, 12, 7,
            -93, -117, -50, -9, -67, 6, -66, -48,
            -127, -101, -109, -108, -123, -121, -88, -99,
            2, -46, -76, -114, -93, -83, -27, -22,
            33, 12, -3, -50, -39, 14, 36, 41,
            14, 42, 32, -24, 22, 6, 76, 76,
        }
    },
    {   // Endgame
        {   // Pawn
            -186, -186, -186, -186, -186, -186, -186, -186,
            212, 181, 173, 148, 157, 173, 190, 195,
            109, 108, 86, 65, 51, 61, 103, 84,
            66, 55, 24, 25, 12, 17, 38, 48,
            40, 39, 18, 17, 11, 16, 33, 29,
            23, 35, 10, 21, 21, 21, 21, 16,
            36, 33, 27, 43, 32, 16, 15, 17,
            -186, -186, -186, -186, -186, -186, -186, -186,
        },
        {   // Knight
            -111, 22, 50, 33, -16, 104, 75, -149,
            -27, -3, -15, 75, 56, -21, 22, -44,
            -24, 5, 34, 23, 11, 35, 19, -3,
            -10, 22, 39, 45, 55, 43, 24, 3,
            -4, 12, 39, 44, 43, 36, 42, -9,
            -30, -4, 12, 26, 18, 8, -2, -32,
            -82, 1, -12, 3, 4, -8, -7, -47,
            -74, -59, -22, -29, -26, -54, -50, -106,
        },
        {   // Bishop
            11, 23, 13, 19, 36, 38, 32, -23,
            12, -2, -3, 18, 23, 7, 6, 5,
            18, 19, 11, 9, 13, 15, 21, -8,
            -13, 31, 22, 31, 31, 29, 13, 1,
            -2, 7, 25, 36, 25, 15, 14, -17,
            -15, -2, 20, 29, 21, 15, -6, -14,
            -54, -25, -6, -2, -3, -29, -17, -94,
            -60, -49, -50, -36, -16, -41, -73, -57,
        },
        {   // Rook
            8, 3, -22, -2, -13, -12, -1, -4,
            18, 32, 19, 20, 20, 2, 35, 12,
            37, 25, 23, 9, -9, 9, 6, 12,
            22, 27, 20, 19, 10, 19, 22, 14,
            9, 14, 6, 13, 3, 8, 9, -7,
            -12, -8, -7, -1, -17, 0, -20, -37,
            -46, -23, -20, -17, -24, -22, -34, -37,
            -18, -10, -4, -13, -17, -17, -3, -28,
        },
        {   // Queen
            39, -23, -4, 7, 30, -32, 54, 64,
            27, 25, 80, 52, 68, 96, 18, 28,
            27, 41, 77, 64, 85, 84, 82, 10,
            40, 57, 69, 95, 142, 81, 102, 43,
            4, 50, 30, 71, 57, 56, 36, -12,
            -30, -45, 22, -13, 14, 18, -26, -67,
            -111, -102, -85, -50, -59, -116, -97, -161,
            -86, -101, -157, -114, -145, -150, -177, -83,
        },
        {   // King
            -192, -68, -89, -66, -62, 18, -67, -8,
            -77, 11, 8, -41, 64, 77, 16, -57,
            14, 46, 66, 47, 56, 57, 58, 34,
            46, 74, 58, 42, 49, 50, 65, 28,
            26, 52, 57, 51, 61, 56, 39, 20,
            -26, 11, 32, 47, 43, 31, 8, -17,
            -55, -28, -9, 11, 8, -11, -32, -52,
            -88, -67, -55, -51, -72, -72, -73, -101
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
