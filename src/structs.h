//
// Created by erena on 13.09.2024.
//
#pragma once

#ifndef POTENTIAL_STRUCTS_H
#define POTENTIAL_STRUCTS_H

#include <stdbool.h>
#include "stdint.h"

#ifndef U64
#define U64 unsigned long long
#endif

#define maxPly 256

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
    int16_t material_scaling;
} evaluation;

typedef struct {
    U64 bitboards[12];
    U64 occupancies[3];
    uint8_t mailbox[64];
    int side;
    int castle;
    int enpassant;

    int ply;
    int seldepth;
    int nmpPly;

    U64 repetitionTable[1000];
    int repetitionIndex;

    int pvLength[maxPly];
    int pvTable[maxPly][maxPly];
    int killerMoves[maxPly][2];
    threats pieceThreats;
    evaluation eval;

    int fifty;

    int staticEval[maxPly];

    int isSingularMove[maxPly];

    uint16_t piece[maxPly];
    int move[maxPly];

    bool benchmark;

    int followPv;
    int scorePv;

    //int depth;
    U64 hashKey;
    U64 pawnKey;
    U64 minorKey;
    U64 majorKey;
    U64 whiteNonPawnKey;
    U64 blackNonPawnKey;

    int gamePhase;
} board;


// transposition table data structure
typedef struct {
    uint64_t hashKey;    // "almost" unique chess position identifier
    int bestMove;        // best move from the search
    int16_t score;       // score (alpha/beta/PV)
    uint8_t depth;       // current search depth
    uint8_t flag;        // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))    
    bool ttPv;           // tt was pv node or not    
} tt;                    // transposition table (TT aka hash table)


// move list structure
typedef struct {
    // moves
    int moves[256];

    // move count
    int count;
} moves;

struct copyposition {
    U64 bitboardsCopy[12];
    U64 occupanciesCopy[3];
    uint8_t mailboxCopy[64];

    U64 hashKeyCopy;
    U64 pawnKeyCopy;
    U64 minorKeyCopy;
    U64 majorKeyCopy;
    U64 whiteNonPawnKeyCopy;
    U64 blackNonPawnKeyCopy;

    int fiftyCopy;

    int sideCopy;
    int enpassantCopy;
    int castleCopy;
};

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

    // UCI "stoptime" command time holder
    int stoptime;

    // variable to flag time control availability
    int timeset;

    // variable to flag when the time is up
    int stopped;
} my_time;

typedef struct  {
    char *fen;
    int move;
    int score;
} see;



#endif //POTENTIAL_STRUCTS_H
