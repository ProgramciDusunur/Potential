//
// Created by erena on 13.09.2024.
//
#pragma once

#ifndef POTENTIAL_STRUCTS_H
#define POTENTIAL_STRUCTS_H

#include "stdint.h"

#ifndef U64
#define U64 unsigned long long
#endif

#define maxPly 64

typedef struct {
    U64 bitboards[12];
    U64 occupancies[3];
    uint8_t mailbox[64];
    int side;
    int castle;
    int enpassant;

    int ply;

    U64 repetitionTable[1000];
    int repetitionIndex;

    int pvLength[maxPly];
    int pvTable[maxPly][maxPly];

    int staticEval[maxPly];

    double improvingRate[maxPly];


    int followPv;
    int scorePv;

    int rootBestMove;

    //int depth;
    U64 hashKey;

    int gamePhase;
} board;



// transposition table data structure
typedef struct {
    U64 hashKey;     // "almost" unique chess position identifier
    int depth;       // current search depth
    int flag;       // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    int score;       // score (alpha/beta/PV)
    int bestMove;
} tt;                 // transposition table (TT aka hash table)



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
    int sideCopy;
    int enpassantCopy;
    int castleCopy;
};

typedef struct {
    // exit from engine flag
    int quit;
    // UCI "movestogo" command moves counter
    int movestogo;

    // UCI "movetime" command timeControl counter
    int movetime;

    // UCI "timeControl" command holder (ms)
    int time;

    // UCI "inc" command's timeControl increment holder
    int inc;

    // UCI "starttime" command timeControl holder
    int starttime;

    int softLimit;

    int hardLimit;

    uint64_t startTime;

    uint32_t baseSoft;

    uint32_t maxTime;

    // UCI "stoptime" command timeControl holder
    int stoptime;

    // variable to flag timeControl control availability
    int timeset;

    // variable to flag when the timeControl is up
    int stopped;
} timeControl;





#endif //POTENTIAL_STRUCTS_H
