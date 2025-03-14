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

typedef struct {
    U64 bitboards[12];
    U64 occupancies[3];
    uint8_t mailbox[64];
    int side;
    int castle;
    int enpassant;

    int ply;
    int nmpPly;

    U64 repetitionTable[1000];
    int repetitionIndex;

    int pvLength[maxPly];
    int pvTable[maxPly][maxPly];
    int killerMoves[maxPly][2];

    int staticEval[maxPly];

    int isSingularMove[maxPly];

    int followPv;
    int scorePv;

    //int depth;
    U64 hashKey;
    U64 pawnKey;
    U64 minorKey;
    U64 whiteNonPawnKey;
    U64 blackNonPawnKey;

    int gamePhase;
} board;


// transposition table data structure
typedef struct {
    uint32_t hashKey;    // "almost" unique chess position identifier
    uint8_t depth;       // current search depth
    uint8_t flag;        // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    int16_t score;       // score (alpha/beta/PV)
    bool ttPv;           // tt was pv node or not
    int bestMove;        // best move from the search
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
    U64 whiteNonPawnKeyCopy;
    U64 blackNonPawnKeyCopy;

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
} time;





#endif //POTENTIAL_STRUCTS_H
