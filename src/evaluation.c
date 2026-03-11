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
    { 56, 300, 335, 445, 885, 0, -56, -300, -335, -445, -885, 0 },
    { 110, 338, 377, 629, 1191, 0, -110, -338, -377, -629, -1191, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -72, -72, -72, -72, -72, -72, -72, -72,
            138, 70, 65, 72, 84, 33, 35, 51,
            -6, 10, 33, 55, 59, 36, 31, 17,
            -10, 21, 20, 34, 44, 31, 28, -2,
            -16, 14, 9, 33, 28, 21, 25, -11,
            -12, 10, 7, 10, 18, 18, 45, -9,
            -27, 7, 3, -13, -2, 26, 39, -21,
            -72, -72, -72, -72, -72, -72, -72, -72,
        },
        {   // Knight
            -153, -195, -46, -25, 50, -169, -236, -110,
            -35, 3, 58, 2, 50, 81, -15, 10,
            -18, 67, 52, 95, 100, 86, 71, 34,
            31, 19, 33, 64, 53, 70, 31, 36,
            -10, -6, 39, 24, 31, 36, 26, -1,
            -18, -5, 9, 28, 30, 18, 11, -17,
            -23, -15, 5, -2, 0, 15, -10, -6,
            -63, -39, -10, -19, -16, -13, -27, -66,
        },
        {   // Bishop
            -60, -70, -40, -75, -43, -140, -81, -31,
            -13, 24, 8, -18, -8, 0, 15, -24,
            -24, 8, 39, 59, 26, 34, 19, 17,
            10, 13, 21, 43, 41, 37, 20, 18,
            -5, 7, 12, 29, 20, 14, 16, -15,
            0, 4, 21, 11, 14, 18, 6, 8,
            -5, 16, 17, 3, 5, 14, 20, 6,
            -6, -20, -12, -7, -18, -22, 20, 2,
        },
        {   // Rook
            63, 48, 94, 111, 98, 89, 40, 35,
            28, 23, 56, 98, 108, 71, 17, 37,
            -33, 30, 40, 59, 114, 40, 73, 24,
            -41, -44, -5, 4, 15, 25, 0, 0,
            -49, -57, -36, -21, -10, -24, -29, -37,
            -65, -59, -40, -49, -43, -49, -34, -50,
            -60, -53, -38, -39, -27, -18, -46, -92,
            -51, -43, -37, -18, -11, -29, -57, -49,
        },
        {   // Queen
            -89, 42, 73, 87, 92, 127, 5, -46,
            -16, -22, 22, -6, -19, 25, 5, 7,
            -14, -1, 10, 6, 20, 20, 23, 23,
            -15, -28, 6, 4, -1, 2, -9, -3,
            -13, -31, -13, -9, -6, -7, -20, -12,
            -17, -6, -11, -3, -9, -10, 7, -14,
            -3, -3, 4, -4, 6, 17, -2, -16,
            -44, -22, -2, -4, -18, -14, -45, -7,
        },
        {   // King
            55, -6, 1, 78, 106, 60, 125, 19,
            68, 74, 90, 190, -111, -70, -23, 67,
            -55, -52, -30, -54, -48, 27, 13, -67,
            -131, -59, -50, -55, -17, -61, -51, -88,
            -107, -98, -104, -84, -85, -84, -74, -94,
            -32, 8, -40, -62, -55, -37, 12, 14,
            91, 52, 45, -1, 3, 55, 75, 83,
            69, 87, 68, 22, 62, 45, 106, 113,
        }
    },
    {   // Endgame
        {   // Pawn
            -179, -179, -179, -179, -179, -179, -179, -179,
            161, 205, 175, 148, 151, 182, 188, 172,
            110, 115, 96, 67, 63, 79, 95, 83,
            51, 46, 20, 21, 9, 20, 30, 47,
            34, 27, 18, 5, 11, 12, 27, 22,
            21, 24, 14, 26, 22, 21, 14, 18,
            29, 30, 30, 29, 29, 22, 18, 19,
            -179, -179, -179, -179, -179, -179, -179, -179,
        },
        {   // Knight
            -99, 16, -2, -10, -37, -4, 7, -150,
            -8, 3, 12, 63, 8, -5, 19, -58,
            23, 9, 32, 23, 3, 25, 19, -25,
            0, 36, 51, 47, 58, 44, 27, 4,
            7, 35, 37, 56, 52, 49, 23, 4,
            -17, 18, 24, 28, 27, 23, 5, -16,
            -80, -21, -15, 18, 11, -15, -2, -26,
            -92, -35, -27, -25, -24, -39, -46, -65,
        },
        {   // Bishop
            10, 15, -1, 12, 18, 40, 17, -5,
            0, -10, 1, 22, 16, 3, -3, 4,
            4, 7, 8, 5, 13, 27, 14, 0,
            -11, 28, 23, 26, 23, 27, 25, -13,
            0, 10, 31, 24, 36, 24, -5, -16,
            -10, 7, 15, 24, 25, 13, 11, -16,
            -65, -12, -17, -2, -2, -20, -7, -66,
            -39, -38, -51, -23, -41, -28, -65, -41,
        },
        {   // Rook
            -7, 8, -18, -2, -8, -17, 5, 6,
            15, 23, 12, -1, -3, -5, 14, 10,
            38, 13, 14, 3, -23, 10, 1, 11,
            32, 32, 31, 17, 13, 11, 21, 13,
            5, 12, 16, 6, -3, 12, 13, -5,
            -15, -4, -7, 2, 4, 7, -12, -24,
            -41, -29, -17, -14, -23, -21, -24, -36,
            -10, -10, 1, -6, -15, -12, 6, -26,
        },
        {   // Queen
            81, -25, -8, -35, -4, -37, 37, 64,
            11, 58, 29, 85, 128, 85, 72, 6,
            -30, 52, 58, 116, 93, 114, 93, 6,
            -19, 52, 51, 72, 123, 121, 82, 51,
            -41, 56, 28, 85, 75, 70, 75, 47,
            -95, -65, 19, -4, 17, 34, -23, -22,
            -167, -92, -93, -60, -74, -88, -116, -124,
            -33, -150, -174, -117, -107, -130, -150, -164,
        },
        {   // King
            -131, 55, 19, -14, 53, 117, 97, -54,
            -2, 38, 1, -26, 72, 95, 108, 23,
            34, 75, 38, 19, 24, 41, 65, 71,
            48, 46, 18, -2, -12, 29, 46, 25,
            14, 34, 20, 11, 18, 26, 22, 4,
            -17, -17, 4, 16, 14, 6, -11, -35,
            -73, -43, -43, -17, -19, -38, -52, -73,
            -115, -99, -81, -92, -99, -90, -84, -106
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
