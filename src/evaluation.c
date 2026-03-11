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
    { 86, 408, 456, 617, 1248, 0, -86, -408, -456, -617, -1248, 0 },
    { 119, 443, 482, 785, 1515, 0, -119, -443, -482, -785, -1515, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -77, -77, -77, -77, -77, -77, -77, -77,
            142, 71, 38, 208, 127, 32, 4, 22,
            -13, 20, 42, 65, 91, 47, 53, 21,
            -12, 22, 23, 34, 43, 33, 24, -11,
            -32, 8, 9, 33, 31, 13, 11, -15,
            -27, 12, 5, 12, 18, 10, 43, -12,
            -44, 10, -6, -19, 2, 29, 41, -24,
            -77, -77, -77, -77, -77, -77, -77, -77,
        },
        {   // Knight
            -286, -73, -119, -69, 80, -266, -241, -170,
            -9, -25, 52, -4, 44, 54, 2, -41,
            -32, 44, 116, 127, 148, 93, 115, 34,
            49, 35, 42, 77, 74, 95, 34, 44,
            -2, 4, 46, 44, 47, 46, 23, 7,
            -13, 6, 17, 29, 36, 21, 27, -20,
            -40, -21, -2, 6, 15, 16, 19, -23,
            -93, -35, -21, -22, -11, 9, -19, -116,
        },
        {   // Bishop
            -74, -76, 25, -81, -90, -177, -196, -40,
            10, 12, 7, 0, 8, 38, 28, -34,
            -12, 32, 44, 59, 56, 75, 25, 25,
            -8, 11, 46, 42, 40, 47, 17, 10,
            5, 12, 18, 45, 45, 10, 30, -11,
            2, 5, 14, 15, 9, 18, 1, 6,
            -3, 9, 20, -4, 9, 28, 20, 14,
            -50, 8, -23, -35, -22, -27, -6, -25,
        },
        {   // Rook
            81, 7, 87, 130, 220, 255, 146, 54,
            13, 23, 66, 101, 94, 133, 26, 9,
            -42, 33, 19, 56, 81, 81, 49, 25,
            -55, -32, -31, 5, 12, 25, -10, -26,
            -72, -71, -56, -4, -42, -44, -30, -69,
            -77, -22, -46, -40, -39, -67, -32, -94,
            -86, -52, -25, -36, -35, -27, -61, -107,
            -61, -56, -50, -24, -20, -36, -78, -72,
        },
        {   // Queen
            -127, 16, 28, 18, 16, 125, -16, -28,
            -45, -29, 10, 14, 10, 53, 15, -37,
            -23, -2, -28, 50, 56, 78, 24, 34,
            -26, -32, 3, -3, 15, 13, -1, -1,
            -12, -41, -8, -7, 2, 11, -14, -20,
            -24, -1, -3, 7, -1, 9, 7, 4,
            -39, 5, 10, -2, 4, 19, 33, 10,
            -65, -22, -11, -3, -8, -29, 1, 8,
        },
        {   // King
            37, 245, 250, 29, 118, -220, -67, -147,
            361, 58, 241, 396, -30, -134, -51, -315,
            56, -149, -95, 190, 172, -102, -54, -90,
            26, -116, -189, -43, -16, -19, -49, -53,
            -99, -29, -84, -41, -86, -73, -47, -89,
            14, -23, -105, -95, -80, -44, -2, -24,
            99, 32, 32, -26, -11, 34, 51, 58,
            75, 70, 55, -23, 41, 11, 89, 77,
        }
    },
    {   // Endgame
        {   // Pawn
            -174, -174, -174, -174, -174, -174, -174, -174,
            151, 209, 178, 25, 55, 115, 159, 209,
            104, 96, 70, 49, 36, 37, 63, 68,
            71, 53, 39, 23, 26, 14, 43, 46,
            56, 57, 38, 15, 15, 28, 54, 38,
            38, 41, 31, 30, 35, 37, 32, 23,
            48, 47, 39, 29, 35, 34, 27, 23,
            -174, -174, -174, -174, -174, -174, -174, -174,
        },
        {   // Knight
            -52, 49, 18, 9, -13, 190, 47, -183,
            -48, 12, 10, 109, 3, 22, -19, -5,
            5, 13, 10, 33, 13, 39, 31, -12,
            -29, 28, 61, 38, 39, 35, 43, 8,
            -19, 19, 32, 30, 48, 43, 25, -18,
            -30, -15, 10, 28, 27, 15, -4, -1,
            -86, -15, -12, 9, 2, 5, -42, -35,
            -151, -48, -48, -46, -23, -58, -45, -100,
        },
        {   // Bishop
            53, 2, 7, 38, 29, 48, 49, -44,
            -32, -9, 21, 9, 12, -35, 10, -20,
            1, -3, 28, 25, 1, 36, 37, 8,
            -10, 28, 20, 16, 45, 43, 15, -9,
            -8, 15, 23, 22, 25, 40, 8, -17,
            -9, -3, 21, 32, 30, 7, 3, -29,
            -22, -17, -10, 3, -1, -26, -19, -58,
            -59, -114, -44, -15, -23, -48, -101, -24,
        },
        {   // Rook
            -7, 10, 8, -19, -45, -70, -36, -6,
            25, 15, 24, 13, -1, -10, 23, 19,
            43, 22, 32, 17, 6, 4, 12, 3,
            27, 31, 33, 28, 25, 29, 16, 24,
            1, 17, 23, 2, 10, 20, 13, 0,
            -28, -30, -8, -10, -7, 16, -11, -19,
            -40, -40, -36, -13, -23, -35, -10, -37,
            -12, 1, 11, -7, -10, -8, 6, -27,
        },
        {   // Queen
            131, 46, -15, 47, 98, -2, 71, 32,
            -14, 3, 104, 46, 67, 22, 71, 49,
            28, -12, 66, 110, 58, 34, 65, -4,
            18, 16, 53, 65, 93, 100, 86, 100,
            -51, 76, 21, 89, 67, 33, 102, 24,
            -45, -64, -25, -15, -7, -2, -31, -23,
            -23, -110, -117, -22, -57, -138, -190, -167,
            9, -223, -132, -105, -104, -77, -131, -195,
        },
        {   // King
            -358, -164, -121, -70, -118, 0, 33, -16,
            -102, -30, -49, -100, 82, 90, 73, 79,
            -20, 92, 70, -5, 9, 116, 87, 47,
            -21, 69, 69, 27, 33, 44, 65, 25,
            2, 25, 48, 37, 43, 43, 35, 26,
            -36, 21, 56, 46, 49, 37, 21, 11,
            -46, -18, 15, 24, 30, 9, -5, -29,
            -80, -42, -25, -35, -49, -34, -45, -70
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
