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
    { 71, 353, 399, 527, 1107, 0, -71, -353, -399, -527, -1107, 0 },
    { 106, 371, 405, 657, 1268, 0, -106, -371, -405, -657, -1268, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -73, -73, -73, -73, -73, -73, -73, -73,
            82, 51, 70, 108, 87, 13, -28, 33,
            6, 27, 40, 65, 60, 36, 44, 28,
            -6, 25, 24, 44, 53, 33, 31, -8,
            -18, 12, 12, 38, 31, 20, 23, -8,
            -15, 10, 12, 16, 22, 18, 46, -2,
            -31, 9, 0, -13, 0, 33, 44, -14,
            -73, -73, -73, -73, -73, -73, -73, -73,
        },
        {   // Knight
            -210, -130, -139, -77, 31, -239, -141, -155,
            -28, 9, 67, 8, 36, 66, -4, -35,
            -27, 84, 92, 103, 134, 100, 93, 0,
            10, 27, 47, 79, 63, 72, 31, 58,
            -9, -6, 35, 33, 39, 40, 20, -2,
            -16, 0, 16, 30, 42, 19, 19, -15,
            -36, -30, 10, 0, 6, 16, 2, -25,
            -34, -37, -28, -11, -7, -22, -27, -44,
        },
        {   // Bishop
            -78, -88, -28, -77, -12, -165, -126, -11,
            -34, 19, 19, -43, 9, -13, 35, -34,
            -15, 38, 31, 77, 49, 50, 24, 27,
            -5, 14, 52, 35, 46, 31, 17, 11,
            -11, 14, 12, 35, 34, 18, 21, -18,
            1, 8, 14, 20, 13, 24, 9, 7,
            2, 10, 20, -6, 14, 21, 29, 19,
            -10, -37, -22, -24, -33, -20, -21, 3,
        },
        {   // Rook
            52, 83, 61, 130, 151, 114, 47, 86,
            28, 21, 60, 103, 121, 112, 23, 13,
            -16, 23, 25, 56, 81, 64, 53, 16,
            -34, -45, -16, 0, 25, 3, -2, -27,
            -51, -61, -44, -27, -28, -33, -24, -47,
            -79, -44, -49, -53, -57, -55, -25, -66,
            -73, -35, -26, -31, -32, -27, -38, -94,
            -53, -43, -36, -16, -12, -33, -59, -61,
        },
        {   // Queen
            -80, -47, 52, 71, 28, 63, 41, 1,
            -32, -21, -11, 6, 7, 16, 14, 4,
            -29, 8, -8, 13, 55, 64, 25, 22,
            -11, -17, -8, 2, 12, 14, -9, 8,
            -2, -29, -5, -1, 0, 3, -9, -16,
            -22, 4, 6, -8, -4, -4, 10, -9,
            -30, -2, 11, 1, 4, 22, 6, -42,
            -26, -16, -7, 1, -12, -24, -6, -45,
        },
        {   // King
            100, 159, 152, 41, 273, 83, 28, -274,
            210, 33, 184, 286, 279, 96, -89, 19,
            15, 11, -33, 24, 23, 1, 51, -18,
            -114, -82, -166, -61, -49, -18, -55, -75,
            -123, -81, -128, -131, -129, -95, -98, -106,
            -3, -35, -109, -112, -98, -67, -38, -24,
            32, 9, 8, -44, -37, 17, 39, 49,
            54, 48, 30, -25, 24, 0, 73, 66,
        }
    },
    {   // Endgame
        {   // Pawn
            -176, -176, -176, -176, -176, -176, -176, -176,
            182, 202, 163, 107, 77, 141, 190, 183,
            104, 99, 73, 68, 57, 48, 85, 79,
            65, 50, 34, 18, 12, 18, 36, 51,
            45, 35, 26, 18, 14, 19, 38, 37,
            31, 37, 22, 24, 17, 22, 22, 26,
            42, 39, 33, 20, 35, 22, 23, 27,
            -176, -176, -176, -176, -176, -176, -176, -176,
        },
        {   // Knight
            -33, 6, 21, 28, -7, 133, 41, -153,
            -21, -8, -1, 62, 34, 11, 8, 3,
            -8, -3, 21, 39, 9, 10, 14, 10,
            -13, 24, 37, 34, 37, 44, 36, -24,
            -3, 21, 30, 41, 42, 33, 6, 18,
            -36, -3, 12, 19, 14, 14, 3, -18,
            -71, -12, -7, 14, 3, -9, -6, -47,
            -112, -44, -31, -41, -26, -27, -42, -127,
        },
        {   // Bishop
            35, 30, 7, 40, 1, 42, 19, -40,
            14, -17, 2, 18, 8, -2, 16, -20,
            -5, 8, 23, -14, 0, 24, 24, -10,
            1, 23, 4, 19, 25, 22, 12, -14,
            -1, -1, 15, 30, 28, 22, -4, 3,
            -13, 3, 18, 19, 22, 2, -13, -19,
            -30, -11, -10, 11, -5, -8, -21, -40,
            -55, -42, -32, -21, -17, -37, -30, -58,
        },
        {   // Rook
            10, 4, 12, -16, -21, -14, 5, -5,
            18, 22, 20, 1, -10, -13, 11, 18,
            30, 15, 22, 9, -5, -3, 1, 5,
            21, 34, 27, 25, 13, 14, 11, 21,
            -1, 20, 17, 18, 6, 18, 0, -9,
            -7, -14, -8, -6, 6, 7, -27, -20,
            -26, -36, -22, -16, -10, -21, -24, -26,
            -12, -14, 5, -10, -19, -17, -1, -32,
        },
        {   // Queen
            85, 74, -22, -32, 56, 30, 8, 2,
            -7, 26, 75, 59, 84, 77, 38, 47,
            23, 1, 47, 74, 67, 55, 62, 9,
            -5, -1, 64, 76, 81, 81, 76, 29,
            -76, 36, 10, 73, 50, 42, 57, 1,
            -56, -59, -45, 14, 7, 26, -36, -7,
            -53, -80, -102, -39, -45, -93, -104, -107,
            -47, -201, -103, -114, -69, -66, -97, -55,
        },
        {   // King
            -263, -94, -80, -46, -66, -18, 6, 9,
            -51, 2, -32, -48, -13, 16, 62, 13,
            1, 49, 59, 35, 43, 71, 39, 25,
            26, 54, 73, 49, 52, 44, 60, 30,
            20, 34, 55, 57, 52, 45, 46, 15,
            -28, 20, 39, 48, 44, 31, 18, -2,
            -36, -18, -6, 15, 21, -5, -20, -42,
            -122, -59, -39, -39, -54, -48, -59, -88
        }
    }
};

// File and Mobility Scores
const int semi_open_file_score = 10;
const int open_file_score = 15;
const int king_semi_open_file_score = 10;
const int king_open_file_score = 20;
const int rook_open_file = 10;

// Mobility Bonuses
const int bishop_mobility_middlegame = 5;
const int bishop_mobility_endgame = 10;
const int queen_mobility_middlegame = 1;
const int queen_mobility_endgame = 2;

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
                case B: score += S(countBits(getBishopAttacks(square, position->occupancies[both])), countBits(getBishopAttacks(square, position->occupancies[both]))); break;
                case b: score -= S(countBits(getBishopAttacks(square, position->occupancies[both])), countBits(getBishopAttacks(square, position->occupancies[both]))); break;
                case R:
                    if ((position->bitboards[P] & fileMasks[square]) == 0) score += S(semi_open_file_score, semi_open_file_score);
                    if (((position->bitboards[P] | position->bitboards[p]) & fileMasks[square]) == 0) score += S(rook_open_file, rook_open_file);
                    break;
                case r:
                    if ((position->bitboards[p] & fileMasks[square]) == 0) score -= S(semi_open_file_score, semi_open_file_score);
                    if (((position->bitboards[P] | position->bitboards[p]) & fileMasks[square]) == 0) score -= S(rook_open_file, rook_open_file);
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
