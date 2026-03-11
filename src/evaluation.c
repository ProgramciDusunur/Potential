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
    { 75, 362, 409, 541, 1112, 0, -75, -362, -409, -541, -1112, 0 },
    { 108, 387, 417, 675, 1307, 0, -108, -387, -417, -675, -1307, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -76, -76, -76, -76, -76, -76, -76, -76,
            136, 114, 80, 107, 80, 27, 6, 36,
            9, 26, 30, 67, 65, 54, 28, 19,
            -16, 21, 17, 38, 46, 31, 32, -5,
            -22, 9, 6, 34, 28, 21, 21, -16,
            -21, 11, 10, 9, 22, 16, 44, -10,
            -35, 7, -2, -18, -5, 31, 43, -23,
            -76, -76, -76, -76, -76, -76, -76, -76,
        },
        {   // Knight
            -210, -210, -146, -39, 69, -279, -216, -152,
            -34, -3, 70, 40, 13, 79, -12, -34,
            11, 80, 65, 98, 121, 107, 82, -2,
            15, 34, 56, 82, 64, 75, 33, 50,
            -2, 14, 36, 42, 47, 42, 32, -8,
            -13, 6, 20, 32, 41, 24, 24, -9,
            -32, -24, 10, 5, 8, 20, -20, -12,
            -45, -31, -22, -8, -9, -12, -22, -38,
        },
        {   // Bishop
            -66, -102, -52, -79, -76, -168, -107, -21,
            -34, 35, 0, -37, -9, -18, 24, -20,
            -27, 31, 31, 60, 48, 69, 31, 22,
            0, 18, 47, 46, 36, 42, 16, 13,
            -11, 9, 15, 38, 32, 19, 11, -17,
            3, 16, 14, 18, 16, 18, 6, 10,
            2, 15, 20, 0, 11, 25, 28, 8,
            -10, 1, -16, -6, -9, -20, 1, 0,
        },
        {   // Rook
            48, 82, 138, 135, 112, 113, 84, 59,
            21, 25, 67, 89, 93, 59, 15, 34,
            -6, 18, 33, 50, 62, 71, 39, 15,
            -30, -27, 2, 12, -1, 9, -20, -18,
            -54, -51, -39, -24, -18, -38, -42, -45,
            -66, -40, -42, -46, -35, -49, -36, -65,
            -73, -50, -24, -35, -37, -26, -39, -94,
            -49, -43, -36, -14, -13, -34, -66, -59,
        },
        {   // Queen
            -63, 15, 81, 67, 46, 62, 7, -37,
            -22, -17, -2, -4, 13, 32, 10, 12,
            -25, 12, -4, 33, 41, 29, 15, 20,
            -10, -15, -1, 3, -6, 4, -21, -2,
            -10, -22, -12, -1, -8, -4, -12, -25,
            -23, 2, -2, -4, -6, -3, 11, -15,
            -17, -5, 12, 1, 4, 21, 14, -24,
            -33, -35, -8, 0, -8, -29, -10, -20,
        },
        {   // King
            86, 93, 215, 148, 273, 34, 129, -103,
            332, 183, 59, 207, 42, -16, 74, 122,
            -3, -30, -73, -38, -19, -32, 47, 39,
            -100, -52, -93, -85, -75, -2, -60, -107,
            -76, -78, -121, -114, -130, -127, -99, -121,
            -18, -17, -75, -108, -104, -78, -29, -53,
            30, 2, -10, -60, -51, 4, 33, 39,
            10, 36, 22, -41, 15, -8, 69, 64,
        }
    },
    {   // Endgame
        {   // Pawn
            -176, -176, -176, -176, -176, -176, -176, -176,
            169, 153, 153, 111, 108, 139, 173, 179,
            104, 99, 86, 58, 51, 47, 100, 77,
            64, 60, 33, 26, 15, 21, 43, 50,
            43, 44, 27, 18, 14, 18, 36, 33,
            30, 39, 16, 24, 25, 26, 23, 23,
            40, 39, 34, 32, 32, 23, 26, 26,
            -176, -176, -176, -176, -176, -176, -176, -176,
        },
        {   // Knight
            -102, 46, 37, 16, -38, 150, 41, -142,
            -19, -10, -8, 59, 24, -8, 0, -17,
            -5, 6, 29, 27, 19, 11, 24, -5,
            9, 22, 42, 35, 41, 36, 27, -6,
            6, 17, 36, 40, 31, 26, 7, -1,
            -21, 3, 3, 19, 25, 12, 2, -39,
            -45, -3, -10, 1, -4, -21, -3, -27,
            -76, -54, -50, -37, -14, -27, -49, -87,
        },
        {   // Bishop
            20, 28, 16, 28, 18, 36, 41, -72,
            -17, -1, 11, 38, 22, 21, -7, -6,
            9, 18, 18, 8, 6, 12, 13, -8,
            -7, 24, 10, 20, 31, 18, 23, -5,
            -16, 4, 29, 27, 22, 19, 5, -11,
            -16, 8, 21, 12, 20, 10, -11, -22,
            -20, -27, -12, -2, -7, -6, -22, -50,
            -44, -48, -45, -30, -29, -37, -27, -63,
        },
        {   // Rook
            1, -4, -28, -23, -24, -30, -18, -5,
            19, 22, 16, 13, 7, 7, 22, 11,
            26, 19, 22, 12, 5, -4, 14, 12,
            22, 26, 19, 24, 22, 13, 23, 7,
            10, 16, 22, 16, 7, 8, 11, -12,
            -7, -16, -1, 1, -4, -2, -10, -21,
            -27, -23, -19, -12, -12, -22, -36, -36,
            -15, -6, 1, -11, -15, -13, 3, -29,
        },
        {   // Queen
            69, -29, -37, 2, 38, 26, 51, 65,
            14, 36, 32, 69, 73, 77, 17, -16,
            12, 4, 47, 23, 62, 102, 99, 20,
            -7, 18, 38, 94, 117, 107, 126, 55,
            -38, 35, 26, 73, 53, 62, 56, 54,
            -54, -38, -14, -2, 12, 20, -25, -31,
            -120, -93, -97, -60, -68, -102, -112, -107,
            -40, -88, -123, -111, -127, -98, -149, -100,
        },
        {   // King
            -216, -55, -84, -44, -69, -35, -9, -64,
            -120, -21, 15, -38, 30, 63, 49, -31,
            24, 52, 63, 54, 41, 72, 62, 18,
            21, 59, 63, 50, 45, 45, 64, 36,
            5, 37, 50, 49, 54, 55, 38, 25,
            -20, 7, 25, 38, 39, 32, 8, -3,
            -23, -15, 0, 14, 18, -6, -25, -44,
            -72, -58, -44, -44, -67, -51, -67, -93
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
