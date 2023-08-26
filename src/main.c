#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN64

#include <windows.h>

#else
#include <sys/time.h>
#endif


#define U64 unsigned long long

#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define emptyBoard "8/8/8/8/8/8/8/8 w - - 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40"

#define popBit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define setBit(bitboard, square) bitboard |= (1ULL << square)
#define getBit(bitboard, square) bitboard & (1ULL << square)

// encode move
#define encodeMove(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

// extract source square
#define getMoveSource(move) (move & 0x3f)
// extract target square
#define getMoveTarget(move) ((move & 0xfc0) >> 6)
// extract piece
#define getMovePiece(move) ((move & 0xf000) >> 12)
// extract promoted piece
#define getMovePromoted(move) ((move & 0xf0000) >> 16)
// extract double pawn push
#define getMoveCapture(move) (move & 0x100000)
// extract double pawn push flag
#define getMoveDouble(move) (move & 0x200000)
// extract enpassant flag
#define getMoveEnpassant(move) (move & 0x400000)
// extract castling flag
#define getMoveCastling(move) (move & 0x800000)

// preserve board state
#define copyBoard()                                                      \
    U64 bitboardsCopy[12], occupanciesCopy[3];                           \
    int sideCopy, enpassantCopy, castleCopy;                             \
    bitboardsCopy[0] = bitboards[0];                                     \
    bitboardsCopy[1] = bitboards[1];                                     \
    bitboardsCopy[2] = bitboards[2];                                     \
    bitboardsCopy[3] = bitboards[3];                                     \
    bitboardsCopy[4] = bitboards[4];                                     \
    bitboardsCopy[5] = bitboards[5];                                     \
    bitboardsCopy[6] = bitboards[6];                                     \
    bitboardsCopy[7] = bitboards[7];                                     \
    bitboardsCopy[8] = bitboards[8];                                     \
    bitboardsCopy[9] = bitboards[9];                                     \
    bitboardsCopy[10] = bitboards[10];                                   \
    bitboardsCopy[11] = bitboards[11];                                   \
    bitboardsCopy[12] = bitboards[12];                                   \
    occupanciesCopy[0] = occupancies[0];                                 \
    occupanciesCopy[1] = occupancies[1];                                 \
    occupanciesCopy[2] = occupancies[2];                                 \
    occupanciesCopy[3] = occupancies[3];                                 \
    U64 hashKeyCopy = hashKey;                                           \
    sideCopy = side, enpassantCopy = enpassant, castleCopy = castle;     \
// restore board state
#define takeBack()                                                       \
    bitboards[0] = bitboardsCopy[0];                                     \
    bitboards[1] = bitboardsCopy[1];                                     \
    bitboards[2] = bitboardsCopy[2];                                     \
    bitboards[3] = bitboardsCopy[3];                                     \
    bitboards[4] = bitboardsCopy[4];                                     \
    bitboards[5] = bitboardsCopy[5];                                     \
    bitboards[6] = bitboardsCopy[6];                                     \
    bitboards[7] = bitboardsCopy[7];                                     \
    bitboards[8] = bitboardsCopy[8];                                     \
    bitboards[9] = bitboardsCopy[9];                                     \
    bitboards[10] = bitboardsCopy[10];                                   \
    bitboards[11] = bitboardsCopy[11];                                   \
    bitboards[12] = bitboardsCopy[12];                                   \
    occupancies[0] = occupanciesCopy[0];                                 \
    occupancies[1] = occupanciesCopy[1];                                 \
    occupancies[2] = occupanciesCopy[2];                                 \
    occupancies[3] = occupanciesCopy[3];                                 \
    hashKey = hashKeyCopy;                                              \
    side = sideCopy, enpassant = enpassantCopy, castle = castleCopy;     \

// move list structure
typedef struct {
    // moves
    int moves[256];

    // move count
    int count;
} moves;


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

char *squareToCoordinates[] = {
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
char asciiPieces[12] = "PNBRQKpnbrqk";
// promoted pieces
char promotedPieces[] = {
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
char *unicodePieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔",
                           "♟", "♞", "♝", "♜", "♛", "♚"};
// convert ASCII character pieces to encoded constants
int charPieces[] = {
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
const int castlingRights[64] = {
        7, 15, 15, 15, 3, 15, 15, 11,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        13, 15, 15, 15, 12, 15, 15, 14
};


// mirror positional score tables for opposite side
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

/*  these are the score bounds for the range of the mating scores
                                        Score layot
    [-infinity, mateValue ... mateScore ... score ... mateScore ... mateValue, infinity]
 */
#define infinity  50000
#define mateValue 49000
#define mateScore 48000



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

// killer moves [id][ply]
int killerMoves[2][64];
// history moves [piece][square]
int historyMoves[12][64];

// PV length
int pvLength[64];
// PV table
int pvTable[64][64];
// follow PVS node
int followPV;
// the score of PVS
int scorePV;



// Pawn attack masks pawnAttacks[side][square]
U64 pawnAtacks[2][64];
// Knight attack masks knightAttacks[square]
U64 knightAttacks[64];
// King attack masks kingAttacks[square]
U64 kingAttacks[64];
// Bishop attack table [square][occupancies]
U64 bishopAttacks[64][512];
// Rook attack table [square][occupancies]
U64 rookAttacks[64][4096];

// Rook attack masks rookMask[square]
U64 rookMask[64];
// BishopMask[square]
U64 bishopMask[64];

// rookMagics[square]
U64 rookMagic[64];
// bishopMagics[square]
U64 bishopMagic[64];

// piece bitboards
U64 bitboards[12];
// occupancies bitboards
U64 occupancies[3];


// performance test node count, variant count, negamax ply count
U64 nodes, variant;
// side to move
int side = -1;
// enpassant square
int enpassant = no_sq;
// castling rights
int castle;

const int bishopRelevantBits[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
};
const int rookRelevantBits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12,
};


// mask H file
U64 notHFile = 9187201950435737471ULL;
// mask A file
U64 notAFile = 18374403900871474942ULL;
// mask H rank
U64 not8Rank = 18446744073709551360ULL;
// mask A rank
U64 not1Rank = 72057594037927935ULL;
// mask 8 rank and AB files
U64 not8RankAndABFile = 18229723555195321344ULL;
// mask 1 rank and GH files
U64 not1RankAndGHFile = 17802464409370431ULL;
// mask 1 and 2 ranks and H file
U64 not1And2RankHFile = 140185576636287ULL;
// mask 8 and 7 ranks and A file
U64 not8And7RankAFile = 18374403900871409664ULL;
// mask 8 and 7 ranks and H file
U64 not8And7RankHFile = 9187201950435704832ULL;
// mask 1 and 2 ranks and A file
U64 not1And2RanksAFile = 280371153272574ULL;
//mask 1 rank and AB files
U64 not1RankAndABFile = 71209857637481724ULL;
// mask 8 rank and GH files
U64 not8RankAndGHFile = 4557430888798830336ULL;
// not A file constant
const U64 not_a_file = 18374403900871474942ULL;
// not H file constant
const U64 not_h_file = 9187201950435737471ULL;
// not H rank and A file
const U64 notAFileAndHRank = 35887507618889599ULL;
// not 8 Rank and A file
const U64 not8RankAndAFile = 18374403900871474688ULL;
// not 8 Rank and H file
const U64 not8RankAndHFile = 9187201950435737344ULL;
// not 1 Rank and A file
const U64 not1RankAndAFile = 71775015237779198ULL;
// pseudo random number state
unsigned int state = 1804289383;

/**********************************\
 ==================================

       Time controls variables

 ==================================
\**********************************/

// exit from engine flag
int quit = 0;

// UCI "movestogo" command moves counter
int movestogo = 30;

// UCI "movetime" command time counter
int movetime = -1;

// UCI "time" command holder (ms)
int time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;

/*   Zobrist Hashing   */

// random piece keys [piece][square]
U64 pieceKeys[12][64];
// random enpassant keys [square]
U64 enpassantKeys[64];
// random castling keys
U64 castleKeys[16];
// random side key
U64 sideKey;
// "almost" unique position identifier akka hash key or position key
U64 hashKey;

// positions repetition table
U64 repetitionTable[1000]; // 1000 is a number of plies (500 moves) in the entire game
// repetition index
int repetitionIndex;

// half move counter
int ply;



/**********************************\
 ==================================

         Transposition Table

 ==================================
\**********************************/

// hash table size
#define hashSize 0x400000

// no hash entry found constant
#define noHashEntry 100000

#define hashFlagExact 0
#define hashFlagAlpha 1
#define hashFlagBeta  2

// transposition table data structure
typedef struct {
    U64 hashKey;     // "almost" unique chess position identifier
    int depth;       // current search depth
    int flag;       // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    int score;       // score (alpha/beta/PV)
    int bestMove;
} tt;                 // transposition table (TT aka hash table)

// define TT insance
tt hashTable[hashSize];


/**********************************\
 ==================================

             Evaluation

 ==================================
\**********************************/

// material scrore

/*
    ♙ =   100   = ♙
    ♘ =   300   = ♙ * 3
    ♗ =   350   = ♙ * 3 + ♙ * 0.5
    ♖ =   500   = ♙ * 5
    ♕ =   1000  = ♙ * 10
    ♔ =   10000 = ♙ * 100

*/

// material score [game phase][piece]
const int material_score[2][12] =
        {
                // opening material score
                82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000,

                // endgame material score
                94, 281, 297, 512, 936, 12000, -94, -281, -297, -512, -936, -12000
        };

// game phase scores
const int opening_phase_score = 6192;
const int endgame_phase_score = 518;

// game phases
enum {
    opening, endgame, middlegame
};

// piece types
enum {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

// positional piece scores [game phase][piece][square]
const int positional_score[2][6][64] =

// opening positional piece scores //
        {
                //pawn
                0, 0, 0, 0, 0, 0, 0, 0,
                98, 134, 61, 95, 68, 126, 34, -11,
                -6, 7, 26, 31, 65, 56, 25, -20,
                -14, 13, 6, 21, 23, 12, 17, -23,
                -27, -2, -5, 12, 17, 6, 10, -25,
                -26, -4, -4, -10, 3, 3, 33, -12,
                -35, -1, -20, -23, -15, 24, 38, -22,
                0, 0, 0, 0, 0, 0, 0, 0,

                // knight
                -167, -89, -34, -49, 61, -97, -15, -107,
                -73, -41, 72, 36, 23, 62, 7, -17,
                -47, 60, 37, 65, 84, 129, 73, 44,
                -9, 17, 19, 53, 37, 69, 18, 22,
                -13, 4, 16, 13, 28, 19, 21, -8,
                -23, -9, 12, 10, 19, 17, 25, -16,
                -29, -53, -12, -3, -1, 18, -14, -19,
                -105, -21, -58, -33, -17, -28, -19, -23,

                // bishop
                -29, 4, -82, -37, -25, -42, 7, -8,
                -26, 16, -18, -13, 30, 59, 18, -47,
                -16, 37, 43, 40, 35, 50, 37, -2,
                -4, 5, 19, 50, 37, 37, 7, -2,
                -6, 13, 13, 26, 34, 12, 10, 4,
                0, 15, 15, 15, 14, 27, 18, 10,
                4, 15, 16, 0, 7, 21, 33, 1,
                -33, -3, -14, -21, -13, -12, -39, -21,

                // rook
                32, 42, 32, 51, 63, 9, 31, 43,
                27, 32, 58, 62, 80, 67, 26, 44,
                -5, 19, 26, 36, 17, 45, 61, 16,
                -24, -11, 7, 26, 24, 35, -8, -20,
                -36, -26, -12, -1, 9, -7, 6, -23,
                -45, -25, -16, -17, 3, 0, -5, -33,
                -44, -16, -20, -9, -1, 11, -6, -71,
                -19, -13, 1, 17, 16, 7, -37, -26,

                // queen
                -28, 0, 29, 12, 59, 44, 43, 45,
                -24, -39, -5, 1, -16, 57, 28, 54,
                -13, -17, 7, 8, 29, 56, 47, 57,
                -27, -27, -16, -16, -1, 17, -2, 1,
                -9, -26, -9, -10, -2, -4, 3, -3,
                -14, 2, -11, -2, -5, 2, 14, 5,
                -35, -8, 11, 2, 8, 15, -3, 1,
                -1, -18, -9, 10, -15, -25, -31, -50,

                // king
                -65, 23, 16, -15, -56, -34, 2, 13,
                29, -1, -20, -7, -8, -4, -38, -29,
                -9, 24, 2, -16, -20, 6, 22, -22,
                -17, -20, -12, -27, -30, -25, -14, -36,
                -49, -1, -27, -39, -46, -44, -33, -51,
                -14, -14, -22, -46, -44, -30, -15, -27,
                1, 7, -8, -64, -43, -16, 9, 8,
                -15, 36, 12, -54, 8, -28, 24, 14,


                // Endgame positional piece scores //

                //pawn
                0, 0, 0, 0, 0, 0, 0, 0,
                178, 173, 158, 134, 147, 132, 165, 187,
                94, 100, 85, 67, 56, 53, 82, 84,
                32, 24, 13, 5, -2, 4, 17, 17,
                13, 9, -3, -7, -7, -8, 3, -1,
                4, 7, -6, 1, 0, -5, -1, -8,
                13, 8, 8, 10, 13, 0, 2, -7,
                0, 0, 0, 0, 0, 0, 0, 0,

                // knight
                -58, -38, -13, -28, -31, -27, -63, -99,
                -25, -8, -25, -2, -9, -25, -24, -52,
                -24, -20, 10, 9, -1, -9, -19, -41,
                -17, 3, 22, 22, 22, 11, 8, -18,
                -18, -6, 16, 25, 16, 17, 4, -18,
                -23, -3, -1, 15, 10, -3, -20, -22,
                -42, -20, -10, -5, -2, -20, -23, -44,
                -29, -51, -23, -15, -22, -18, -50, -64,

                // bishop
                -14, -21, -11, -8, -7, -9, -17, -24,
                -8, -4, 7, -12, -3, -13, -4, -14,
                2, -8, 0, -1, -2, 6, 0, 4,
                -3, 9, 12, 9, 14, 10, 3, 2,
                -6, 3, 13, 19, 7, 10, -3, -9,
                -12, -3, 8, 10, 13, 3, -7, -15,
                -14, -18, -7, -1, 4, -9, -15, -27,
                -23, -9, -23, -5, -9, -16, -5, -17,

                // rook
                13, 10, 18, 15, 12, 12, 8, 5,
                11, 13, 13, 11, -3, 3, 8, 3,
                7, 7, 7, 5, 4, -3, -5, -3,
                4, 3, 13, 1, 2, 1, -1, 2,
                3, 5, 8, 4, -5, -6, -8, -11,
                -4, 0, -5, -1, -7, -12, -8, -16,
                -6, -6, 0, 2, -9, -9, -11, -3,
                -9, 2, 3, -1, -5, -13, 4, -20,

                // queen
                -9, 22, 22, 27, 27, 19, 10, 20,
                -17, 20, 32, 41, 58, 25, 30, 0,
                -20, 6, 9, 49, 47, 35, 19, 9,
                3, 22, 24, 45, 57, 40, 57, 36,
                -18, 28, 19, 47, 31, 34, 39, 23,
                -16, -27, 15, 6, 9, 17, 10, 5,
                -22, -23, -30, -16, -16, -23, -36, -32,
                -33, -28, -22, -43, -5, -32, -20, -41,

                // king
                -74, -35, -18, -18, -11, 15, 4, -17,
                -12, 17, 14, 17, 17, 38, 23, 11,
                10, 17, 23, 15, 20, 45, 44, 13,
                -8, 22, 24, 27, 26, 33, 26, 3,
                -18, -4, 21, 24, 27, 23, 9, -11,
                -19, -3, 11, 21, 23, 16, 7, -9,
                -27, -11, 4, 13, 14, 4, -5, -17,
                -53, -34, -21, -11, -28, -14, -24, -43
        };

U64 maskPawnAttacks(int side, int square);

U64 maskKnightAttacks(int square);

U64 maskKingAttacks(int square);

U64 maskBishopAttacks(int square);

U64 bishopAttack(int square, U64 block);

U64 maskRookAttacks(int square);

U64 rookAttack(int square, U64 block);

U64 setOccupancy(int index, int bitsInMask, U64 attackMask);

U64 getRandom64Numbers();

U64 generateMagicNumber();

U64 findMagicNumber(int square, int relevantBits, int bishop);

static inline U64 getBishopAttacks(int square, U64 occupancy);

static inline U64 getRookAttacks(int square, U64 occupancy);

static inline U64 getQueenAttacks(int square, U64 occupancy);

static inline void perft(int depth);

static inline void addMove(moves *moveList, int move);

static inline void enablePVScoring(moves *moveList);

static inline int makeMove(int move, int moveFlag);

static inline int isSquareAttacked(int square, int side);

unsigned int getRandom32BitNumber();

int countBits(U64 bitboard);

int getLS1BIndex(U64 bitboard);

int getTimeMiliSecond();

int areSubStringsEqual(char *command, char *uciCommand, int stringSize);

void pBitboard(U64 bitboard);

void printAttackedSquares(int side);

void pBoard();

void printMove(int move);

void printMoveList(moves *moveList);

void initAll();

void initSlidersAttacks(int bishop);

void initLeaperAttacks();

void initMagicNumbers();

void parseFEN(char *fen);

void searchPosition(int depth);

void uciProtocol();

void goCommand(char *command);

void communicate();

void read_input();

static inline void moveGenerator(moves *moveList);

static inline void perftRoot(int depth);

static inline void perftChild(int depth);

static inline int evaluate();

static inline int negamax(int alpha, int beta, int depth);

static inline int quiescence(int alpha, int beta);

static inline int scoreMove(int move);

static inline int sort_moves(moves *moveList, int bestMove);

static inline void enable_pv_scoring(moves *moveList);

void initRandomKeys();

U64 generateHashKey();

static inline int readHashEntry(int alpha, int beta, int *bestMove, int depth);

void writeHashEntry(int score, int bestMove, int depth, int hashFlag);


int main() {
    initAll();
    uciProtocol();
    return 0;
}

// get game phase score
static inline int get_game_phase_score() {
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
        white_piece_scores += countBits(bitboards[piece]) * material_score[opening][piece];

    // loop over white pieces
    for (int piece = n; piece <= q; piece++)
        black_piece_scores += countBits(bitboards[piece]) * -material_score[opening][piece];

    // return game phase score
    return white_piece_scores + black_piece_scores;
}


void writeHashEntry(int score, int bestMove, int depth, int hashFlag) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[hashKey % hashSize];

    // store score independent from the actual path
    // from root node (position) to current node (position)
    if (score < -mateScore) score -= ply;
    if (score > mateScore) score += ply;

    hashEntry->hashKey = hashKey;
    hashEntry->score = score;
    hashEntry->flag = hashFlag;
    hashEntry->depth = depth;
    hashEntry->bestMove = bestMove;
}

// read hash entry data
static inline int readHashEntry(int alpha, int beta, int *bestMove, int depth) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[hashKey % hashSize];

    // make sure we're dealing with the exact position we need
    if (hashEntry->hashKey == hashKey) {

        // make sure that we watch the exact depth our search is now at
        if (hashEntry->depth >= depth) {

            // extract stored score from TT entry
            int score = hashEntry->score;

            // retrieve score independent from the actual path
            // from root node (position) to current node (position)
            if (score < -mateScore) score -= ply;
            if (score > mateScore) score += ply;


            // match the exact (PV node) score
            if (hashEntry->flag == hashFlagExact) {
                // return exact (PV node) score
                return score;
            }
            // match alpha (fail-low node) score
            if ((hashEntry->flag == hashFlagAlpha) && (score <= alpha)) {
                // return alpha (fail-low node) score
                return alpha;
            }
            if ((hashEntry->flag == hashFlagBeta) && (score >= beta)) {
                // return beta (fail-high node) score
                return beta;
            }
        }
        // store best move
        *bestMove = hashEntry->bestMove;

    }
    // if hash entry doesn' exist
    return noHashEntry;
}

void clearHashTable() {
    // loop over TT elements
    for (int index = 0; index < hashSize; index++) {
        //
        hashTable[index].hashKey = 0;
        hashTable[index].depth = 0;
        hashTable[index].flag = 0;
        hashTable[index].score = 0;
        hashTable[index].bestMove = 0;
    }
}

// init random hash keys
void initRandomKeys() {
    // update pseudo random number state
    state = 1804289383;
    // loop over piece codes
    for (int piece = P; piece <= k; piece++) {
        // loop over board squares
        for (int square = 0; square < 64; square++) {
            pieceKeys[piece][square] = getRandom64Numbers();
        }
    }
    // loop over board squares
    for (int square = 0; square < 64; square++) {
        // init random enpassant keys
        enpassantKeys[square] = getRandom64Numbers();
    }
    // loop over castling keys
    for (int index = 0; index < 16; index++) {
        // init castling keys
        castleKeys[index] = getRandom64Numbers();
    }
    // loop over castling keys
    sideKey = getRandom64Numbers();
}

// generate "almost" unique position ID aka hash key from scratch
U64 generateHashKey() {
    // final hash key
    U64 finalKey = 0ULL;

    // temp piece bitboard copy
    U64 bitboard;


    // loop over piece bitboards
    for (int piece = P; piece <= k; piece++) {
        // init piece bitboard copy
        bitboard = bitboards[piece];

        // loop over the pieces within a bitboard
        while (bitboard) {
            // init square occupied by the piece
            int square = getLS1BIndex(bitboard);

            // hash piece
            finalKey ^= pieceKeys[piece][square];

            // pop LS1B
            popBit(bitboard, square);
        }
    }

    if (enpassant != no_sq) {
        // hash enpassant
        finalKey ^= enpassantKeys[enpassant];
    }
    // hash castling rights
    finalKey ^= castleKeys[castle];

    // hash the side only if black is to move
    if (side == black) { finalKey ^= sideKey; }

    // return generated hash key
    return finalKey;
}


// parse user/GUI move string input (e.g. "e7e8q")
int parse_move(char *move_string) {
    // create move list instance
    moves moveList[1];

    // generate moves
    moveGenerator(moveList);

    // parse source square
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;

    // parse target square
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    // loop over the moves within a move list
    for (int move_count = 0; move_count < moveList->count; move_count++) {
        // init move
        int move = moveList->moves[move_count];

        // make sure source & target squares are available within the generated move
        if (source_square == getMoveSource(move) && target_square == getMoveTarget(move)) {
            // init promoted piece
            int promoted_piece = getMovePromoted(move);

            // promoted piece is available
            if (promoted_piece) {
                // promoted to queen
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    // return legal move
                    return move;

                    // promoted to rook
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    // return legal move
                    return move;

                    // promoted to bishop
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    // return legal move
                    return move;

                    // promoted to knight
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    // return legal move
                    return move;

                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }

            // return legal move
            return move;
        }
    }

    // return illegal move
    return 0;
}

// parse UCI "position" command
void parse_position(char *command) {
    // shift pointer to the right where next token begins
    command += 9;

    // init pointer to the current character in the command string
    char *current_char = command;

    // parse UCI "startpos" command
    if (strncmp(command, "startpos", 8) == 0)
        // init chess board with start position
        parseFEN(startPosition);

        // parse UCI "fen" command
    else {
        // make sure "fen" command is available within command string
        current_char = strstr(command, "fen");

        // if no "fen" command is available within command string
        if (current_char == NULL)
            // init chess board with start position
            parseFEN(startPosition);

            // found "fen" substring
        else {
            // shift pointer to the right where next token begins
            current_char += 4;

            // init chess board with position from FEN string
            parseFEN(current_char);
        }
    }

    // parse moves after position
    current_char = strstr(command, "moves");

    // moves available
    if (current_char != NULL) {
        // shift pointer to the right where next token begins
        current_char += 6;

        // loop over moves within a move string
        while (*current_char) {
            // parse next move
            int move = parse_move(current_char);

            // if no more moves
            if (move == 0)
                // break out of the loop
                break;

            // increment repetition index
            repetitionIndex++;

            // write hash key into a repetition table
            repetitionTable[repetitionIndex] = hashKey;

            // make move on the chess board
            makeMove(move, allMoves);

            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ') current_char++;

            // go to the next move
            current_char++;
        }
    }

    // print board
    pBoard();
}


void uciProtocol() {
    // reset STDIN & STDOUT buffers
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    int shouldWork = 1;
    char command[2000];

    while (shouldWork) {
        memset(command, 0, sizeof(command));
        fflush(stdout);
        fgets(command, sizeof(command), stdin);
        size_t length = strlen(command);

        if (length > 0 && command[length - 1] == '\n') {
            command[length - 1] = '\0';
        }
        // position startpos
        if (strlen(command) <= 17 && areSubStringsEqual(command, "position startpos", strlen("position startpos"))) {
            parseFEN(startPosition);
            pBoard();
        } else if (areSubStringsEqual(command, "position startpos moves", strlen("position startpos moves"))) {
            parse_position(command);
        } else if (areSubStringsEqual(command, "go depth", strlen("go depth"))) {
            int depth = -1;
            // parse search depth
            depth = atoi(command + 9);
            searchPosition(depth);
        } else if (strncmp(command, "go", 2) == 0) {
            goCommand(command);
        } else if (!strcmp("position", command)) {
            parse_position(command);
            clearHashTable();
        } else if (!strcmp("ucinewgame", command)) {
            // call parse position function
            parse_position("position startpos");
            // clear hash table
            clearHashTable();
        } else if (!strcmp("quit", command)) {
            shouldWork = 0;
        } else if (!strcmp("uci", command)) {
            printf("id name Potential\n");
            printf("id author ProgramciDusunur\n");
            printf("uciok\n");
        } else if (!strcmp("isready", command)) {
            printf("readyok\n");
        }
    }
}

// reset time control variables
void resetTimeControl() {
    // reset timing
    quit = 0;
    movestogo = 30;
    movetime = -1;
    time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}

void goCommand(char *command) {

    // reset time control
    resetTimeControl();

    // init parameters
    int depth = -1;

    // init argument
    char *argument = NULL;

    // infinite search
    if ((argument = strstr(command, "infinite"))) {}

    // match UCI "binc" command
    if ((argument = strstr(command, "binc")) && side == black)
        // parse black time increment
        inc = atoi(argument + 5);

    // match UCI "winc" command
    if ((argument = strstr(command, "winc")) && side == white)
        // parse white time increment
        inc = atoi(argument + 5);

    // match UCI "wtime" command
    if ((argument = strstr(command, "wtime")) && side == white)
        // parse white time limit
        time = atoi(argument + 6);

    // match UCI "btime" command
    if ((argument = strstr(command, "btime")) && side == black)
        // parse black time limit
        time = atoi(argument + 6);

    // match UCI "movestogo" command
    if ((argument = strstr(command, "movestogo")))
        // parse number of moves to go
        movestogo = atoi(argument + 10);

    // match UCI "movetime" command
    if ((argument = strstr(command, "movetime")))
        // parse amount of time allowed to spend to make a move
        movetime = atoi(argument + 9);

    // match UCI "depth" command
    if ((argument = strstr(command, "depth")))
        // parse search depth
        depth = atoi(argument + 6);

    // if move time is not available
    if (movetime != -1) {
        // set time equal to move time
        time = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = getTimeMiliSecond();

    // init search depth
    depth = depth;

    // if time control is available
    if (time != -1) {
        // flag we're playing with time control
        timeset = 1;

        // set up timing
        time /= movestogo;

        // "illegal" (empty) move bug fix
        if (time > 1500) time -= 50;

        // init stoptime
        stoptime = starttime + time + inc;
    }

    // if depth is not available
    if (depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 64;

    // print debug info
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
           time, starttime, stoptime, depth, timeset);

    // search position
    searchPosition(depth);

}


void pBitboard(U64 bitboard) {
    printf("\n");
    for (int y = 0, coordinate = 8; y < 8; y++, coordinate--) {
        printf("%d  ", coordinate);
        for (int x = 0; x < 8; x++) {
            int index = y * 8 + x;
            printf("%d ", getBit(bitboard, index) ? 1 : 0);
        }
        printf(" \n");
    }
    printf("\n   a b c d e f g h \n");
    printf("\n   Bitboard: %llu decimal\n\n", bitboard);
}

void pBoard() {
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if (!file) {
                printf(" %d ", 8 - rank);
            }
            int piece = -1;
            for (int bbPiece = P; bbPiece <= k; bbPiece++) {
                if (getBit(bitboards[bbPiece], square)) {
                    piece = bbPiece;
                }
            }
#ifdef WIN64
            printf(" %c", (piece == -1) ? '.' : asciiPieces[piece]);
#else
            printf(" %s", (piece == -1) ? '.' : unicodePieces[piece]);
#endif
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");

    printf("    Side:     %s\n", !side ? "white" : "black");

    printf("    Enpassant: %s\n", (enpassant != no_sq) ? squareToCoordinates[enpassant] : "  no");

    printf("    Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
           (castle & wq) ? 'Q' : '-',
           (castle & bk) ? 'k' : '-',
           (castle & bq) ? 'q' : '-');

    // print hash key
    printf("    Hash key:  %llx\n\n", hashKey);
}

// print move
void printMove(int move) {
    if (getMovePromoted(move)) {
        printf("%s%s%c", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               promotedPieces[getMovePromoted(move)]);
    } else {
        printf("%s%s", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)]);
    }

}

// print move list
void printMoveList(moves *moveList) {
    /*if (!moveList->count) {
        printf("\n    No move in the move list.\n");
        return;
    }*/



    printf("\n  move   piece   capture   double   enpassant   castling");

    // loop over moves within a move list
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        int move = moveList->moves[moveCount];
#ifdef WIN64
        printf(" \n  %s%s%c   %c       %d         %d        %d           %d", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               getMovePromoted(move) ? promotedPieces[getMovePromoted(move)] : ' ',
               asciiPieces[getMovePiece(move)],
               getMoveCapture(move) ? 1 : 0,
               getMoveDouble(move) ? 1 : 0,
               getMoveEnpassant(move) ? 1 : 0,
               getMoveCastling(move) ? 1 : 0);

#else
        printf(" \n  %s%s%c  %c       %d         %d        %d           %d",            squareToCoordinates[getMoveSource(move)],
                                                                                        squareToCoordinates[getMoveTarget(move)],
                                                                                        getMovePromoted(move) ? promotedPieces[getMovePromoted(move)] : ' ',
                                                                                        unicodePieces[getMovePiece(move)],
                                                                                        getMoveCapture(move) ? 1 : 0,
                                                                                        getMoveDouble(move) ? 1 : 0,
                                                                                        getMoveEnpassant(move) ? 1 : 0,
                                                                                        getMoveCastling(move) ? 1 : 0);
#endif
    }
    printf("\n\n  Total number of moves: %d\n\n", moveList->count);
}

// parse FEN string
void parseFEN(char *fen) {
    // reset board position (bitboards)
    memset(bitboards, 0ULL, sizeof(bitboards));
    // reset board occupancies (bitboards)
    memset(occupancies, 0ULL, sizeof(occupancies));

    // reset game state variables
    side = 0;
    enpassant = no_sq;
    castle = 0;

    // reset repetition index
    repetitionIndex = 0;
    // reset repetition table
    memset(repetitionTable, 0ULL, sizeof(repetitionTable));

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                int piece = charPieces[*fen];
                setBit(bitboards[piece], square);
                fen++;
            }
            if (*fen >= '0' && *fen <= '9') {
                int offset = *fen - '0';
                int piece = -1;
                for (int bbPiece = P; bbPiece <= k; bbPiece++) {
                    if (getBit(bitboards[bbPiece], square)) {
                        piece = bbPiece;
                    }
                }
                if (piece == -1) {
                    file--;
                }
                file += offset;
                fen++;
            }
            if (*fen == '/') {
                fen++;
            }
        }
    }
    fen++;
    (*fen == 'w') ? (side = white) : (side = black);
    fen += 2;
    while (*fen != ' ') {
        switch (*fen) {
            case 'K':
                castle |= wk;
                break;
            case 'Q':
                castle |= wq;
                break;
            case 'k':
                castle |= bk;
                break;
            case 'q':
                castle |= bq;
                break;
            case '-':
                break;
        }
        fen++;
    }
    fen++;
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        enpassant = rank * 8 + file;
    } else {
        enpassant = no_sq;
    }
    for (int piece = P; piece <= K; piece++) {
        occupancies[white] |= bitboards[piece];
    }
    for (int piece = p; piece <= k; piece++) {
        occupancies[black] |= bitboards[piece];
    }
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

    // init hash key
    hashKey = generateHashKey();
}

static inline void perftRoot(int depth) {
    moves moveList[1];
    moveGenerator(moveList);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        copyBoard();
        if (makeMove(moveList->moves[moveCount], allMoves) == 0) {
            // skip to the next move
            continue;
        }
        // call perft driver recursively
        perftChild(depth - 1);
        printf("%s%s%c %llu \n", squareToCoordinates[getMoveSource(moveList->moves[moveCount])],
               squareToCoordinates[getMoveTarget(moveList->moves[moveCount])],
               promotedPieces[getMovePromoted(moveList->moves[moveCount])], variant);
        variant = 0;
        takeBack();
    }
    printf("\n");

}

static inline void perftChild(int depth) {
    if (depth == 0) {
        nodes++;
        variant++;
        return;
    }
    moves moveList[1];
    moveGenerator(moveList);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        copyBoard();
        if (makeMove(moveList->moves[moveCount], allMoves) == 0) {
            // skip to the next move
            continue;
        }

        // call perft driver recursively
        perftChild(depth - 1);
        takeBack();
    }
}

static inline void perft(int depth) {
    if (depth == 0) {
        nodes++;
        return;
    }
    moves moveList[1];
    moveGenerator(moveList);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        copyBoard();
        if (makeMove(moveList->moves[moveCount], allMoves) == 0) {
            // skip to the next move
            continue;
        }
        // call perft driver recursively
        perft(depth - 1);
        takeBack();
    }
}

// search position for the best move
void searchPosition(int depth) {
    // define best score variable
    int score;

    // reset "time is up" flag
    stopped = 0;

    // reset nodes counter
    nodes = 0;

    // reset follow PV flags
    followPV = 0;
    scorePV = 0;

    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyMoves, 0, sizeof(historyMoves));
    memset(pvTable, 0, sizeof(pvTable));
    memset(pvLength, 0, sizeof(pvLength));

    // define initial alpha beta bounds
    int alpha = -infinity;
    int beta = infinity;

    // iterative deepening
    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (stopped == 1) {
            break;
        }


        int firstTime = getTimeMiliSecond();
        followPV = 1;
        // find best move within a given position
        score = negamax(alpha, beta, current_depth);

        if (score <= alpha || score >= beta) {
            alpha = -infinity;
            beta = infinity;
            continue;
        }

        alpha = score - 50;
        beta = score + 50;

        int secondTime = getTimeMiliSecond() - firstTime;

        if (score > -mateValue && score < -mateScore)
            printf("info score mate %d depth %d nodes %llu time %d pv ", -(score + mateValue) / 2 - 1, current_depth,
                   nodes, secondTime);

        else if (score > mateScore && score < mateValue)
            printf("info score mate %d depth %d nodes %llu time %d pv ", (mateValue - score) / 2 + 1, current_depth,
                   nodes, secondTime);

        else
            printf("info score cp %d depth %d nodes %llu time %d pv ", score, current_depth, nodes, secondTime);

        //loop over the moves within a PV line
        for (int count = 0; count < pvLength[0]; count++) {
            printMove(pvTable[0][count]);
            printf(" ");
        }
        // print new line
        printf("\n");
    }
    // best move placeholder
    printf("bestmove ");
    printMove(pvTable[0][0]);
    printf("\n");
}

int areSubStringsEqual(char *command, char *uciCommand, int stringSize) {
    if (stringSize > strlen(command)) {
        return 0;
    }
    for (int index = 0; index < stringSize; index++) {
        if (*command != *uciCommand) {
            return 0;
        }
        command++;
        uciCommand++;
    }
    return 1;
}

// position evaluation
static inline int evaluate() {
    // get game phase score
    int game_phase_score = get_game_phase_score();

    // game phase (opening, middle game, endgame)
    int game_phase = -1;

    // pick up game phase based on game phase score
    if (game_phase_score > opening_phase_score) game_phase = opening;
    else if (game_phase_score < endgame_phase_score) game_phase = endgame;
    else game_phase = middlegame;


    // static evaluation score
    int score = 0;

    // current pieces bitboard copy
    U64 bitboard;

    // init piece & square
    int piece, square;

    // penalties
    int double_pawns = 0;

    // loop over piece bitboards
    for (int bb_piece = P; bb_piece <= k; bb_piece++) {
        // init piece bitboard copy
        bitboard = bitboards[bb_piece];

        // loop over pieces within a bitboard
        while (bitboard) {
            // init piece
            piece = bb_piece;

            // init square
            square = getLS1BIndex(bitboard);

            /*
                Now in order to calculate interpolated score
                for a given game phase we use this formula
                (same for material and positional scores):

                (
                  score_opening * game_phase_score +
                  score_endgame * (opening_phase_score - game_phase_score)
                ) / opening_phase_score

                E.g. the score for pawn on d4 at phase say 5000 would be
                interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
            */

            // interpolate scores in middle_game
            if (game_phase == middlegame)
                score += (
                                 material_score[opening][piece] * game_phase_score +
                                 material_score[endgame][piece] * (opening_phase_score - game_phase_score)
                         ) / opening_phase_score;

                // score material weights with pure scores in opening or endgame
            else score += material_score[game_phase][piece];

            // score positional piece scores
            switch (piece) {
                // evaluate white pawns
                case P:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][PAWN][square] * game_phase_score +
                                         positional_score[endgame][PAWN][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][PAWN][square];

                    /* double pawn penalty
                    double_pawns = count_bits(bitboards[P] & file_masks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1)
                        score += double_pawns * double_pawn_penalty;

                    // on isolated pawn
                    if ((bitboards[P] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score += isolated_pawn_penalty;

                    // on passed pawn
                    if ((white_passed_masks[square] & bitboards[p]) == 0)
                        // give passed pawn bonus
                        score += passed_pawn_bonus[get_rank[square]];
                    */
                    break;

                    // evaluate white knights
                case N:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][KNIGHT][square] * game_phase_score +
                                         positional_score[endgame][KNIGHT][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][KNIGHT][square];

                    break;

                    // evaluate white bishops
                case B:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][BISHOP][square] * game_phase_score +
                                         positional_score[endgame][BISHOP][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][BISHOP][square];

                    // mobility
                    //score += count_bits(get_bishop_attacks(square, occupancies[both]));

                    break;

                    // evaluate white rooks
                case R:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][ROOK][square] * game_phase_score +
                                         positional_score[endgame][ROOK][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][ROOK][square];

                    /* semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += open_file_score;
                    */
                    break;

                    // evaluate white queens
                case Q:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][QUEEN][square] * game_phase_score +
                                         positional_score[endgame][QUEEN][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][QUEEN][square];

                    // mobility
                    //score += count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate white king
                case K:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][KING][square] * game_phase_score +
                                         positional_score[endgame][KING][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][KING][square];

                    /* semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= open_file_score;

                    // king safety bonus
                    score += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
                    */
                    break;

                    // evaluate black pawns
                case p:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][PAWN][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][PAWN][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][PAWN][mirrorScore[square]];

                    /* double pawn penalty
                    double_pawns = count_bits(bitboards[p] & file_masks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1)
                        score -= double_pawns * double_pawn_penalty;

                    // on isolated pawnd
                    if ((bitboards[p] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score -= isolated_pawn_penalty;

                    // on passed pawn
                    if ((black_passed_masks[square] & bitboards[P]) == 0)
                        // give passed pawn bonus
                        score -= passed_pawn_bonus[get_rank[mirror_score[square]]];
                    */
                    break;

                    // evaluate black knights
                case n:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][KNIGHT][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][KNIGHT][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][KNIGHT][mirrorScore[square]];

                    break;

                    // evaluate black bishops
                case b:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][BISHOP][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][BISHOP][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][BISHOP][mirrorScore[square]];

                    // mobility
                    //score -= count_bits(get_bishop_attacks(square, occupancies[both]));
                    break;

                    // evaluate black rooks
                case r:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][ROOK][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][ROOK][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][ROOK][mirrorScore[square]];

                    /* semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= open_file_score;
                    */
                    break;

                    // evaluate black queens
                case q:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][QUEEN][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][QUEEN][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][QUEEN][mirrorScore[square]];

                    // mobility
                    //score -= count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate black king
                case k:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][KING][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][KING][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][KING][mirrorScore[square]];

                    /* semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += open_file_score;

                    // king safety bonus
                    score -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
                    */
                    break;
            }

            // pop ls1b
            popBit(bitboard, square);
        }
    }

    // return final evaluation based on side
    return (side == white) ? score : -score;
}

int input_waiting() {
#ifndef WIN32
    fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    } else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }

#endif
}


// read GUI/user input
void read_input() {
    // bytes to read holder
    int bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting()) {
        // tell engine to stop calculating
        stopped = 1;

        // loop to read bytes from STDIN
        do {
            // read bytes from STDIN
            bytes = read(fileno(stdin), input, 256);
        }

            // until bytes available
        while (bytes < 0);

        // searches for the first occurrence of '\n'
        endc = strchr(input, '\n');

        // if found new line set value at pointer to 0
        if (endc) *endc = 0;

        // if input is available
        if (strlen(input) > 0) {
            // match UCI "quit" command
            if (!strncmp(input, "quit", 4)) {
                // tell engine to terminate exacution
                quit = 1;
            }

                // // match UCI "stop" command
            else if (!strncmp(input, "stop", 4)) {
                // tell engine to terminate exacution
                quit = 1;
            }
        }
    }
}


void communicate() {
    // if time is up break here
    if (timeset == 1 && getTimeMiliSecond() > stoptime) {
        // tell engine to stop calculating
        stopped = 1;
    }
    //read_input();
}

// enable PV move scoring
static inline void enable_pv_scoring(moves *moveList) {
    // disable following PV
    followPV = 0;

    // loop over the moves within a move list
    for (int count = 0; count < moveList->count; count++) {
        // make sure we hit PV move
        if (pvTable[0][ply] == moveList->moves[count]) {
            // enable move scoring
            scorePV = 1;

            // enable following PV
            followPV = 1;
        }
    }
}

/*  =======================
         Move ordering
    =======================

    1. PV move
    2. Captures in MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
    6. Unsorted moves
*/


// score moves
static inline int scoreMove(int move) {
    // if PV move scoring is allowed
    if (scorePV) {
        // make sure we are dealing with PV move
        if (pvTable[0][ply] == move) {
            // disable score PV flag
            scorePV = 0;

            // give PV move the highest score to search it first
            return 20000;
        }
    }

    // score capture move
    if (getMoveCapture(move)) {
        // init target piece
        int target_piece = P;

        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;

        // pick up side to move
        if (side == white) {
            start_piece = p;
            end_piece = k;
        }
        else {
            start_piece = P;
            end_piece = K;
        }

        // loop over bitboards opposite to the current side to move
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
            // if there's a piece on the target square
            if (getBit(bitboards[bb_piece], getMoveTarget(move))) {
                // remove it from corresponding bitboard
                target_piece = bb_piece;
                break;
            }
        }

        // score move by MVV LVA lookup [source piece][target piece]
        return mvvLva[getMovePiece(move)][target_piece] + 10000;
    }

        // score quiet move
    else {
        // score 1st killer move
        if (killerMoves[0][ply] == move)
            return 9000;

            // score 2nd killer move
        else if (killerMoves[1][ply] == move)
            return 8000;

            // score history move
        else
            return historyMoves[getMovePiece(move)][getMoveTarget(move)];
    }


    return 0;
}

// sort moves in descending order
static inline int sort_moves(moves *moveList, int bestMove) {
    // move scores
    int move_scores[moveList->count];

    // score all the moves within a move list
    for (int count = 0; count < moveList->count; count++) {
        // if hash move available
        if (bestMove == moveList->moves[count])
            // score move
            move_scores[count] = 30000;

        else
            // score move
            move_scores[count] = scoreMove(moveList->moves[count]);
    }

    // loop over current move within a move list
    for (int current_move = 0; current_move < moveList->count; current_move++) {
        // loop over next move within a move list
        for (int next_move = current_move + 1; next_move < moveList->count; next_move++) {
            // compare current and next move scores
            if (move_scores[current_move] < move_scores[next_move]) {
                // swap scores
                int temp_score = move_scores[current_move];
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = temp_score;

                // swap moves
                int temp_move = moveList->moves[current_move];
                moveList->moves[current_move] = moveList->moves[next_move];
                moveList->moves[next_move] = temp_move;
            }
        }
    }
}

// position repetition detection
static inline int isRepetition() {
    // loop over repetition indicies range
    for (int index = 0; index < repetitionIndex; index++) {
        // if we found the hash kkey same with a current
        if (repetitionTable[index] == hashKey) {
            // we found a repetition
            return 1;
        }
    }

    // if no repetition found
    return 0;
}


// quiescence search
static inline int quiescence(int alpha, int beta) {
    if ((nodes & 2047) == 0) {
        communicate();
    }
    // increment nodes count
    nodes++;


    // evaluate position
    int evaluation = evaluate();

    // fail-hard beta cutoff
    if (evaluation >= beta) {
        // node (move) fails high
        return beta;
    }

    // found a better move
    if (evaluation > alpha) {
        // PV node (move)
        alpha = evaluation;
    }

    // create move list instance
    moves moveList[1];

    // generate moves
    moveGenerator(moveList);

    // sort moves
    sort_moves(moveList, 0);

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        // preserve board state
        copyBoard();

        // increment ply
        ply++;

        // increment repetition index & store hash key
        repetitionIndex++;
        repetitionTable[repetitionIndex] = hashKey;

        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], onlyCaptures) == 0) {
            // decrement ply
            ply--;

            // decrement repetition index
            repetitionIndex--;

            // skip to next move
            continue;
        }


        // score current move
        int score = -quiescence(-beta, -alpha);

        // decrement ply
        ply--;

        // decrement repetition index
        repetitionIndex--;

        // take move back
        takeBack();

        if (stopped == 1) return 0;


        // found a better move
        if (score > alpha) {
            // PV node (move)
            alpha = score;
            // fail-hard beta cutoff
            if (score >= beta) {
                // node (move) fails high
                return beta;
            }
        }
    }

    // node (move) fails low
    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth) {
    // variable to store current move's score (from the static evaluation perspective)
    int score;

    // best move (to store in TT)
    int bestMove = 0;

    // define hash flag
    int hashFlag = hashFlagAlpha;

    if ((nodes & 2047) == 0) {
        communicate();
    }

    if (ply && isRepetition()) {
        return 0;
    }

    // a hack by Pedro Castro to figure ot whether the crrent node is PV node or not
    int pvNode = beta - alpha > 1;

    // read hash entry
    if (ply && (score = readHashEntry(alpha, beta, &bestMove, depth)) != noHashEntry && pvNode == 0) {
        // if the move has already been searched (hence has a value)
        // we just retrn the score for this move
        return score;
    }
    // init PV length
    pvLength[ply] = ply;

    // recursion escapre condition
    if (depth == 0)
        // run quiescence search
        return quiescence(alpha, beta);



    // increment nodes count
    nodes++;

    // is king in check
    int in_check = isSquareAttacked((side == white) ? getLS1BIndex(bitboards[K]) :
                                    getLS1BIndex(bitboards[k]),
                                    side ^ 1);

    // increase search depth if the king has been exposed into a check
    if (in_check) depth++;

    // legal moves counter
    int legal_moves = 0;

    // get static evaluation score
    int static_eval = evaluate();

    // evaluation pruning / static null move pruning
    if (depth < 4 && !pvNode && !in_check && abs(beta - 1) > -infinity + 100) {
        // define evaluation margin
        int eval_margin = 100 * depth;

        // evaluation margin substracted from static evaluation score fails high
        if (static_eval - eval_margin >= beta)
            // evaluation margin substracted from static evaluation score
            return static_eval - eval_margin;
    }

    // null move pruning
    if (depth >= 3 && in_check == 0 && ply) {
        // preserve board state
        copyBoard();

        ply++;

        // increment repetition index & store hash key
        repetitionIndex++;
        repetitionTable[repetitionIndex] = hashKey;

        // hash enpassant if available
        if (enpassant != no_sq) { hashKey ^= enpassantKeys[enpassant]; }

        // reset enpassant capture square
        enpassant = no_sq;

        // switch the side, literally giving opponent an extra move to make
        side ^= 1;

        // hash the side
        hashKey ^= sideKey;


        /* search moves with reduced depth to find beta cutoffs
           depth - 1 - R where R is a reduction limit */
        score = -negamax(-beta, -beta + 1, depth - 1 - 2);

        // decrement ply
        ply--;

        // decrement repetition index
        repetitionIndex--;

        // restore board state
        takeBack();


        if (stopped == 1) return 0;

        // fail-hard beta cutoff
        if (score >= beta)

            // node (move) fails high
            return beta;
    }

    // razoring
    if (!pvNode && !in_check && depth <= 4) {
        // get static eval and add first bonus
        score = static_eval + 125;

        // define new score
        int new_score;

        // static evaluation indicates a fail-low node
        if (score < beta) {
            // on depth 1
            if (depth == 1) {
                // get quiscence score
                new_score = quiescence(alpha, beta);

                // return quiescence score if it's greater then static evaluation score
                return (new_score > score) ? new_score : score;
            }

            // add second bonus to static evaluation
            score += 175;

            // static evaluation indicates a fail-low node
            if (score < beta && depth <= 2) {
                // get quiscence score
                new_score = quiescence(alpha, beta);

                // quiescence score indicates fail-low node
                if (new_score < beta)
                    // return quiescence score if it's greater then static evaluation score
                    return (new_score > score) ? new_score : score;
            }
            // add third bonus to static evaluation
            score += 200;
            // static evaluation indicates a fail-low node
            if (score < beta && depth <= 3) {
                // get negamax score
                new_score = -negamax(-alpha, -beta, depth - 2);
                // quiescence score indicates fail-low node
                if (new_score < beta)
                    // return quiescence score if it's greater then static evaluation score
                    return (new_score > score) ? new_score : score;
            }
        }
    }

    // create move list instance
    moves moveList[1];

    // generate moves
    moveGenerator(moveList);

    // if we are now following PV line
    if (followPV)
        // enable PV move scoring
        enable_pv_scoring(moveList);

    // sort moves
    sort_moves(moveList, bestMove);

    // number of moves searched in a move list
    int moves_searched = 0;

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        // preserve board state
        copyBoard();

        // increment ply
        ply++;

        // increment repetition index & store hash key
        repetitionIndex++;
        repetitionTable[repetitionIndex] = hashKey;


        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], allMoves) == 0) {
            // decrement ply
            ply--;

            // decrement repetition index
            repetitionIndex--;

            // skip to next move
            continue;
        }

        // increment legal moves
        legal_moves++;

        // full depth search
        if (moves_searched == 0)
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, depth - 1);

            // late move reduction (LMR)
        else {
            // condition to consider LMR
            if (
                    moves_searched >= full_depth_moves &&
                    depth >= reduction_limit &&
                    in_check == 0 &&
                    getMoveCapture(moveList->moves[count]) == 0 &&
                    getMovePromoted(moveList->moves[count]) == 0
                    )
                // search current move with reduced depth:
                score = -negamax(-alpha - 1, -alpha, depth - 2);

                // hack to ensure that full-depth search is done
            else score = alpha + 1;

            // principle variation search PVS
            if (score > alpha) {
                /* Once you've found a move with a score that is between alpha and beta,
                   the rest of the moves are searched with the goal of proving that they are all bad.
                   It's possible to do this a bit faster than a search that worries that one
                   of the remaining moves might be good. */
                score = -negamax(-alpha - 1, -alpha, depth - 1);

                /* If the algorithm finds out that it was wrong, and that one of the
                   subsequent moves was better than the first PV move, it has to search again,
                   in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                   but generally not often enough to counteract the savings gained from doing the
                   "bad move proof" search referred to earlier. */
                if ((score > alpha) && (score < beta))
                    /* re-search the move that has failed to be proved to be bad
                       with normal alpha beta score bounds*/
                    score = -negamax(-beta, -alpha, depth - 1);
            }
        }

        // decrement ply
        ply--;

        // decrement repetition index
        repetitionIndex--;

        // take move back
        takeBack();

        if (stopped == 1) return 0;

        // increment the counter of moves searched so far
        moves_searched++;

        // found a better move
        if (score > alpha) {
            // switch hash flag from storing for fail-low node
            // to the one storing score for PV node
            hashFlag = hashFlagExact;

            // store best move (for TT)
            bestMove = moveList->moves[count];

            // on quiet moves
            if (getMoveCapture(moveList->moves[count]) == 0)
                // store history moves
                historyMoves[getMovePiece(moveList->moves[count])][getMoveTarget(moveList->moves[count])] += depth;

            // PV node (move)
            alpha = score;

            // write PV move
            pvTable[ply][ply] = moveList->moves[count];

            // loop over the next ply
            for (int next_ply = ply + 1; next_ply < pvLength[ply + 1]; next_ply++)
                // copy move from deeper ply into a current ply's line
                pvTable[ply][next_ply] = pvTable[ply + 1][next_ply];

            // adjust PV length
            pvLength[ply] = pvLength[ply + 1];

            // fail-hard beta cutoff
            if (score >= beta) {
                // store hash entry with the score equal to beta
                writeHashEntry(beta, bestMove, depth, hashFlagBeta);

                // on quiet moves
                if (getMoveCapture(moveList->moves[count]) == 0) {
                    // store killer moves
                    killerMoves[1][ply] = killerMoves[0][ply];
                    killerMoves[0][ply] = moveList->moves[count];
                }

                // node (move) fails high
                return beta;
            }
        }
    }

    // we don't have any legal moves to make in the current postion
    if (legal_moves == 0) {
        // king is in check
        if (in_check)
            // return mating score (assuming closest distance to mating position)
            return -mateValue + ply;

            // king is not in check
        else
            // return stalemate score
            return 0;
    }
    // store hash entry with the score equal to alpha
    writeHashEntry(alpha, bestMove, depth, hashFlag);

    // node (move) fails low
    return alpha;
}

int getTimeMiliSecond() {
#ifdef WIN64
    return GetTickCount();
#else
    struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}

int countBits(U64 bitboard) {
    return __builtin_popcountll(bitboard);
}

// get least significant 1st bit index
int getLS1BIndex(U64 bitboard) {
    return __builtin_ctzll(bitboard);
}

U64 setOccupancy(int index, int bitsInMask, U64 attackMask) {
    U64 occupancy = 0ULL;
    for (int count = 0; count < bitsInMask; count++) {
        int square = getLS1BIndex(attackMask);
        popBit(attackMask, square);
        if (index & (1 << count)) {
            occupancy |= (1ULL << square);
        }
    }
    return occupancy;
}

unsigned int getRandom32BitNumber() {
    //get current state
    unsigned int number = state;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // update random number state
    state = number;

    return number;
}

U64 getRandom64Numbers() {
    U64 n1, n2, n3, n4;

    n1 = (U64) (getRandom32BitNumber()) & 0xFFFF;
    n2 = (U64) (getRandom32BitNumber()) & 0xFFFF;
    n3 = (U64) (getRandom32BitNumber()) & 0xFFFF;
    n4 = (U64) (getRandom32BitNumber()) & 0xFFFF;

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generateMagicNumber() {
    return getRandom64Numbers() & getRandom64Numbers() & getRandom64Numbers();
}

U64 findMagicNumber(int square, int relevantBits, int bishop) {
    // init occupancies
    U64 magicOccupancies[4096];

    // init attack tables
    U64 attacks[4096];

    // init used attacks
    U64 usedAttacks[4096];

    // init attack mask for a current piece
    U64 attackMask = bishop ? maskBishopAttacks(square) : maskRookAttacks(square);

    // init occupancy indicies
    int occupancyIndicies = 1 << relevantBits;

    // loop over occupancy indicies
    for (int index = 0; index < occupancyIndicies; index++) {
        // init occupancies
        magicOccupancies[index] = setOccupancy(index, relevantBits, attackMask);

        // init attacks
        attacks[index] = bishop ? bishopAttack(square, magicOccupancies[index]) :
                         rookAttack(square, magicOccupancies[index]);
    }
    // test magic numbers loop
    for (int randomCount = 0; randomCount < 100000000; randomCount++) {

        // generate magic number candidate
        U64 magicNumber = generateMagicNumber();

        // skip inappropriate magic numbers
        if (countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6) continue;

        // init used attacks
        memset(usedAttacks, 0ULL, sizeof(usedAttacks));

        // init index & fail flag
        int index, fail;

        // test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancyIndicies; index++) {
            // init magic index
            int magicIndex = (int) ((magicOccupancies[index] * magicNumber) >> (64 - relevantBits));

            // if magic index works
            if (usedAttacks[magicIndex] == 0ULL) {
                // init used attacks
                usedAttacks[magicIndex] = attacks[index];
            } else if (usedAttacks[magicIndex] != attacks[index]) {
                // magic index doesn't work
                fail = 1;
            }
        }
        // if magic number works
        if (!fail) {
            return magicNumber;
        }
    }
    // if magic number doesn't work
    printf("  Magic number fails!\n");
    return 0ULL;
}

// init magic numbers
void initMagicNumbers() {
    // loop over 64 board squares
    for (int square = 0; square < 64; square++) {
        // init rook magic numbers
        rookMagic[square] = findMagicNumber(square, rookRelevantBits[square], rook);;
    }
    printf("\n");
    for (int square = 0; square < 64; square++) {
        // init bishop magic numbers
        bishopMagic[square] = findMagicNumber(square, bishopRelevantBits[square], bishop);
    }

}

// init slider piece's attack tables
void initSlidersAttacks(int bishop) {
    // init bishop & rook masks
    for (int square = 0; square < 64; square++) {
        bishopMask[square] = maskBishopAttacks(square);
        rookMask[square] = maskRookAttacks(square);

        // init current mask
        U64 attackMask = bishop ? bishopMask[square] : rookMask[square];

        // init relevant occupancy bit count
        int relevantBitsCount = countBits(attackMask);

        // init occupancyIndicies
        int occupancIndicies = 1 << relevantBitsCount;

        // loop over occupancy indicies
        for (int index = 0; index < occupancIndicies; index++) {
            // bishop
            if (bishop) {
                // int current occupancy variation
                U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

                // init magic index
                int magicIndex = (occupancy * bishopMagic[square]) >> (64 - bishopRelevantBits[square]);

                // init bishop attacks
                bishopAttacks[square][magicIndex] = bishopAttack(square, occupancy);
            }
                // rook
            else {
                U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

                // init magic index
                int magicIndex = (occupancy * rookMagic[square]) >> (64 - rookRelevantBits[square]);

                // init bishop attacks
                rookAttacks[square][magicIndex] = rookAttack(square, occupancy);
            }
        }
    }
}

// make move on chess board
static inline int makeMove(int move, int moveFlag) {
    // quiet moves
    if (moveFlag == allMoves) {
        // preserve board state
        copyBoard();

        // parse move
        int sourceSquare = getMoveSource(move);
        int targetSquare = getMoveTarget(move);
        int piece = getMovePiece(move);
        int promotedPiece = getMovePromoted(move);
        int capture = getMoveCapture(move);
        int doublePush = getMoveDouble(move);
        int enpass = getMoveEnpassant(move);
        int castling = getMoveCastling(move);

        // move piece
        popBit(bitboards[piece], sourceSquare);
        setBit(bitboards[piece], targetSquare);

        // hash piece
        hashKey ^= pieceKeys[piece][sourceSquare]; // remove piece from source square in hash key
        hashKey ^= pieceKeys[piece][targetSquare]; // set piece to the target square in hash key

        // handling capture moves
        if (capture) {
            int startPiece, endPiece;
            if (side == white) {
                startPiece = p;
                endPiece = k;
            } else {
                startPiece = P;
                endPiece = K;
            }
            for (int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++) {
                if (getBit(bitboards[bbPiece], targetSquare)) {
                    // remove it from corresponding bitboard
                    popBit(bitboards[bbPiece], targetSquare);

                    // remove the piece from hash key
                    hashKey ^= pieceKeys[bbPiece][targetSquare];
                    break;
                }
            }
        }
        // handle pawn promotions
        if (promotedPiece) {
            // white to move
            if (side == white) {
                // erase the pawn from the target square
                popBit(bitboards[P], targetSquare);

                // remove pawn from hash key
                hashKey ^= pieceKeys[P][targetSquare];
            }

                // black to move
            else {
                // erase the pawn from the target square
                popBit(bitboards[p], targetSquare);

                // remove pawn from hash key
                hashKey ^= pieceKeys[p][targetSquare];
            }

            // set up promoted piece on chess board
            setBit(bitboards[promotedPiece], targetSquare);

            // add promoted piece into the hash key
            hashKey ^= pieceKeys[promotedPiece][targetSquare];
        }

        // handle enpassant captures
        if (enpass) {
            // erase the pawn depending on side to move
            (side == white) ? popBit(bitboards[p], targetSquare + 8) :
            popBit(bitboards[P], targetSquare - 8);

            // white to move
            if (side == white) {
                // remove captured pawn
                popBit(bitboards[p], targetSquare + 8);

                // remove pawn from hash key
                hashKey ^= pieceKeys[p][targetSquare + 8];
            }

                // black to move
            else {
                // remove captured pawn
                popBit(bitboards[P], targetSquare - 8);

                // remove pawn from hash key
                hashKey ^= pieceKeys[P][targetSquare - 8];
            }
        }

        // hash enpassant if available (remove enpassant square from hash key )
        if (enpassant != no_sq) hashKey ^= enpassantKeys[enpassant];

        // reset enpassant square
        enpassant = no_sq;

        // handle double pawn push
        if (doublePush) {
            // white to move
            if (side == white) {
                // set enpassant square
                enpassant = targetSquare + 8;

                // hash enpassant
                hashKey ^= enpassantKeys[targetSquare + 8];
            }

                // black to move
            else {
                // set enpassant square
                enpassant = targetSquare - 8;

                // hash enpassant
                hashKey ^= enpassantKeys[targetSquare - 8];
            }
        }

        // handle castling moves
        if (castling) {
            switch (targetSquare) {
                // white castles king side
                case (g1):
                    // move H rook
                    popBit(bitboards[R], h1);
                    setBit(bitboards[R], f1);

                    // hash rook
                    hashKey ^= pieceKeys[R][h1];  // remove rook from h1 from hash key
                    hashKey ^= pieceKeys[R][f1];  // put rook on f1 into a hash key
                    break;

                    // white castles queen side
                case (c1):
                    // move A rook
                    popBit(bitboards[R], a1);
                    setBit(bitboards[R], d1);

                    // hash rook
                    hashKey ^= pieceKeys[R][a1];  // remove rook from a1 from hash key
                    hashKey ^= pieceKeys[R][d1];  // put rook on d1 into a hash key
                    break;

                    // black castles king side
                case (g8):
                    // move H rook
                    popBit(bitboards[r], h8);
                    setBit(bitboards[r], f8);

                    // hash rook
                    hashKey ^= pieceKeys[r][h8];  // remove rook from h8 from hash key
                    hashKey ^= pieceKeys[r][f8];  // put rook on f8 into a hash key
                    break;

                    // black castles queen side
                case (c8):
                    // move A rook
                    popBit(bitboards[r], a8);
                    setBit(bitboards[r], d8);

                    // hash rook
                    hashKey ^= pieceKeys[r][a8];  // remove rook from a8 from hash key
                    hashKey ^= pieceKeys[r][d8];  // put rook on d8 into a hash key
                    break;
            }
        }


        // hash castling
        hashKey ^= castleKeys[castle];

        // update castling rights
        castle &= castlingRights[sourceSquare];
        castle &= castlingRights[targetSquare];

        // hash castling
        hashKey ^= castleKeys[castle];

        // reset occupancies
        occupancies[white] = 0LL;
        occupancies[black] = 0LL,
                occupancies[both] = 0LL;

        // loop over white pieces bitboards
        for (int bbPiece = P; bbPiece <= K; bbPiece++) {
            // update white occupancies
            occupancies[white] |= bitboards[bbPiece];
        }
        // loop over black pieces bitboards
        for (int bbPiece = p; bbPiece <= k; bbPiece++) {
            // update black occupancies
            occupancies[black] |= bitboards[bbPiece];
        }
        // update both side occupancies
        occupancies[both] |= occupancies[white];
        occupancies[both] |= occupancies[black];

        // change side
        side ^= 1;

        // hash side
        hashKey ^= sideKey;

        U64 hash_from_scratch = generateHashKey();

        // make sure that king has not been exposed into a check
        if (isSquareAttacked((side == white) ? getLS1BIndex(bitboards[k]) : getLS1BIndex(bitboards[K]), side)) {
            // take move back
            takeBack();
            // return illegal move
            return 0;
        }
        return 1;
    } else {
        if (getMoveCapture(move)) {
            makeMove(move, allMoves);
        } else {
            return 0;
        }
    }
}

// add move to the move list
static inline void addMove(moves *moveList, int move) {
    // store move
    moveList->moves[moveList->count] = move;
    // increment move count
    moveList->count++;
}

// get bishop attacks
static inline U64 getBishopAttacks(int square, U64 occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishopMask[square];
    occupancy *= bishopMagic[square];
    occupancy >>= 64 - bishopRelevantBits[square];
    return bishopAttacks[square][occupancy];
}

// get rook attacks
static inline U64 getRookAttacks(int square, U64 occupancy) {
    // get rook attacks assuming current board occupancy
    occupancy &= rookMask[square];
    occupancy *= rookMagic[square];
    occupancy >>= 64 - rookRelevantBits[square];
    return rookAttacks[square][occupancy];
}

// get queen attacks
static inline U64 getQueenAttacks(int square, U64 occupancy) {
    // get queen attacks assuming current board occupancy
    U64 queenAttacks;
    U64 bishopOccupancy = occupancy;
    U64 rookOccupancy = occupancy;
    bishopOccupancy &= bishopMask[square];
    bishopOccupancy *= bishopMagic[square];
    bishopOccupancy >>= 64 - bishopRelevantBits[square];
    queenAttacks = bishopAttacks[square][bishopOccupancy];

    rookOccupancy &= rookMask[square];
    rookOccupancy *= rookMagic[square];
    rookOccupancy >>= 64 - rookRelevantBits[square];
    queenAttacks |= rookAttacks[square][rookOccupancy];
    return queenAttacks;
}

static inline int isSquareAttacked(int square, int whichSide) {
    if ((whichSide == white) && (pawnAtacks[black][square] & bitboards[P])) {
        return 1;
    }
    if ((whichSide == black) && (pawnAtacks[white][square] & bitboards[p])) {
        return 1;
    }
    if (knightAttacks[square] & ((whichSide == white) ? bitboards[N] : bitboards[n])) {
        return 1;
    }
    if (getBishopAttacks(square, occupancies[both]) & ((whichSide == white) ? bitboards[B] : bitboards[b])) {
        return 1;
    }
    if (kingAttacks[square] & ((whichSide == white) ? bitboards[K] : bitboards[k])) {
        return 1;
    }
    if (getQueenAttacks(square, occupancies[both]) & ((whichSide == white) ? bitboards[Q] : bitboards[q])) {
        return 1;
    }
    if (getRookAttacks(square, occupancies[both]) & ((whichSide == white) ? bitboards[R] : bitboards[r])) {
        return 1;
    }
    return 0;
}

void printAttackedSquares(int whichSide) {
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (!file) {
                printf("  %d  ", 8 - rank);
            }
            printf("%d ", isSquareAttacked(square, whichSide) ? 1 : 0);

        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n\n");

}

void initAll() {
    initLeaperAttacks();
    initMagicNumbers();
    initSlidersAttacks(bishop);
    initSlidersAttacks(rook);
    // init random keys for tranposition table
    initRandomKeys();
    // clear hash table
    clearHashTable();
}

U64 maskPawnAttacks(int isWhite, int square) {
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

    // set piece on board
    setBit(bitboard, square);

    // white pawns
    if (!isWhite) {
        // generate pawn attacks
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
        // black pawns
    } else {
        // generate pawn attacks
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    }
    // return attack map
    return attacks;
}

U64 maskKnightAttacks(int square) {
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    setBit(bitboard, square);

    if (bitboard & not8RankAndABFile) { attacks |= bitboard >> 10; }

    if (bitboard & not1RankAndGHFile) { attacks |= bitboard << 10; }

    if (bitboard & not1And2RankHFile) { attacks |= bitboard << 17; }

    if (bitboard & not8And7RankAFile) { attacks |= bitboard >> 17; }

    if (bitboard & not8And7RankHFile) { attacks |= bitboard >> 15; }

    if (bitboard & not1And2RanksAFile) { attacks |= bitboard << 15; }

    if (bitboard & not1RankAndABFile) { attacks |= bitboard << 6; }

    if (bitboard & not8RankAndGHFile) { attacks |= bitboard >> 6; }

    return attacks;
}

U64 maskKingAttacks(int square) {
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    setBit(bitboard, square);

    if (bitboard & not8Rank) { attacks |= bitboard >> 8; }

    if (bitboard & not1Rank) { attacks |= bitboard << 8; }

    if (bitboard & notAFileAndHRank) { attacks |= bitboard << 9; }

    if (bitboard & not8RankAndAFile) { attacks |= bitboard >> 9; }

    if (bitboard & not8RankAndHFile) { attacks |= bitboard >> 7; }

    if (bitboard & not1RankAndAFile) { attacks |= bitboard << 7; }

    if (bitboard & notAFile) { attacks |= bitboard >> 1; }

    if (bitboard & notHFile) { attacks |= bitboard << 1; }

    return attacks;
}

U64 maskBishopAttacks(int square) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1, f0 = file + 1; r0 < 7 && f0 < 7; r0++, f0++) {
        attacks |= (1ULL << (r0 * 8 + f0));
    }
    for (int r1 = rank - 1, f1 = file - 1; r1 > 0 && f1 > 0; r1--, f1--) {
        attacks |= (1ULL << (r1 * 8 + f1));
    }
    for (int r2 = rank + 1, f2 = file - 1; r2 < 7 && f2 > 0; r2++, f2--) {
        attacks |= (1ULL << (r2 * 8 + f2));
    }
    for (int r3 = rank - 1, f3 = file + 1; r3 > 0 && f3 < 7; r3--, f3++) {
        attacks |= (1ULL << (r3 * 8 + f3));
    }
    return attacks;
}

U64 bishopAttack(int square, U64 block) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1, f0 = file + 1; r0 < 8 && f0 < 8; r0++, f0++) {
        attacks |= (1ULL << (r0 * 8 + f0));
        if ((1ULL << (r0 * 8 + f0)) & block) { break; }
    }
    for (int r1 = rank - 1, f1 = file - 1; r1 > -1 && f1 > -1; r1--, f1--) {
        attacks |= (1ULL << (r1 * 8 + f1));
        if ((1ULL << (r1 * 8 + f1)) & block) { break; }
    }
    for (int r2 = rank + 1, f2 = file - 1; r2 < 8 && f2 > -1; r2++, f2--) {
        attacks |= (1ULL << (r2 * 8 + f2));
        if ((1ULL << (r2 * 8 + f2)) & block) { break; }
    }
    for (int r3 = rank - 1, f3 = file + 1; r3 > -1 && f3 < 8; r3--, f3++) {
        attacks |= (1ULL << (r3 * 8 + f3));
        if ((1ULL << (r3 * 8 + f3)) & block) { break; }
    }
    return attacks;
}

U64 maskRookAttacks(int square) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1; r0 < 7; r0++) {
        attacks |= 1ULL << (r0 * 8 + file);
    }
    for (int f0 = file + 1; f0 < 7; f0++) {
        attacks |= 1ULL << (rank * 8 + f0);
    }
    for (int r1 = rank - 1; r1 > 0; r1--) {
        attacks |= 1ULL << (r1 * 8 + file);
    }
    for (int f1 = file - 1; f1 > 0; f1--) {
        attacks |= 1ULL << (rank * 8 + f1);
    }
    return attacks;
}

U64 rookAttack(int square, U64 block) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1; r0 < 8; r0++) {
        attacks |= 1ULL << (r0 * 8 + file);
        if ((1ULL << (r0 * 8 + file)) & block) { break; }
    }
    for (int f0 = file + 1; f0 < 8; f0++) {
        attacks |= 1ULL << (rank * 8 + f0);
        if ((1ULL << (rank * 8 + f0)) & block) { break; }
    }
    for (int r1 = rank - 1; r1 > -1; r1--) {
        attacks |= 1ULL << (r1 * 8 + file);
        if ((1ULL << (r1 * 8 + file)) & block) { break; }
    }
    for (int f1 = file - 1; f1 > -1; f1--) {
        attacks |= 1ULL << (rank * 8 + f1);
        if ((1ULL << (rank * 8 + f1)) & block) { break; }
    }
    return attacks;
}

void initLeaperAttacks() {
    for (int square = 0; square < 64; square++) {
        // init pawn attacks
        pawnAtacks[white][square] = maskPawnAttacks(white, square);
        pawnAtacks[black][square] = maskPawnAttacks(black, square);

        // init knight attacks
        knightAttacks[square] = maskKnightAttacks(square);

        // init king attacks
        kingAttacks[square] = maskKingAttacks(square);
        //printf("Target Square %d \n", square);
        //pBitboard(kingAttacks[square]);
    }
}

// generate all moves
static inline void moveGenerator(moves *moveList) {
    // init move count
    moveList->count = 0;


    // define source & target squares
    int source_square, target_square;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;

    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++) {
        // init piece bitboard copy
        bitboard = bitboards[piece];

        // generate white pawns & white king castling moves
        if (side == white) {
            // pick up white pawn bitboards index
            if (piece == P) {
                // loop over white pawns within white pawn bitboard
                while (bitboard) {
                    // init source square
                    source_square = getLS1BIndex(bitboard);

                    // init target square
                    target_square = source_square - 8;

                    // generate quiet pawn moves
                    if (!(target_square < a8) && !(getBit(occupancies[both], target_square))) {
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, R, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, B, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, N, 0, 0, 0, 0));
                        } else {
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                            // two squares ahead pawn move
                            if ((source_square >= a2 && source_square <= h2) &&
                                !(getBit(occupancies[both], (target_square - 8))))
                                addMove(moveList, encodeMove(source_square, (target_square - 8), piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawnAtacks[side][source_square] & occupancies[black];

                    // generate pawn captures
                    while (attacks) {
                        // init target square
                        target_square = getLS1BIndex(attacks);

                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, R, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, B, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, N, 1, 0, 0, 0));
                        } else
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b of the pawn attacks
                        popBit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (enpassant != no_sq) {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawnAtacks[side][source_square] & (1ULL << enpassant);

                        // make sure enpassant capture available
                        if (enpassant_attacks) {
                            // init enpassant capture target square
                            int target_enpassant = getLS1BIndex(enpassant_attacks);
                            addMove(moveList, encodeMove(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
                }
            }

            // castling moves
            if (piece == K) {
                // king side castling is available
                if (castle & wk) {
                    // make sure square between king and king's rook are empty
                    if (!(getBit(occupancies[both], f1)) && !(getBit(occupancies[both], g1))) {
                        // make sure king and the f1 squares are not under attacks
                        if (!isSquareAttacked(e1, black) && !isSquareAttacked(f1, black))
                            addMove(moveList, encodeMove(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }

                // queen side castling is available
                if (castle & wq) {
                    // make sure square between king and queen's rook are empty
                    if (!(getBit(occupancies[both], d1)) && !(getBit(occupancies[both], c1)) &&
                        !(getBit(occupancies[both], b1))) {
                        // make sure king and the d1 squares are not under attacks
                        if (!isSquareAttacked(e1, black) && !isSquareAttacked(d1, black))
                            addMove(moveList, encodeMove(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

            // generate black pawns & black king castling moves
        else {
            // pick up black pawn bitboards index
            if (piece == p) {
                // loop over white pawns within white pawn bitboard
                while (bitboard) {
                    // init source square
                    source_square = getLS1BIndex(bitboard);

                    // init target square
                    target_square = source_square + 8;

                    // generate quiet pawn moves
                    if (!(target_square > h1) && !(getBit(occupancies[both], target_square))) {
                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, q, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, r, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, b, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, n, 0, 0, 0, 0));
                        } else {
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                            // two squares ahead pawn move
                            if ((source_square >= a7 && source_square <= h7) &&
                                !(getBit(occupancies[both], (target_square + 8))))
                                addMove(moveList, encodeMove(source_square, (target_square + 8), piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawnAtacks[side][source_square] & occupancies[white];

                    // generate pawn captures
                    while (attacks) {
                        // init target square
                        target_square = getLS1BIndex(attacks);

                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, q, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, r, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, b, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, n, 1, 0, 0, 0));
                        } else
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b of the pawn attacks
                        popBit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (enpassant != no_sq) {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawnAtacks[side][source_square] & (1ULL << enpassant);

                        // make sure enpassant capture available
                        if (enpassant_attacks) {
                            // init enpassant capture target square
                            int target_enpassant = getLS1BIndex(enpassant_attacks);
                            addMove(moveList, encodeMove(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
                }
            }

            // castling moves
            if (piece == k) {
                // king side castling is available
                if (castle & bk) {
                    // make sure square between king and king's rook are empty
                    if (!(getBit(occupancies[both], f8)) && !(getBit(occupancies[both], g8))) {
                        // make sure king and the f8 squares are not under attacks
                        if (!isSquareAttacked(e8, white) && !isSquareAttacked(f8, white))
                            addMove(moveList, encodeMove(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }

                // queen side castling is available
                if (castle & bq) {
                    // make sure square between king and queen's rook are empty
                    if (!(getBit(occupancies[both], d8)) && !(getBit(occupancies[both], c8)) &&
                        !(getBit(occupancies[both], b8))) {
                        // make sure king and the d8 squares are not under attacks
                        if (!isSquareAttacked(e8, white) && !isSquareAttacked(d8, white))
                            addMove(moveList, encodeMove(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

        // genarate knight moves
        if ((side == white) ? piece == N : piece == n) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = knightAttacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((side == white) ? occupancies[black] : occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }

        // generate bishop moves
        if ((side == white) ? piece == B : piece == b) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = getBishopAttacks(source_square, occupancies[both]) &
                          ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((side == white) ? occupancies[black] : occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }

        // generate rook moves
        if ((side == white) ? piece == R : piece == r) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = getRookAttacks(source_square, occupancies[both]) &
                          ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((side == white) ? occupancies[black] : occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }

        // generate queen moves
        if ((side == white) ? piece == Q : piece == q) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = getQueenAttacks(source_square, occupancies[both]) &
                          ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((side == white) ? occupancies[black] : occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }
        // generate king moves
        if ((side == white) ? piece == K : piece == k) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = kingAttacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((side == white) ? occupancies[black] : occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }
    }
}