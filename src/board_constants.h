//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_BOARD_CONSTANTS_H
#define POTENTIAL_BOARD_CONSTANTS_H

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
    P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE
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

extern char *squareToCoordinates[];
extern char asciiPieces[12];
extern char promotedPieces[];
extern char *unicodePieces[12];
extern int charPieces[];
extern int minorPieces[6];
extern int majorPieces[4];
extern int mvvLva[12][12];
extern const U64 notHFile;
extern const U64 notAFile;
extern const U64 not8Rank;
extern const U64 not1Rank;
extern const U64 not8RankAndABFile;
extern const U64 not1RankAndGHFile;
extern const U64 not1And2RankHFile;
extern const U64 not8And7RankAFile;
extern const U64 not8And7RankHFile;
extern const U64 not1And2RanksAFile;
extern const U64 not1RankAndABFile;
extern const U64 not8RankAndGHFile;
extern const U64 not_a_file;
extern const U64 not_h_file;
extern const U64 notAFileAndHRank;
extern const U64 not8RankAndAFile;
extern const U64 not8RankAndHFile;
extern const U64 not1RankAndAFile;
extern int get_rank[64];



/*   Zobrist Hashing   */

// random piece keys [piece][square]
extern U64 pieceKeys[12][64];
// random enpassant keys [square]
extern U64 enpassantKeys[64];
// random castling keys
extern U64 castleKeys[16];










#endif //POTENTIAL_BOARD_CONSTANTS_H
