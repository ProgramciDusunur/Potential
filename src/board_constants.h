//
// Created by erena on 29.05.2024.
//
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

extern char *squareToCoordinates[];

// ASCII pieces
extern char asciiPieces[12];
// promoted pieces
extern char promotedPieces[];
// unicode ieces
extern char *unicodePieces[12];
// convert ASCII character pieces to encoded constants
extern int charPieces[];



// most valuable victim & less valuable attacker

/*
    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]
extern int mvvLva[12][12];


// mask H file
extern const U64 notHFile;
// mask A file
extern const U64 notAFile;
// mask H rank
extern const U64 not8Rank;
// mask A rank
extern const U64 not1Rank;
// mask 8 rank and AB files
extern const U64 not8RankAndABFile;
// mask 1 rank and GH files
extern const U64 not1RankAndGHFile;
// mask 1 and 2 ranks and H file
extern const U64 not1And2RankHFile;
// mask 8 and 7 ranks and A file
extern const U64 not8And7RankAFile;
// mask 8 and 7 ranks and H file
extern const U64 not8And7RankHFile;
// mask 1 and 2 ranks and A file
extern const U64 not1And2RanksAFile;
//mask 1 rank and AB files
extern const U64 not1RankAndABFile;
// mask 8 rank and GH files
extern const U64 not8RankAndGHFile;
// not A file constant
extern const U64 not_a_file;
// not H file constant
extern const U64 not_h_file;
// not H rank and A file
extern const U64 notAFileAndHRank;
// not 8 Rank and A file
extern const U64 not8RankAndAFile;
// not 8 Rank and H file
extern const U64 not8RankAndHFile;
// not 1 Rank and A file
extern const U64 not1RankAndAFile;

extern const int get_rank[64];


/*   Zobrist Hashing   */

// random piece keys [piece][square]
extern U64 pieceKeys[12][64];
// random enpassant keys [square]
extern U64 enpassantKeys[64];
// random castling keys
extern U64 castleKeys[16];

