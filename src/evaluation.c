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
    { 65, 305, 346, 462, 963, 0, -65, -305, -346, -462, -963, 0 },
    { 100, 341, 370, 603, 1153, 0, -100, -341, -370, -603, -1153, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -73, -73, -73, -73, -73, -73, -73, -73,
            92, 99, 67, 59, 41, 24, 21, 72,
            13, 37, 39, 58, 63, 37, 31, 18,
            -5, 25, 21, 35, 42, 28, 35, -1,
            -16, 13, 10, 30, 29, 21, 24, -11,
            -14, 13, 12, 12, 20, 17, 41, -8,
            -28, 10, 1, -13, 1, 29, 39, -18,
            -73, -73, -73, -73, -73, -73, -73, -73,
        },
        {   // Knight
            -167, -151, -124, -54, 13, -227, -157, -127,
            -20, 2, 67, -1, 5, 85, -14, -7,
            7, 66, 69, 99, 103, 83, 74, 10,
            24, 28, 52, 69, 53, 69, 30, 35,
            -3, 11, 39, 33, 39, 40, 16, 2,
            -14, 6, 17, 31, 34, 20, 20, -9,
            -23, -20, 3, 3, 7, 11, -7, -6,
            -63, -30, -38, -13, -9, -15, -20, -51,
        },
        {   // Bishop
            -44, -38, -60, -76, -49, -152, -85, -9,
            -20, 23, 11, -28, -13, -6, 9, -33,
            -19, 31, 34, 55, 41, 49, 25, 12,
            5, 16, 34, 37, 37, 32, 19, 0,
            -6, 11, 14, 29, 30, 18, 4, -14,
            5, 11, 16, 13, 16, 18, 13, 7,
            -6, 14, 14, 4, 10, 21, 25, 6,
            -18, -6, -13, -8, -33, -17, -20, 4,
        },
        {   // Rook
            47, 46, 92, 76, 80, 68, 36, 58,
            25, 14, 57, 67, 64, 67, -1, 38,
            -13, 17, 26, 58, 75, 42, 51, 14,
            -20, -14, 13, 14, 22, 5, -7, -21,
            -43, -40, -22, -17, -14, -24, -23, -37,
            -60, -38, -38, -41, -26, -46, -22, -52,
            -58, -43, -30, -31, -24, -20, -40, -72,
            -40, -34, -27, -12, -8, -26, -45, -44,
        },
        {   // Queen
            -48, 44, 64, 66, 60, 87, 3, -33,
            -9, -12, 4, 9, 17, 19, 19, 13,
            -26, -2, -1, 26, 35, 41, 28, 19,
            -29, -21, 2, 0, -2, 12, -8, 5,
            -17, -30, -11, -4, -1, -6, -10, -5,
            -34, -9, -10, -7, -6, -5, 5, -13,
            -32, -8, 1, -7, -4, 8, 0, -18,
            -23, -33, -16, -9, -14, -18, -28, -18,
        },
        {   // King
            120, 104, 234, 165, 78, -88, 169, -83,
            138, 62, 96, 151, -31, -54, 64, 211,
            52, 52, -15, 10, -12, 23, 21, 9,
            -78, -91, -46, -11, -53, -11, -50, -38,
            -96, -84, -100, -98, -109, -103, -79, -84,
            -15, -44, -70, -99, -85, -74, -29, -27,
            16, 2, -10, -48, -40, 4, 21, 24,
            0, 25, 18, -28, 8, -2, 53, 52,
        }
    },
    {   // Endgame
        {   // Pawn
            -177, -177, -177, -177, -177, -177, -177, -177,
            176, 164, 157, 132, 139, 152, 170, 171,
            101, 101, 83, 65, 56, 62, 96, 80,
            61, 52, 28, 28, 18, 22, 39, 47,
            39, 36, 22, 19, 15, 20, 32, 30,
            25, 34, 15, 25, 25, 23, 22, 19,
            35, 33, 30, 40, 32, 20, 18, 21,
            -177, -177, -177, -177, -177, -177, -177, -177,
        },
        {   // Knight
            -89, 24, 38, 28, -7, 81, 50, -123,
            -20, -1, -12, 54, 40, -16, 15, -32,
            -14, 5, 28, 20, 10, 26, 12, -3,
            -7, 20, 33, 37, 44, 36, 22, 2,
            -2, 13, 33, 38, 36, 30, 31, -6,
            -25, -1, 10, 22, 17, 7, 0, -26,
            -66, -2, -9, 4, 4, -8, -7, -40,
            -63, -48, -22, -24, -23, -42, -43, -86,
        },
        {   // Bishop
            9, 18, 12, 16, 25, 31, 23, -18,
            6, -3, -2, 16, 16, 5, 3, 2,
            12, 13, 8, 5, 10, 15, 17, -6,
            -11, 24, 16, 25, 24, 24, 11, -1,
            -3, 8, 22, 29, 23, 14, 12, -14,
            -13, 1, 16, 23, 18, 13, -4, -13,
            -41, -20, -6, -2, -3, -22, -14, -67,
            -48, -40, -42, -27, -15, -33, -56, -45,
        },
        {   // Rook
            7, 4, -16, -2, -10, -12, 1, -4,
            17, 25, 16, 17, 15, 1, 27, 10,
            30, 21, 20, 7, -5, 8, 5, 11,
            20, 23, 18, 14, 8, 15, 17, 10,
            8, 13, 8, 11, 1, 6, 7, -6,
            -8, -6, -5, -1, -10, 1, -16, -29,
            -35, -19, -17, -14, -19, -19, -28, -32,
            -17, -10, -5, -12, -15, -16, -6, -27,
        },
        {   // Queen
            33, -11, 1, -1, 23, -25, 37, 48,
            15, 24, 64, 49, 59, 74, 22, 19,
            15, 30, 60, 53, 73, 70, 63, 12,
            26, 47, 57, 80, 111, 70, 85, 32,
            -4, 42, 29, 60, 48, 48, 35, -7,
            -28, -38, 14, -7, 12, 14, -19, -51,
            -86, -85, -74, -44, -52, -90, -82, -115,
            -73, -93, -129, -102, -115, -120, -137, -65,
        },
        {   // King
            -148, -45, -72, -51, -51, 19, -44, -11,
            -45, 15, 9, -29, 44, 59, 19, -42,
            16, 42, 54, 38, 45, 49, 49, 32,
            36, 61, 49, 35, 41, 45, 54, 25,
            19, 41, 48, 42, 49, 45, 34, 16,
            -19, 9, 23, 35, 33, 23, 5, -16,
            -44, -25, -12, 4, 2, -14, -30, -46,
            -77, -60, -50, -50, -65, -65, -64, -88
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
