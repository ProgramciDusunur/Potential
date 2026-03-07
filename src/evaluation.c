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
    { 296, 1011, 1150, 1256, 2902, 0, -296, -1011, -1150, -1256, -2902, 0 },
    { 253, 826, 886, 1679, 3191, 0, -253, -826, -886, -1679, -3191, 0 }
};

// Tuned PSQT tables ÔÇö paste into evaluation.c
const int positional_score[2][6][64] = {
    {   // Opening
        {   // Pawn
            -59, -59, -59, -59, -59, -59, -59, -59,
            125, 69, 52, 126, 76, 15, 44, 246,
            -120, -50, 3, 70, 82, 26, -56, 27,
            -119, 8, 11, 85, 121, 15, 77, -155,
            -167, -22, -8, 107, 60, 12, 25, -177,
            -114, 39, 17, 23, 86, -23, 148, -96,
            -86, 89, 12, -42, 4, 106, 210, -99,
            -59, -59, -59, -59, -59, -59, -59, -59,
        },
        {   // Knight
            -467, -314, -536, -390, -20, -683, -132, -287,
            -163, -71, 371, -529, -239, 277, 53, -87,
            -50, 159, -15, 211, 245, -138, 212, 92,
            104, 151, 150, 264, 191, 223, 122, 162,
            36, -43, 159, 107, 135, 130, 76, 27,
            57, 84, 149, -5, 61, 151, 97, -6,
            22, 47, 73, 88, 84, 78, -47, 17,
            -183, -6, -56, -45, 22, -20, -7, -160,
        },
        {   // Bishop
            -82, -355, -502, -664, -199, -811, -506, -30,
            -56, 128, 76, -706, -458, 70, 125, -48,
            -223, 103, -249, 78, -3, -400, -13, 9,
            21, 149, 94, 136, 168, 144, 160, 37,
            37, 28, 148, 115, 126, 124, 43, 48,
            161, 178, 166, 165, 162, 179, 205, 134,
            12, 208, 195, 133, 150, 267, 247, 84,
            -59, 72, 80, 38, -31, 62, 130, 152,
        },
        {   // Rook
            20, 141, -49, -1060, 136, -486, 9, 179,
            150, 131, 146, 157, 204, 208, 84, 213,
            159, -22, -50, 245, 56, -61, -2, 175,
            73, 37, 133, 59, 31, 103, 46, 120,
            -20, -212, -94, -181, -125, -176, -135, 25,
            -43, -137, -149, -329, 8, -97, -63, -1,
            -59, -1, -193, 40, 15, -11, 27, -141,
            24, 48, 120, 178, 203, 122, -22, 43,
        },
        {   // Queen
            -59, 83, 215, -418, -379, 78, 24, -22,
            -62, 41, 26, -235, 78, 117, 148, 45,
            21, -82, 36, 60, 72, 144, 42, 79,
            4, 37, 42, 52, 109, -4, 19, -21,
            54, -46, 41, 2, 25, 16, -105, 7,
            69, 95, 16, 28, -15, 31, 44, -29,
            54, 68, 116, 71, 87, 58, 60, -164,
            -33, -5, 68, 68, 23, 15, -369, -587,
        },
        {   // King
            15, 103, 96, 65, 24, 46, 82, 93,
            99, 79, 46, 73, 26, 76, 42, 51,
            22, 104, 82, 64, 129, 253, 274, 14,
            -99, 45, 68, 206, 135, 172, 38, 44,
            -234, -40, -252, -269, -212, -241, -224, -194,
            -228, -119, -300, -363, -359, -234, -45, -83,
            -8, -116, -128, -163, -69, 110, 127, 81,
            -108, 83, 129, -71, 187, 81, 360, 290,
        }
    },
    {   // Endgame
        {   // Pawn
            -199, -199, -199, -199, -199, -199, -199, -199,
            472, 494, 443, 358, 226, 573, 386, 198,
            195, 182, 109, 71, 31, 103, 48, 38,
            71, 14, 4, -54, -107, -74, -43, 35,
            64, 14, -16, -66, -66, 4, -82, 43,
            -62, -21, -36, 13, -7, 4, -73, -61,
            -128, -69, -17, 70, 83, -48, -65, -97,
            -199, -199, -199, -199, -199, -199, -199, -199,
        },
        {   // Knight
            -520, -69, 32, 18, 59, 8, -224, -337,
            -89, 16, -131, 207, 85, 45, -58, -170,
            67, -19, 93, 126, 149, 172, 3, -33,
            3, -6, 137, 104, 127, 139, 110, 83,
            -85, 71, 97, 71, 85, 148, 100, 42,
            -30, 47, 9, 113, 168, 27, 62, -65,
            -69, -83, 41, 38, 59, -121, -55, -66,
            -190, -27, 55, -142, -31, -23, -112, -225,
        },
        {   // Bishop
            -7, -86, 143, 52, 69, -26, -245, 54,
            -84, -67, 47, -29, 142, -37, 0, -103,
            215, 83, 142, 89, 57, 228, 104, 215,
            24, 82, 77, 66, 80, 86, 77, 146,
            -18, 85, 65, 108, -3, 88, 23, 4,
            -122, 12, 17, 27, 55, -8, -63, 62,
            -79, -160, -106, -24, -32, -74, -159, -62,
            -293, -173, -40, -70, -203, -108, -245, -133,
        },
        {   // Rook
            87, 72, 53, 364, -36, 166, 72, 62,
            16, 69, 85, 106, 80, 59, 58, 7,
            1, 113, 90, 34, 57, 22, 75, -20,
            60, 124, 35, 83, 51, 54, 39, -6,
            10, 30, 197, 72, 36, 113, 140, 5,
            -30, -9, -38, 18, -175, -97, -84, -47,
            -249, -86, -31, -98, -203, -175, -317, -27,
            -152, -117, -58, -129, -126, -120, -57, -266,
        },
        {   // Queen
            126, 10, 90, 79, 2, -5, 44, 185,
            -2, -50, 94, 360, 199, 106, 58, 95,
            -33, 42, 104, 234, 171, 190, 89, 104,
            67, 59, 152, 257, 144, 326, 423, 419,
            46, 123, 109, 277, 195, 227, 424, 304,
            -198, -145, 147, 48, 189, 116, 162, 55,
            -319, -122, -186, -64, -158, -220, -283, -234,
            -575, -829, -680, -351, -250, -514, -1106, -282,
        },
        {   // King
            -133, -296, -188, -310, -191, -190, -201, -45,
            -318, -54, -4, -17, 44, 105, 3, -17,
            -197, 100, 208, 166, 184, 56, 25, -28,
            -181, 9, 161, 84, 81, 79, 139, -14,
            -211, 99, 166, 151, 149, 149, 172, 109,
            -89, 171, 152, 254, 245, 214, 127, 35,
            0, 28, 97, 100, 149, -43, -55, -50,
            -154, -44, -155, -84, -148, -166, -211, -163
        }
    }
};






// Pawn Penalties and Bonuses
const int double_pawn_penalty_opening = -5;
const int double_pawn_penalty_endgame = -10;
const int isolated_pawn_penalty_opening = -5;
const int isolated_pawn_penalty_endgame = -10;

// passed pawn bonus
const int passed_pawn_bonus_middle[64] = { 0, 0, 0, 0, 0, 0, 0, 0,
                                                  36, 42, 42, 42, 42, 42, 42, 36,
                                                  14, 17, 17, 17, 17, 17, 17, 14,
                                                  5, 7, 7, 7, 7, 7, 7, 5,
                                                  0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 0,};

const int passed_pawn_bonus_endgame[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                  80, 85, 90, 103, 103, 90, 85, 80,
                                                  20, 27, 30, 34, 34, 30, 27, 20,
                                                  10, 12, 15, 20, 20, 15, 12, 10,
                                                  0, 10, 10, 10, 10, 10, 10, 0,
                                                  -3, 0, 0, 0, 0, 0, 0, 0,
                                                  -2, 0, 0, 0, 0, 0, -1, -2,
                                                  0, 0, 0, 0, 0, 0, 0, 0};

// Pawn Hole Bonus [square]
const int pawnHoleBonus[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 2, 2, 1, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
};

// Pawn hole knight check [square]
const bool pawnHoleSquareCheck[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
};


// Knight Evaluation
const int knightOutpost[2][64] = {
        {   0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 1, 1, 1, 1, 0, 0,
                0, 1, 3, 3, 3, 3, 1, 0,
                0, 2, 1, 6, 3, 4, 2, 0,
                0, 0, 3, 0, 3, 3, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
        },
        {   0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 3, 0, 0, 0, 0,
                0, 2, 2, 3, 6, 1, 2, 0,
                0, 1, 0, 3, 2, 0, 1, 0,
                0, 0, 1, 0, 1, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
        },
};

// File and Mobility Scores
const int semi_open_file_score = 10;
const int open_file_score = 15;
const int king_semi_open_file_score = 10;
const int king_open_file_score = 20;
const int rook_open_file = 10;
const int bishop_unit = 4;
const int queen_unit = 9;

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

// Passed Can Move Bonus
const int passedCanMoveBonus = 5;

// Bishop Pair Bonus
const int bishop_pair_bonus_midgame = 8;
const int bishop_pair_bonus_endgame = 48;

const int bishop_pair_bonus[] = {0, 8, 15, 23, 30, 38};

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
    int passed_pawn_count = 0;
    
    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = position->bitboards[piece];
        while (bitboard) {
            const int square = getLS1BIndex(bitboard);
            score += packed_table[piece][square];

            switch (piece) {
                case P:
                    if ((whitePassedMasks[square] & position->bitboards[p]) == 0) {
                        passed_pawn_count++;
                        if (!(getBit(position->occupancies[both], (square - 8))))
                            score += S(passedCanMoveBonus, passedCanMoveBonus);
                    }
                    break;
                case p:
                    if ((blackPassedMasks[square] & position->bitboards[P]) == 0) {
                        passed_pawn_count--;
                        if (!(getBit(position->occupancies[both], (square + 8))))
                            score -= S(passedCanMoveBonus, passedCanMoveBonus);
                    }
                    break;
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
    
    if (countBits(position->bitboards[B]) == 2) score += S(bishop_pair_bonus_midgame, bishop_pair_bonus_endgame);
    if (countBits(position->bitboards[b]) == 2) score -= S(bishop_pair_bonus_midgame, bishop_pair_bonus_endgame);

    // Winnable Score
    int winnable = 6 * passed_pawn_count + 8 * (countBits(position->bitboards[P]) - countBits(position->bitboards[p]));
    score += S(winnable, winnable);

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
