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

// Material Scores
const int material_score[2][12] =
        {
                // Opening material score
                {82, 337, 365, 477, 1025, 0, -82, -337, -365, -477, -1025, 0},

                // Endgame material score
                {94, 281, 297, 512, 936, 0, -94, -281, -297, -512, -936, 0}
        };

// SEE Material Array
const int seeMaterial[12] = {100, 300, 300, 500, 900, 12000, -100, -300, -300, -500, -900, -12000};

/// positional piece scores [game phase][piece][square]
const int positional_score[2][6][64] = {
        {   // Opening positional piece scores
                {   // pawn
                        0, 0, 0, 0, 0, 0, 0, 0,
                        98, 134, 61, 95, 68, 126, 34, -11,
                        -6, 7, 26, 31, 65, 56, 25, -20,
                        -14, 13, 6, 21, 23, 12, 17, -23,
                        -27, -2, -5, 12, 17, 6, 10, -25,
                        -26, -4, -4, -10, 3, 3, 33, -12,
                        -35, -1, -20, -23, -15, 24, 38, -22,
                        0, 0, 0, 0, 0, 0, 0, 0
                },
                {   // knight
                        -167, -89, -34, -49, 61, -97, -15, -107,
                        -73, -41, 72, 36, 23, 62, 7, -17,
                        -47, 60, 37, 65, 84, 129, 73, 44,
                        -9, 17, 19, 53, 37, 69, 18, 22,
                        -13, 4, 16, 13, 28, 19, 21, -8,
                        -23, -9, 12, 10, 19, 17, 25, -16,
                        -29, -53, -12, -3, -1, 18, -14, -19,
                        -105, -21, -58, -33, -17, -28, -19, -23
                },
                {   // bishop
                        -29, 4, -82, -37, -25, -42, 7, -8,
                        -26, 16, -18, -13, 30, 59, 18, -47,
                        -16, 37, 43, 40, 35, 50, 37, -2,
                        -4, 5, 19, 50, 37, 37, 7, -2,
                        -6, 13, 13, 26, 34, 12, 10, 4,
                        0, 15, 15, 15, 14, 27, 18, 10,
                        4, 15, 16, 0, 7, 21, 33, 1,
                        -33, -3, -14, -21, -13, -12, -39, -21
                },
                {   // rook
                        32, 42, 32, 51, 63, 9, 31, 43,
                        27, 32, 58, 62, 80, 67, 26, 44,
                        -5, 19, 26, 36, 17, 45, 61, 16,
                        -24, -11, 7, 26, 24, 35, -8, -20,
                        -36, -26, -12, -1, 9, -7, 6, -23,
                        -45, -25, -16, -17, 3, 0, -5, -33,
                        -44, -16, -20, -9, -1, 11, -6, -71,
                        -19, -13, 1, 17, 16, 7, -37, -26
                },
                {   // queen
                        -28, 0, 29, 12, 59, 44, 43, 45,
                        -24, -39, -5, 1, -16, 57, 28, 54,
                        -13, -17, 7, 8, 29, 56, 47, 57,
                        -27, -27, -16, -16, -1, 17, -2, 1,
                        -9, -26, -9, -10, -2, -4, 3, -3,
                        -14, 2, -11, -2, -5, 2, 14, 5,
                        -35, -8, 11, 2, 8, 15, -3, 1,
                        -1, -18, -9, 10, -15, -25, -31, -50
                },
                {   // king
                        -65, 23, 16, -15, -56, -34, 2, 13,
                        29, -1, -20, -7, -8, -4, -38, -29,
                        -9, 24, 2, -16, -20, 6, 22, -22,
                        -17, -20, -12, -27, -30, -25, -14, -36,
                        -49, -1, -27, -39, -46, -44, -33, -51,
                        -14, -14, -22, -46, -44, -30, -15, -27,
                        1, 7, -8, -64, -43, -16, 9, 8,
                        -15, 36, 12, -54, 8, -28, 24, 14
                }
        },
        {   // Endgame positional piece scores
                {   // pawn
                        0, 0, 0, 0, 0, 0, 0, 0,
                        178, 173, 158, 134, 147, 132, 165, 187,
                        94, 100, 85, 67, 56, 53, 82, 84,
                        32, 24, 13, 5, -2, 4, 17, 17,
                        13, 9, -3, -7, -7, -8, 3, -1,
                        4, 7, -6, 1, 0, -5, -1, -8,
                        13, 8, 8, 10, 13, 0, 2, -7,
                        0, 0, 0, 0, 0, 0, 0, 0
                },
                {   // knight
                        -58, -38, -13, -28, -31, -27, -63, -99,
                        -25, -8, -25, -2, -9, -25, -24, -52,
                        -24, -20, 10, 9, -1, -9, -19, -41,
                        -17, 3, 22, 22, 22, 11, 8, -18,
                        -18, -6, 16, 25, 16, 17, 4, -18,
                        -23, -3, -1, 15, 10, -3, -20, -22,
                        -42, -20, -10, -5, -2, -20, -23, -44,
                        -29, -51, -23, -15, -22, -18, -50, -64
                },
                {   // bishop
                        -14, -21, -11, -8, -7, -9, -17, -24,
                        -8, -4, 7, -12, -3, -13, -4, -14,
                        2, -8, 0, -1, -2, 6, 0, 4,
                        -3, 9, 12, 9, 14, 10, 3, 2,
                        -6, 3, 13, 19, 7, 10, -3, -9,
                        -12, -3, 8, 10, 13, 3, -7, -15,
                        -14, -18, -7, -1, 4, -9, -15, -27,
                        -23, -9, -23, -5, -9, -16, -5, -17
                },
                {   // rook
                        13, 10, 18, 15, 12, 12, 8, 5,
                        11, 13, 13, 11, -3, 3, 8, 3,
                        7, 7, 7, 5, 4, -3, -5, -3,
                        4, 3, 13, 1, 2, 1, -1, 2,
                        3, 5, 8, 4, -5, -6, -8, -11,
                        -4, 0, -5, -1, -7, -12, -8, -16,
                        -6, -6, 0, 2, -9, -9, -11, -3,
                        -9, 2, 3, -1, -5, -13, 4, -20
                },
                {   // queen
                        -9, 22, 22, 27, 27, 19, 10, 20,
                        -17, 20, 32, 41, 58, 25, 30, 0,
                        -20, 6, 9, 49, 47, 35, 19, 9,
                        3, 22, 24, 45, 57, 40, 57, 36,
                        -18, 28, 19, 47, 31, 34, 39, 23,
                        -16, -27, 15, 6, 9, 17, 10, 5,
                        -22, -23, -30, -16, -16, -23, -36, -32,
                        -33, -28, -22, -43, -5, -32, -20, -41
                },
                {   // king
                        -74, -35, -18, -18, -11, 15, 4, -17,
                        -12, 17, 14, 17, 17, 38, 23, 11,
                        10, 17, 23, 15, 20, 45, 44, 13,
                        -8, 22, 24, 27, 26, 33, 26, 3,
                        -18, -4, 21, 24, 27, 23, 9, -11,
                        -19, -3, 11, 21, 23, 16, 7, -9,
                        -27, -11, 4, 13, 14, 4, -5, -17,
                        -53, -34, -21, -11, -28, -14, -24, -43
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
    for (int piece = N; piece <= Q; piece++) {
        phase_scores[piece - N] = phase_scores[piece - N + 4] = material_score[opening][piece];
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


int64_t phase_scores[8];

#ifdef __x86_64__
#include <immintrin.h>
#endif

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


#if defined(__AVX512F__) && defined(__AVX512VPOPCNTDQ__)
    __m512i material_values = _mm512_loadu_epi64(phase_scores); 

    __m256i white_pieces = _mm256_loadu_si256((const __m256i *)&position->bitboards[N]);
    __m256i black_pieces = _mm256_loadu_si256((const __m256i *)&position->bitboards[n]);

    __m512i pieces = _mm512_inserti64x4(_mm512_castsi256_si512(white_pieces), black_pieces, 1);
    __m512i popcounts = _mm512_popcnt_epi64(pieces);

    return _mm512_reduce_add_epi64(_mm512_mullo_epi64(popcounts, material_values));
#endif

    // white & black game phase scores
    int white_piece_scores = 0, black_piece_scores = 0;

    // loop over white pieces
    for (int piece = N; piece <= Q; piece++)
        white_piece_scores += countBits(position->bitboards[piece]) * phase_scores[piece - N];


    // loop over white pieces
    for (int piece = n; piece <= q; piece++)
        black_piece_scores += countBits(position->bitboards[piece]) * phase_scores[piece - n];



    // return game phase score
    return white_piece_scores + black_piece_scores;
}


int evaluate(board* position) {
    const int game_phase_score = get_game_phase_score(position);
    Score score = S(0, 0);

    // Tehdit bitboardlarını sıfırla ve doldur
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


void clearStaticEvaluationHistory(board* position) {
    for (int i = 0;i < 64;i++) {
        position->staticEval[i] = noEval;
    }
}
