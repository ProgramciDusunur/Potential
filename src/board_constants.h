//
// Created by erena on 29.05.2024.
//

#ifndef BOARD_CONSTANTS_H
#define BOARD_CONSTANTS_H


#pragma once


#ifndef U64
#define U64 unsigned long long
#endif


/*  these are the score bounds for the range of the mating scores
                                        Score layot
    [-infinity, mateValue ... mateScore ... score ... mateScore ... mateValue, infinity]
 */
#define infinity  50000
#define mateValue 49000
#define mateScore 48000


enum chessBoard {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};


// encode pieces
enum {
    P, N, B, R, Q, K, p, n, b, r, q, k
};
// white and black castling
enum {
    wk = 1, wq = 2, bk = 4, bq = 8
};
enum {
    white, black, both
};
enum {
    rook, bishop
};
enum {
    allMoves, onlyCaptures
};

static char *squareToCoordinates[] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "no"
};

// ASCII pieces
static char asciiPieces[12] = "PNBRQKpnbrqk";
// promoted pieces
static char promotedPieces[] = {
        [Q] = 'q',
        [R] = 'r',
        [B] = 'b',
        [N] = 'n',
        [q] = 'q',
        [r] = 'r',
        [b] = 'b',
        [n] = 'n'
};
// unicode ieces
static char *unicodePieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔",
                                  "♟", "♞", "♝", "♜", "♛", "♚"};
// convert ASCII character pieces to encoded constants
static int charPieces[] = {
        ['P'] = P,
        ['N'] = N,
        ['B'] = B,
        ['R'] = R,
        ['Q'] = Q,
        ['K'] = K,
        ['p'] = p,
        ['n'] = n,
        ['b'] = b,
        ['r'] = r,
        ['q'] = q,
        ['k'] = k
};


// MVV LVA [attacker][victim]
static int mvvLva[12][12] = {
        105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,

        105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600
};




// mask H file
static const U64 notHFile = 9187201950435737471ULL;
// mask A file
static const U64 notAFile = 18374403900871474942ULL;
// mask H rank
static const U64 not8Rank = 18446744073709551360ULL;
// mask A rank
static const U64 not1Rank = 72057594037927935ULL;
// mask 8 rank and AB files
static const U64 not8RankAndABFile = 18229723555195321344ULL;
// mask 1 rank and GH files
static const U64 not1RankAndGHFile = 17802464409370431ULL;
// mask 1 and 2 ranks and H file
static const U64 not1And2RankHFile = 140185576636287ULL;
// mask 8 and 7 ranks and A file
static const U64 not8And7RankAFile = 18374403900871409664ULL;
// mask 8 and 7 ranks and H file
static const U64 not8And7RankHFile = 9187201950435704832ULL;
// mask 1 and 2 ranks and A file
static const U64 not1And2RanksAFile = 280371153272574ULL;
//mask 1 rank and AB files
static const U64 not1RankAndABFile = 71209857637481724ULL;
// mask 8 rank and GH files
static const U64 not8RankAndGHFile = 4557430888798830336ULL;
// not A file constant
static const U64 not_a_file = 18374403900871474942ULL;
// not H file constant
static const U64 not_h_file = 9187201950435737471ULL;
// not H rank and A file
static const U64 notAFileAndHRank = 35887507618889599ULL;
// not 8 Rank and A file
static const U64 not8RankAndAFile = 18374403900871474688ULL;
// not 8 Rank and H file
static const U64 not8RankAndHFile = 9187201950435737344ULL;
// not 1 Rank and A file
static const U64 not1RankAndAFile = 71775015237779198ULL;

static const  int get_rank[64] = {
        7, 7, 7, 7, 7, 7, 7, 7,
        6, 6, 6, 6, 6, 6, 6, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0
};


/*   Zobrist Hashing   */

// random piece keys [piece][square]
extern U64 pieceKeys[12][64];
// random enpassant keys [square]
extern U64 enpassantKeys[64];
// random castling keys
extern U64 castleKeys[16];


#endif // BOARD_CONSTANTS_H