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
    { 58, 300, 341, 433, 901, 0, -58, -300, -341, -433, -901, 0 },
    { 107, 398, 432, 704, 1385, 0, -107, -398, -432, -704, -1385, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -70, -70, -70, -70, -70, -70, -70, -70,
            85, 67, 54, 78, 17, 19, 10, 37,
            13, 23, 30, 59, 59, 52, 23, 24,
            -7, 23, 24, 42, 48, 30, 31, 3,
            -17, 16, 15, 35, 31, 24, 24, -5,
            -14, 18, 14, 14, 22, 17, 46, -2,
            -28, 16, 2, -12, 1, 31, 46, -11,
            -70, -70, -70, -70, -70, -70, -70, -70,
        },
        {   // Knight
            -165, -189, -129, -84, 30, -241, -245, -97,
            -27, -18, 55, 15, 41, 83, -18, -17,
            1, 70, 54, 92, 112, 79, 71, 22,
            19, 28, 46, 70, 57, 71, 31, 48,
            -5, 7, 41, 32, 40, 39, 17, 1,
            -9, 8, 20, 31, 38, 23, 25, -2,
            -18, -21, 0, 7, 9, 23, 7, -10,
            -41, -28, -20, -3, -8, -11, -16, -40,
        },
        {   // Bishop
            -37, -90, -61, -93, -80, -145, -87, -19,
            -25, 27, 14, -41, 4, 2, 32, -32,
            -27, 39, 22, 60, 44, 48, 27, 20,
            2, 12, 25, 35, 30, 28, 16, 3,
            -8, 9, 14, 31, 20, 12, 12, -12,
            4, 14, 15, 13, 14, 17, 16, 14,
            -5, 16, 11, 7, 13, 22, 27, 16,
            3, -3, -11, -5, -15, -14, -19, 21,
        },
        {   // Rook
            48, 68, 78, 118, 79, 25, 19, 58,
            15, 17, 54, 72, 70, 64, 8, 44,
            -19, 18, 25, 63, 70, 32, 27, 23,
            -28, -23, -5, 14, 8, -2, -14, -18,
            -53, -43, -20, -16, -17, -27, -35, -32,
            -50, -32, -33, -36, -32, -47, -17, -46,
            -56, -28, -19, -22, -23, -15, -24, -73,
            -37, -30, -23, -4, -2, -23, -51, -43,
        },
        {   // Queen
            -71, 10, 51, 61, 35, 86, -32, -36,
            -26, -21, -10, -3, 12, 6, 4, 14,
            -16, -6, 4, 16, 24, 32, 20, 28,
            -10, -20, -14, 3, -1, 7, -14, 10,
            -10, -21, -14, -1, -7, -1, -15, -20,
            -13, -1, -12, -6, -1, -4, 7, -7,
            -5, 6, 13, 5, 9, 21, 15, -21,
            -8, -20, 3, 4, -2, -17, -27, 6,
        },
        {   // King
            57, 109, 43, 119, 78, 166, 106, 36,
            154, 191, 71, 205, -47, -91, 1, 43,
            -6, -20, 33, -6, -82, -31, -69, 5,
            -54, 11, -97, -139, -87, -77, -65, -76,
            -116, -66, -126, -148, -161, -102, -77, -102,
            -61, 9, -57, -83, -80, -58, -11, 10,
            87, 44, 25, -23, -11, 39, 64, 67,
            64, 64, 52, 1, 50, 32, 96, 95,
        }
    },
    {   // Endgame
        {   // Pawn
            -179, -179, -179, -179, -179, -179, -179, -179,
            175, 190, 170, 120, 135, 147, 183, 185,
            106, 103, 90, 60, 53, 60, 109, 83,
            59, 55, 30, 20, 8, 20, 36, 46,
            36, 43, 20, 16, 14, 18, 35, 28,
            30, 36, 13, 24, 25, 24, 20, 19,
            35, 31, 29, 32, 34, 24, 17, 15,
            -179, -179, -179, -179, -179, -179, -179, -179,
        },
        {   // Knight
            -119, 13, 33, 38, -10, 127, 6, -199,
            -41, 8, 2, 47, 14, -12, 4, -39,
            -11, 13, 45, 32, 18, 32, 11, -13,
            1, 38, 42, 53, 57, 46, 26, -12,
            -2, 18, 38, 50, 47, 35, 29, 4,
            -19, 3, 9, 24, 32, 16, 7, -23,
            -69, -9, 2, 14, 9, -18, -28, -56,
            -93, -50, -13, -39, -25, -26, -45, -68,
        },
        {   // Bishop
            11, 45, 23, 33, 40, 24, 20, -49,
            5, -7, 4, 18, 16, 24, -4, -17,
            24, 4, 20, 6, 13, 20, 21, -9,
            -8, 30, 26, 42, 34, 20, 16, 1,
            -5, 14, 35, 28, 30, 22, -3, -19,
            -13, -12, 20, 25, 23, 16, -16, -27,
            -51, -28, -17, -8, -3, -18, -23, -50,
            -53, -51, -51, -28, -34, -39, -35, -77,
        },
        {   // Rook
            7, 6, 1, -14, -3, 11, 12, 1,
            27, 29, 24, 21, 12, 8, 30, 7,
            31, 20, 22, 3, -7, 15, 16, 6,
            23, 22, 31, 24, 14, 22, 19, 7,
            9, 11, 15, 10, 4, 4, 5, -21,
            -24, -9, -1, 4, -8, 1, -20, -33,
            -34, -30, -13, -17, -23, -22, -42, -52,
            -25, -14, -2, -18, -22, -19, -10, -50,
        },
        {   // Queen
            50, 12, -4, 5, 45, -3, 84, 85,
            10, 33, 78, 58, 80, 78, 45, -14,
            -17, 17, 53, 67, 73, 83, 73, 24,
            -35, 61, 76, 94, 108, 75, 97, 19,
            -39, 45, 40, 60, 62, 45, 66, 40,
            -79, -56, 24, 1, -3, 21, -21, -31,
            -124, -98, -74, -59, -67, -78, -110, -86,
            -108, -103, -140, -107, -119, -130, -129, -153,
        },
        {   // King
            -195, -46, -41, -66, -41, -50, -48, -75,
            -43, -3, 22, -34, 48, 72, 46, 13,
            13, 62, 54, 40, 54, 73, 82, 25,
            31, 46, 60, 67, 55, 66, 54, 35,
            17, 36, 59, 60, 68, 54, 37, 20,
            -1, -2, 28, 41, 39, 31, 7, -26,
            -56, -27, -12, 8, 2, -17, -37, -54,
            -115, -71, -61, -62, -82, -76, -80, -102
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
