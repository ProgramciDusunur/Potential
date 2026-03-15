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
    { 64, 314, 358, 471, 969, 0, -64, -314, -358, -471, -969, 0 },
    { 82, 336, 362, 582, 1115, 0, -82, -336, -362, -582, -1115, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -73, -73, -73, -73, -73, -73, -73, -73,
            94, 74, 76, 97, 28, 8, 9, 87,
            11, 20, 35, 60, 44, 51, 43, 27,
            -6, 24, 20, 39, 44, 27, 30, -6,
            -17, 13, 12, 33, 31, 25, 21, -11,
            -14, 14, 11, 12, 19, 18, 42, -7,
            -27, 12, 2, -13, 1, 30, 41, -19,
            -73, -73, -73, -73, -73, -73, -73, -73,
        },
        {   // Knight
            -210, -178, -146, -57, -13, -276, -124, -142,
            -35, -3, 54, 2, 9, 67, -37, -23,
            2, 70, 61, 93, 106, 85, 80, -12,
            33, 31, 62, 70, 59, 73, 34, 52,
            3, 0, 42, 36, 43, 43, 17, 10,
            -10, 17, 19, 30, 41, 26, 28, -3,
            -20, -16, 9, 9, 11, 17, 7, -9,
            -65, -25, -12, -4, 2, -4, -12, -22,
        },
        {   // Bishop
            -65, -78, -101, -85, -70, -129, -101, -31,
            -21, 24, 7, -15, 10, -10, 2, -20,
            -8, 35, 29, 58, 47, 53, 22, 22,
            6, 12, 38, 38, 37, 36, 19, 14,
            -10, 5, 12, 30, 28, 18, 14, -4,
            10, 14, 14, 15, 12, 17, 12, 9,
            0, 15, 19, 6, 9, 23, 23, -11,
            3, 2, -14, 3, -23, -18, -8, -1,
        },
        {   // Rook
            48, 41, 71, 111, 86, 108, 53, 57,
            15, 23, 41, 86, 69, 69, 18, 30,
            -7, 24, 27, 41, 66, 48, 36, -3,
            -26, -27, 2, 10, 12, 15, -9, -18,
            -43, -57, -26, -21, -12, -22, -22, -40,
            -44, -35, -32, -37, -21, -38, -26, -51,
            -61, -38, -27, -29, -31, -21, -37, -78,
            -42, -36, -26, -13, -10, -28, -60, -52,
        },
        {   // Queen
            -88, -52, 29, 51, 37, 121, 7, -43,
            -16, -17, 15, -41, 32, 15, 15, 26,
            -21, 7, -15, 24, 40, 30, 24, 46,
            -9, -11, -1, 2, 6, 8, -3, 6,
            -12, -17, -2, 1, 3, 3, -7, -9,
            -16, -1, -1, -3, 0, -2, 9, -1,
            -25, 0, 12, 5, 6, 7, 8, -35,
            -1, -24, 2, 0, -1, -34, -36, -53,
        },
        {   // King
            134, 177, 162, 36, 158, 6, 67, -279,
            195, 115, 110, 198, 116, -8, -66, -2,
            -28, 31, 17, 48, 29, -65, -39, -17,
            -46, -18, -7, -6, -17, -38, -41, -99,
            -66, -47, -81, -66, -91, -66, -49, -94,
            -27, -27, -60, -87, -89, -72, -26, -49,
            47, 4, -9, -52, -43, 4, 23, 31,
            37, 40, 17, -35, 12, -11, 57, 53,
        }
    },
    {   // Endgame
        {   // Pawn
            -169, -169, -169, -169, -169, -169, -169, -169,
            136, 141, 127, 86, 90, 118, 128, 135,
            92, 90, 70, 57, 56, 42, 74, 72,
            61, 55, 35, 32, 21, 27, 39, 49,
            50, 49, 32, 26, 26, 27, 44, 36,
            40, 41, 29, 35, 36, 33, 32, 30,
            44, 42, 41, 39, 46, 33, 30, 30,
            -169, -169, -169, -169, -169, -169, -169, -169,
        },
        {   // Knight
            -114, 25, 28, 20, 7, 144, 11, -110,
            3, 0, -2, 63, 30, -1, 14, -8,
            -2, 2, 23, 26, 12, 33, 12, -6,
            -18, 23, 28, 26, 38, 31, 20, -14,
            -10, 19, 27, 38, 33, 22, 22, -12,
            -26, 2, 13, 18, 12, 11, -2, -29,
            -47, -9, -17, 2, 3, -14, -21, -48,
            -43, -42, -29, -26, -34, -40, -47, -71,
        },
        {   // Bishop
            26, 25, 27, 26, 31, 34, 34, -16,
            4, 1, 12, 15, 9, 10, 1, 1,
            -6, 7, 16, 3, 6, 4, 11, -8,
            -5, 26, 13, 15, 16, 17, 9, -3,
            -2, 10, 22, 21, 18, 11, -3, -19,
            -12, -1, 16, 16, 18, 6, -12, -19,
            -37, -20, -12, -7, -4, -15, -18, -49,
            -46, -37, -38, -21, -19, -33, -37, -41,
        },
        {   // Rook
            0, 7, -5, -23, -5, -26, -5, -2,
            17, 17, 14, -2, 8, -10, 15, 11,
            24, 13, 13, 14, -4, 2, 7, 15,
            22, 32, 22, 21, 12, 14, 13, 11,
            8, 22, 12, 15, 6, 2, -3, -4,
            -24, -9, 1, -1, -10, -5, -14, -18,
            -22, -22, -14, -8, -10, -25, -24, -27,
            -15, -4, -1, -7, -11, -11, 2, -23,
        },
        {   // Queen
            86, 83, 28, 0, 11, -55, 24, 56,
            -5, 20, 25, 112, 39, 53, 20, -13,
            15, 3, 60, 46, 46, 62, 29, -23,
            -3, 16, 48, 59, 86, 63, 43, 22,
            -27, 7, 10, 53, 29, 32, 40, 19,
            -28, -36, -12, -3, 4, 16, -7, -55,
            -39, -58, -72, -47, -53, -43, -114, -57,
            -103, -79, -113, -84, -90, -58, -69, -15,
        },
        {   // King
            -229, -59, -53, -49, -61, -24, 4, -61,
            -89, -7, -9, -46, 18, 41, 60, -39,
            23, 32, 35, 25, 25, 64, 53, 36,
            14, 35, 32, 25, 32, 46, 54, 36,
            6, 25, 43, 39, 48, 41, 31, 22,
            -6, 15, 28, 40, 41, 34, 15, 8,
            -31, -5, 8, 23, 18, 3, -9, -33,
            -73, -45, -26, -23, -44, -38, -50, -69
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
