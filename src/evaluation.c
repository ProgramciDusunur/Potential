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
    { 61, 311, 355, 450, 937, 0, -61, -311, -355, -450, -937, 0 },
    { 113, 413, 449, 732, 1438, 0, -113, -413, -449, -732, -1438, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -71, -71, -71, -71, -71, -71, -71, -71,
            88, 69, 55, 80, 17, 18, 10, 38,
            13, 23, 30, 61, 60, 53, 23, 24,
            -8, 23, 24, 42, 49, 31, 32, 2,
            -19, 16, 15, 36, 31, 24, 24, -6,
            -16, 17, 13, 13, 22, 17, 47, -3,
            -30, 16, 1, -13, 0, 32, 47, -13,
            -71, -71, -71, -71, -71, -71, -71, -71,
        },
        {   // Knight
            -172, -197, -134, -87, 31, -250, -255, -101,
            -28, -19, 58, 16, 43, 86, -18, -18,
            1, 73, 56, 96, 116, 83, 74, 23,
            20, 29, 48, 72, 59, 74, 32, 50,
            -5, 8, 43, 33, 42, 41, 17, 1,
            -9, 8, 20, 32, 40, 24, 26, -2,
            -18, -22, 0, 7, 9, 23, 7, -11,
            -43, -29, -21, -4, -8, -11, -16, -42,
        },
        {   // Bishop
            -39, -94, -64, -97, -83, -151, -91, -20,
            -26, 28, 15, -42, 4, 2, 33, -33,
            -28, 40, 23, 62, 45, 50, 28, 20,
            2, 12, 25, 37, 31, 29, 17, 3,
            -8, 9, 14, 32, 20, 13, 12, -12,
            4, 15, 15, 13, 15, 17, 17, 14,
            -6, 17, 12, 7, 14, 23, 28, 17,
            4, -3, -11, -5, -15, -15, -19, 21,
        },
        {   // Rook
            50, 70, 81, 123, 83, 26, 20, 61,
            15, 18, 56, 75, 73, 66, 9, 46,
            -19, 19, 26, 66, 73, 34, 28, 24,
            -29, -24, -6, 15, 9, -2, -14, -19,
            -55, -45, -21, -17, -18, -29, -37, -33,
            -52, -33, -34, -37, -33, -49, -18, -47,
            -58, -29, -20, -23, -24, -15, -25, -76,
            -38, -31, -24, -4, -2, -24, -52, -45,
        },
        {   // Queen
            -74, 10, 53, 63, 36, 90, -33, -37,
            -27, -22, -11, -3, 13, 6, 4, 15,
            -17, -6, 4, 17, 24, 33, 20, 29,
            -11, -21, -14, 3, -1, 7, -14, 11,
            -10, -22, -15, -1, -7, -1, -16, -21,
            -13, -1, -13, -6, -1, -4, 7, -8,
            -5, 6, 13, 5, 9, 21, 16, -22,
            -8, -20, 3, 4, -2, -17, -28, 6,
        },
        {   // King
            54, 105, 46, 123, 80, 165, 108, 33,
            159, 197, 74, 210, -47, -92, 3, 43,
            -4, -20, 35, -6, -85, -31, -71, 6,
            -56, 12, -100, -144, -90, -80, -67, -78,
            -120, -68, -131, -153, -167, -105, -79, -106,
            -63, 10, -59, -85, -83, -60, -11, 11,
            91, 46, 27, -24, -11, 41, 67, 70,
            67, 67, 55, 2, 53, 34, 100, 99,
        }
    },
    {   // Endgame
        {   // Pawn
            -179, -179, -179, -179, -179, -179, -179, -179,
            180, 195, 175, 123, 139, 151, 188, 190,
            108, 105, 91, 60, 53, 60, 111, 84,
            59, 55, 29, 19, 6, 19, 35, 46,
            35, 42, 19, 15, 13, 16, 34, 27,
            29, 35, 11, 23, 24, 23, 18, 18,
            35, 30, 28, 32, 33, 23, 16, 13,
            -179, -179, -179, -179, -179, -179, -179, -179,
        },
        {   // Knight
            -124, 14, 34, 40, -11, 132, 7, -207,
            -43, 8, 2, 49, 15, -13, 4, -41,
            -11, 14, 47, 33, 19, 34, 11, -13,
            1, 39, 44, 55, 59, 48, 27, -13,
            -2, 19, 40, 52, 49, 36, 30, 5,
            -20, 3, 9, 25, 33, 17, 7, -24,
            -72, -9, 2, 14, 9, -19, -29, -58,
            -96, -52, -14, -41, -26, -27, -47, -70,
        },
        {   // Bishop
            12, 47, 24, 34, 41, 25, 20, -51,
            6, -7, 4, 19, 17, 25, -4, -18,
            25, 4, 21, 6, 13, 21, 22, -9,
            -8, 31, 27, 44, 36, 21, 17, 1,
            -5, 14, 36, 29, 31, 23, -3, -19,
            -13, -13, 20, 26, 24, 17, -17, -28,
            -53, -29, -18, -9, -4, -19, -24, -52,
            -55, -53, -53, -29, -35, -41, -37, -80,
        },
        {   // Rook
            7, 6, 1, -15, -3, 11, 12, 1,
            28, 30, 25, 22, 12, 8, 32, 8,
            33, 20, 23, 3, -7, 16, 16, 6,
            24, 23, 32, 25, 15, 23, 20, 7,
            9, 11, 16, 10, 4, 4, 6, -22,
            -25, -9, -1, 4, -8, 1, -21, -35,
            -35, -31, -13, -18, -24, -23, -43, -55,
            -26, -15, -2, -19, -23, -20, -11, -51,
        },
        {   // Queen
            52, 13, -5, 6, 47, -4, 87, 88,
            10, 34, 81, 60, 83, 81, 47, -14,
            -17, 17, 56, 70, 76, 87, 76, 25,
            -37, 64, 79, 98, 112, 78, 101, 19,
            -40, 47, 42, 62, 64, 47, 69, 41,
            -82, -58, 25, 1, -3, 22, -22, -32,
            -129, -102, -77, -61, -69, -81, -115, -90,
            -112, -107, -146, -112, -124, -135, -134, -159,
        },
        {   // King
            -201, -45, -43, -69, -42, -50, -50, -77,
            -45, -2, 23, -34, 49, 74, 47, 14,
            13, 64, 55, 41, 56, 76, 85, 26,
            32, 47, 62, 70, 57, 69, 56, 36,
            17, 37, 61, 62, 70, 56, 38, 21,
            -1, -2, 28, 43, 41, 32, 7, -27,
            -59, -28, -13, 8, 2, -18, -39, -56,
            -119, -74, -63, -65, -85, -79, -83, -106
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
