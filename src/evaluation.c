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

// Paketlenmiş Materyal Skorları
const int packed_material_score[6] = {
    S(82, 94),    // (P)
    S(337, 281),  // (N)
    S(365, 297),  // (B)
    S(477, 512),  // (R)
    S(1025, 936), // (Q)
    S(0, 0)       // (K)
};

const int opening_material_score[12] = {
        82, 337, 365, 477, 1025, 0, -82, -337, -365, -477, -1025, 0
};

// Material Scores
const int material_score[2][12] = {
        // Opening material score
        {82, 337, 365, 477, 1025, 0, -82, -337, -365, -477, -1025, 0},

        // Endgame material score
        {94, 281, 297, 512, 936, 0, -94, -281, -297, -512, -936, 0}
};

// SEE Material Array
const int seeMaterial[12] = {100, 300, 300, 500, 900, 12000, -100, -300, -300, -500, -900, -12000};

/// positional piece scores [piece][square]
const int packed_positional_score[6][64] = {
    { // pawn
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(98, 178), S(134, 173), S(61, 158), S(95, 134), S(68, 147), S(126, 132), S(34, 165), S(-11, 187),
        S(-6, 94), S(7, 100), S(26, 85), S(31, 67), S(65, 56), S(56, 53), S(25, 82), S(-20, 84),
        S(-14, 32), S(13, 24), S(6, 13), S(21, 5), S(23, -2), S(12, 4), S(17, 17), S(-23, 17),
        S(-27, 13), S(-2, 9), S(-5, -3), S(12, -7), S(17, -7), S(6, -8), S(10, 3), S(-25, -1),
        S(-26, 4), S(-4, 7), S(-4, -6), S(-10, 1), S(3, 0), S(3, -5), S(33, -1), S(-12, -8),
        S(-35, 13), S(-1, 8), S(-20, 8), S(-23, 10), S(-15, 13), S(24, 0), S(38, 2), S(-22, -7),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    },
    { // knight
        S(-167, -58), S(-89, -38), S(-34, -13), S(-49, -28), S(61, -31), S(-97, -27), S(-15, -63), S(-107, -99),
        S(-73, -25), S(-41, -8), S(72, -25), S(36, -2), S(23, -9), S(62, -25), S(7, -24), S(-17, -52),
        S(-47, -24), S(60, -20), S(37, 10), S(65, 9), S(84, -1), S(129, -9), S(73, -19), S(44, -41),
        S(-9, -17), S(17, 3), S(19, 22), S(53, 22), S(37, 22), S(69, 11), S(18, 8), S(22, -18),
        S(-13, -18), S(4, -6), S(16, 16), S(13, 25), S(28, 16), S(19, 17), S(21, 4), S(-8, -18),
        S(-23, -23), S(-9, -3), S(12, -1), S(10, 15), S(19, 10), S(17, -3), S(25, -20), S(-16, -22),
        S(-29, -42), S(-53, -20), S(-12, -10), S(-3, -5), S(-1, -2), S(18, -20), S(-14, -23), S(-19, -44),
        S(-105, -29), S(-21, -51), S(-58, -23), S(-33, -15), S(-17, -22), S(-28, -18), S(-19, -50), S(-23, -64)
    },
    { // bishop
        S(-29, -14), S(4, -21), S(-82, -11), S(-37, -8), S(-25, -7), S(-42, -9), S(7, -17), S(-8, -24),
        S(-26, -8), S(16, -4), S(-18, 7), S(-13, -12), S(30, -3), S(59, -13), S(18, -4), S(-47, -14),
        S(-16, 2), S(37, -8), S(43, 0), S(40, -1), S(35, -2), S(50, 6), S(37, 0), S(-2, 4),
        S(-4, -3), S(5, 9), S(19, 12), S(50, 9), S(37, 14), S(37, 10), S(7, 3), S(-2, 2),
        S(-6, -6), S(13, 3), S(13, 13), S(26, 19), S(34, 7), S(12, 10), S(10, -3), S(4, -9),
        S(0, -12), S(15, -3), S(15, 8), S(15, 10), S(14, 13), S(27, 3), S(18, -7), S(10, -15),
        S(4, -14), S(15, -18), S(16, -7), S(0, -1), S(7, 4), S(21, -9), S(33, -15), S(1, -27),
        S(-33, -23), S(-3, -9), S(-14, -23), S(-21, -5), S(-13, -9), S(-12, -16), S(-39, -5), S(-21, -17)
    },
    { // rook
        S(32, 13), S(42, 10), S(32, 18), S(51, 15), S(63, 12), S(9, 12), S(31, 8), S(43, 5),
        S(27, 11), S(32, 13), S(58, 13), S(62, 11), S(80, -3), S(67, 3), S(26, 8), S(44, 3),
        S(-5, 7), S(19, 7), S(26, 7), S(36, 5), S(17, 4), S(45, -3), S(61, -5), S(16, -3),
        S(-24, 4), S(-11, 3), S(7, 13), S(26, 1), S(24, 2), S(35, 1), S(-8, -1), S(-20, 2),
        S(-36, 3), S(-26, 5), S(-12, 8), S(-1, 4), S(9, -5), S(-7, -6), S(6, -8), S(-23, -11),
        S(-45, -4), S(-25, 0), S(-16, -5), S(-17, -1), S(3, -7), S(0, -12), S(-5, -8), S(-33, -16),
        S(-44, -6), S(-16, -6), S(-20, 0), S(-9, 2), S(-1, -9), S(11, -9), S(-6, -11), S(-71, -3),
        S(-19, -9), S(-13, 2), S(1, 3), S(17, -1), S(16, -5), S(7, -13), S(-37, 4), S(-26, -20)
    },
    { // queen
        S(-28, -9), S(0, 22), S(29, 22), S(12, 27), S(59, 27), S(44, 19), S(43, 10), S(45, 20),
        S(-24, -17), S(-39, 20), S(-5, 32), S(1, 41), S(-16, 58), S(57, 25), S(28, 30), S(54, 0),
        S(-13, -20), S(-17, 6), S(7, 9), S(8, 49), S(29, 47), S(56, 35), S(47, 19), S(57, 9),
        S(-27, 3), S(-27, 22), S(-16, 24), S(-16, 45), S(-1, 57), S(17, 40), S(-2, 57), S(1, 36),
        S(-9, -18), S(-26, 28), S(-9, 19), S(-10, 47), S(-2, 31), S(-4, 34), S(3, 39), S(-3, 23),
        S(-14, -16), S(2, -27), S(-11, 15), S(-2, 6), S(-5, 9), S(2, 17), S(14, 10), S(5, 5),
        S(-35, -22), S(-8, -23), S(11, -30), S(2, -16), S(8, -16), S(15, -23), S(-3, -36), S(1, -32),
        S(-1, -33), S(-18, -28), S(-9, -22), S(10, -43), S(-15, -5), S(-25, -32), S(-31, -20), S(-50, -41)
    },
    { // king
        S(-65, -74), S(23, -35), S(16, -18), S(-15, -18), S(-56, -11), S(-34, 15), S(2, 4), S(13, -17),
        S(29, -12), S(-1, 17), S(-20, 14), S(-7, 17), S(-8, 17), S(-4, 38), S(-38, 23), S(-29, 11),
        S(-9, 10), S(24, 17), S(2, 23), S(-16, 15), S(-20, 20), S(6, 45), S(22, 44), S(-22, 13),
        S(-17, -8), S(-20, 22), S(-12, 24), S(-27, 27), S(-30, 26), S(-25, 33), S(-14, 26), S(-36, 3),
        S(-49, -18), S(-1, -4), S(-27, 21), S(-39, 24), S(-46, 27), S(-44, 23), S(-33, 9), S(-51, -11),
        S(-14, -19), S(-14, -3), S(-22, 11), S(-46, 21), S(-44, 23), S(-30, 16), S(-15, 7), S(-27, -9),
        S(1, -27), S(7, -11), S(-8, 4), S(-64, 13), S(-43, 14), S(-16, 4), S(9, -5), S(8, -17),
        S(-15, -53), S(36, -34), S(12, -21), S(-54, -11), S(8, -28), S(-28, -14), S(24, -24), S(14, -43)
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

// Pre-interpolated tables
int mg_table[12][64]; // [piece][square] -> midgame score
int eg_table[12][64]; // [piece][square] -> endgame score

void init_tables() {
        /*// White pieces (P, N, B, R, Q, K)
        for (int piece = P; piece <= K; piece++) {
                for (int square = 0; square < 64; square++) {
                        mg_table[piece][square] = material_score[opening][piece]
                                                + positional_score[opening][piece][square];
                        eg_table[piece][square] = material_score[endgame][piece]
                                                + positional_score[endgame][piece][square];
                }
        }



        // Black pieces (p, n, b, r, q, k)
        for (int piece = p; piece <= k; piece++) {
                int piece_type = piece - p; // 0-5 (Pawn, Knight,... King)
                for (int square = 0; square < 64; square++) {
                        int mirrored_sq = mirrorScore[square];
                        mg_table[piece][square] = material_score[opening][piece]
                                                - positional_score[opening][piece_type][mirrored_sq];
                        eg_table[piece][square] = material_score[endgame][piece]
                                                - positional_score[endgame][piece_type][mirrored_sq];
                }
        }*/
}

void get_threats(int side, board* pos) {

    uint64_t bb;

    uint64_t knightBB;
    uint64_t bishopBB;
    uint64_t rookBB;
    uint64_t queenBB;
    uint64_t pawnBB;

    // init piece bitboards
    knightBB = pos->bitboards[side == white ? N : n];
    bishopBB = pos->bitboards[side == white ? B : b];
    rookBB = pos->bitboards[side == white ? R : r];
    queenBB = pos->bitboards[side == white ? Q : q];     
    pawnBB = pos->bitboards[side == white ? P : p];    

    // Calculate Knight attacks
    while (knightBB) {
        int knightSquare = getLS1BIndex(knightBB);
        pos->pieceThreats.knightThreats |= getKnightAttacks(knightSquare);
        popBit(knightBB, knightSquare);
    }

    // Calculate Bishop attacks
    while (bishopBB) {
        int bishopSquare = getLS1BIndex(bishopBB);
        pos->pieceThreats.bishopThreats |= getBishopAttacks(bishopSquare, pos->occupancies[both]);
        popBit(bishopBB, bishopSquare);
    }

    // Calculate Rook attacks
    while (rookBB) {    
        int rookSquare = getLS1BIndex(rookBB);
        pos->pieceThreats.rookThreats |= getRookAttacks(rookSquare, pos->occupancies[both]);
        popBit(rookBB, rookSquare);
    }

    // Calculate Pawn attacks
    while (pawnBB) { 
        int pawnSquare = getLS1BIndex(pawnBB);
        pos->pieceThreats.pawnThreats |= getPawnAttacks(side, pawnSquare);
        popBit(pawnBB, pawnSquare);
    }
    
    // Calculate Queen attacks
    while (queenBB) {
        int queenSquare = getLS1BIndex(queenBB);
        pos->pieceThreats.queenThreats |= getQueenAttacks(queenSquare, pos->occupancies[both]);
        popBit(queenBB, queenSquare);
    }
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
        white_piece_scores += countBits(position->bitboards[piece]) * opening_material_score[piece];


    // loop over white pieces
    for (int piece = n; piece <= q; piece++)
        black_piece_scores += countBits(position->bitboards[piece]) * -opening_material_score[piece];



    // return game phase score
    return white_piece_scores + black_piece_scores;
}


int evaluate(board* position) {
        const int game_phase_score = get_game_phase_score(position);

        int packed_score = 0;        

        position->pieceThreats.pawnThreats = 0;
        position->pieceThreats.knightThreats = 0;
        position->pieceThreats.bishopThreats = 0;
        position->pieceThreats.rookThreats = 0;
        position->pieceThreats.queenThreats = 0;

        get_threats(position->side, position);

        const int whiteKingSquare = getLS1BIndex(position->bitboards[K]);
        const int blackKingSquare = getLS1BIndex(position->bitboards[k]);        

        for (int piece = P; piece <= k; piece++) {
                U64 bitboard = position->bitboards[piece];

                while (bitboard) {
                        const int square = getLS1BIndex(bitboard);                       
        
                        popBit(bitboard, square);
                }
        }

        int score_midgame = MgScore(packed_score);
        int score_endgame = EgScore(packed_score);
              
        int final_score = (score_midgame * game_phase_score + score_endgame * (opening_phase_score - game_phase_score)) 
                        / opening_phase_score;


        return (position->side == white) ? final_score : -final_score;
}


void clearStaticEvaluationHistory(board* position) {
    for (int i = 0;i < 64;i++) {
        position->staticEval[i] = noEval;
    }
}
