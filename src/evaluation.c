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
    { 80, 368, 416, 552, 1161, 0, -80, -368, -416, -552, -1161, 0 },
    { 134, 424, 460, 748, 1428, 0, -134, -424, -460, -748, -1428, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -77, -77, -77, -77, -77, -77, -77, -77,
            93, 102, 85, 60, 51, 37, 3, 100,
            13, 41, 47, 61, 78, 40, 36, 9,
            -10, 28, 21, 40, 49, 30, 43, -8,
            -25, 12, 12, 33, 35, 22, 25, -19,
            -25, 13, 16, 13, 23, 19, 46, -15,
            -41, 9, 1, -19, -1, 33, 43, -29,
            -77, -77, -77, -77, -77, -77, -77, -77,
        },
        {   // Knight
            -207, -150, -151, -43, 7, -276, -228, -138,
            -29, -9, 85, -23, -19, 109, -25, 2,
            28, 72, 90, 122, 123, 93, 93, 14,
            26, 33, 62, 81, 66, 77, 39, 42,
            -4, 13, 52, 42, 49, 51, 21, 2,
            -19, 9, 21, 43, 45, 26, 26, -10,
            -33, -32, 4, 6, 10, 8, -6, -6,
            -79, -37, -51, -12, -11, -19, -22, -52,
        },
        {   // Bishop
            -58, -25, -104, -91, -60, -170, -120, -21,
            -18, 27, 13, -32, -18, -14, 8, -51,
            -18, 37, 28, 63, 44, 70, 33, 11,
            8, 22, 47, 43, 46, 39, 24, 0,
            -8, 24, 20, 38, 38, 24, 2, -21,
            10, 15, 22, 16, 23, 21, 17, 10,
            0, 21, 20, 9, 16, 24, 33, 4,
            -40, -2, -13, -4, -41, -20, -26, 3,
        },
        {   // Rook
            50, 50, 119, 69, 126, 44, 39, 44,
            32, 16, 77, 85, 74, 71, -8, 51,
            -28, 12, 29, 81, 104, 57, 76, 31,
            -26, -23, 16, 25, 28, 5, -15, -32,
            -61, -46, -30, -17, -16, -27, -28, -38,
            -74, -43, -58, -38, -20, -48, -21, -66,
            -71, -57, -36, -38, -19, -17, -49, -84,
            -51, -43, -32, -14, -5, -30, -53, -53,
        },
        {   // Queen
            -55, 57, 102, 79, 31, 90, -13, -52,
            -9, -17, -3, 11, 39, 11, 28, 20,
            -41, 1, -5, 25, 49, 45, 24, 18,
            -38, -30, 8, 2, -13, 21, -3, 5,
            -26, -35, -15, -6, 4, -7, -7, -6,
            -51, -10, -11, -4, -3, -3, 16, -11,
            -40, -8, 2, -8, -4, 12, -2, -27,
            -13, -39, -16, -10, -13, -19, -17, -12,
        },
        {   // King
            173, 59, 272, 223, -37, -201, 209, -144,
            180, 68, 143, 137, -135, -120, 82, 228,
            97, 65, -59, -14, -51, 43, 31, 9,
            -110, -126, -47, -7, -91, 16, -70, -44,
            -124, -111, -91, -110, -98, -122, -68, -101,
            -9, -31, -63, -102, -83, -76, -18, -22,
            47, 20, 9, -40, -29, 24, 45, 52,
            33, 54, 42, -15, 30, 13, 83, 83,
        }
    },
    {   // Endgame
        {   // Pawn
            -188, -188, -188, -188, -188, -188, -188, -188,
            216, 195, 192, 165, 170, 193, 221, 210,
            99, 112, 83, 56, 41, 68, 111, 90,
            65, 51, 25, 18, 6, 18, 34, 45,
            40, 34, 15, 12, 5, 14, 31, 25,
            22, 35, 4, 18, 18, 19, 18, 10,
            32, 30, 24, 42, 35, 13, 11, 10,
            -188, -188, -188, -188, -188, -188, -188, -188,
        },
        {   // Knight
            -125, 22, 41, 20, -6, 60, 79, -166,
            -23, -1, -13, 86, 62, -29, 25, -61,
            -30, 3, 34, 21, 9, 46, 13, 2,
            -5, 25, 43, 48, 58, 50, 19, 4,
            -6, 10, 39, 50, 46, 36, 51, -1,
            -27, -6, 14, 25, 19, 10, 1, -31,
            -79, -1, -9, 3, 5, 3, -8, -29,
            -73, -58, -22, -27, -32, -58, -47, -107,
        },
        {   // Bishop
            11, 14, 29, 19, 35, 30, 36, -33,
            16, 1, -9, 22, 21, 7, -2, 6,
            22, 26, 12, 8, 25, 18, 32, -7,
            -14, 31, 20, 31, 29, 29, 19, 1,
            1, 7, 27, 37, 27, 16, 25, -22,
            -28, -2, 19, 32, 19, 21, -12, -15,
            -62, -32, -11, -5, -4, -34, -22, -102,
            -37, -50, -55, -40, -9, -44, -78, -51,
        },
        {   // Rook
            13, 4, -25, 8, -16, -4, 2, 10,
            14, 32, 13, 19, 20, 3, 38, 14,
            38, 27, 21, 4, -12, 3, 3, 9,
            25, 30, 24, 12, 11, 20, 25, 17,
            11, 10, 10, 9, 2, 9, 5, -11,
            -11, -11, 0, -6, -21, 1, -16, -41,
            -53, -25, -22, -17, -24, -25, -34, -36,
            -16, -6, -6, -16, -17, -15, -4, -27,
        },
        {   // Queen
            30, -9, -18, -5, 77, -52, 68, 61,
            14, 16, 89, 62, 46, 94, 14, 23,
            31, 13, 74, 50, 70, 85, 117, 23,
            38, 63, 67, 89, 160, 69, 92, 50,
            11, 54, 34, 74, 57, 51, 30, -6,
            -24, -42, 15, -10, 19, 13, -43, -74,
            -89, -107, -91, -57, -55, -114, -67, -160,
            -109, -102, -150, -114, -147, -151, -220, -26,
        },
        {   // King
            -182, -68, -100, -86, -42, 34, -71, 2,
            -63, 11, -12, -38, 73, 82, 8, -42,
            10, 52, 70, 57, 61, 47, 51, 31,
            57, 80, 63, 45, 61, 50, 67, 28,
            28, 55, 52, 54, 59, 58, 35, 20,
            -20, 7, 32, 48, 44, 31, 5, -18,
            -73, -28, -11, 10, 8, -12, -34, -59,
            -103, -77, -59, -60, -76, -75, -73, -102
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
