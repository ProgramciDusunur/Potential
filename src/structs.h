//
// Created by erena on 13.09.2024.
//
#pragma once

#ifndef POTENTIAL_STRUCTS_H
#define POTENTIAL_STRUCTS_H

#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>

#ifndef U64
#define U64 unsigned long long
#endif

#define maxPly 256
#define MAX_THREADS 512

typedef struct  {
    uint64_t pawnThreats;
    uint64_t knightThreats;
    uint64_t bishopThreats;
    uint64_t rookThreats;
    uint64_t queenThreats;
    uint64_t kingThreats;
    uint64_t stmThreats[2];    
} threats;

typedef struct {
    U64 bitboards[12];
    U64 occupancies[3];
    U64 hashKey;
    U64 pawnKey;
    U64 minorKey;
    U64 majorKey;
    U64 whiteNonPawnKey;
    U64 blackNonPawnKey;
    U64 krpKey;
    uint8_t mailbox[64];
    int side;
    int castle;
    int enpassant;
    int fifty;
    int full_moves;
    int phase_score;

    U64 pinned[2];
    int ply;
    int seldepth;
    uint8_t rootDepth;
    int nmpPly;

    U64 repetitionTable[1000];
    int repetitionIndex;

    int pvLength[maxPly];
    int pvTable[maxPly][maxPly];
    threats pieceThreats;

    U64 nodes_searched;

    bool benchmark;

    int followPv;
    int scorePv;

    int gamePhase;
} board;

struct copyposition {
    U64 bitboards[12];
    U64 occupancies[3];
    U64 hashKey;
    U64 pawnKey;
    U64 minorKey;
    U64 majorKey;
    U64 whiteNonPawnKey;
    U64 blackNonPawnKey;
    U64 krpKey;
    uint8_t mailbox[64];
    int side;
    int castle;
    int enpassant;
    int fifty;
    int full_moves;
    int phase_score;
};

// transposition table data structure
typedef struct {
    uint64_t hashKey;    // "almost" unique chess position identifier
    uint16_t bestMove;        // best move from the search
    int16_t score;       // score (alpha/beta/PV)
    uint8_t depth;       // current search depth
    uint8_t flag;        // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    bool ttPv;           // tt was pv node or not
} tt;                    // transposition table (TT aka hash table)


// move list structure
typedef struct {
    // moves
    uint16_t moves[256];

    // move count
    int count;
} moves;

typedef struct {
    // exit from engine flag
    int quit;
    // UCI "movestogo" command moves counter
    int movestogo;

    // UCI "movetime" command time counter
    int movetime;

    // UCI "time" command holder (ms)
    int time;

    // UCI "inc" command's time increment holder
    int inc;

    // UCI "starttime" command time holder
    int starttime;

    int softLimit;

    int hardLimit;

    uint64_t startTime;

    uint32_t baseSoft;

    uint32_t maxTime;
    
    uint32_t node_limit;
    bool isNodeLimit;

    // UCI "stoptime" command time holder
    int stoptime;

    // variable to flag time control availability
    int timeset;

    // variable to flag when the time is up
    int stopped;
} my_time;

typedef struct  {
    char *fen;
    uint16_t move;
    int score;
} see;

typedef struct {
    char str[128]; // FEN string    
} FenString;

typedef struct {
    int cutoff_count;
    int staticEval;
    uint16_t singular_move;
    uint16_t nmp_refutation_move;
    uint16_t move;
    uint16_t piece;
} SearchStack;

typedef struct {
    // quietHistory[side to move][fromSquare][toSquare][threatSource][threatTarget]
    int16_t quietHistory[2][64][64][2][2];

    // continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
    int16_t continuationHistory[12][64][12][64];

    // continuationCorrectionHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
    int16_t contCorrhist[12][64][12][64];

    // pawnHistory [pawnKey][piece][to]
    int16_t pawnHistory[2048][12][64];

    // captureHistory [piece][toSquare][capturedPiece]
    int16_t captureHistory[12][64][13];


    // pawn correction history [side to move][key]
    int16_t pawn_corrhist[2][16384];

    // minor correction history [side to move][key]
    int16_t minor_corrhist[2][16384];

    // major correction history [side to move][key]
    int16_t major_corrhist[2][16384];

    // non pawn correction history [side to move][key]
    int16_t non_pawn_corrhist[2][2][16384];

    // king rook pawn correction history [side to move][key]
    int16_t krp_corrhist[2][16384];
} SearchData;

typedef struct {
    _Atomic uint64_t nodes_searched;
    int16_t seldepth;
    int16_t rootDepth;
    _Atomic bool stopped;
} SearchInfo;

typedef struct {
    int16_t id;
    SearchInfo search_i;
    SearchData search_d;
    board pos;
    SearchStack ss_base[maxPly + 20]; // 20 = STACK_SAFETY_MARGIN
    SearchStack *ss;                   // points to ss_base + STACK_OFFSET (10)
} ThreadData;

typedef struct {
    ThreadData *threads[MAX_THREADS];
    int thread_count;

    _Atomic bool stop;    
    board root_pos;
} ThreadPool;

#endif //POTENTIAL_STRUCTS_H
