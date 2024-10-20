//
// Created by erena on 13.09.2024.
//
#pragma once

#ifndef POTENTIAL_STRUCTS_H
#define POTENTIAL_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>

#ifndef U64
#define U64 unsigned long long
#endif

#define maxPly 256

typedef struct {
    U64 bitboards[12];
    U64 occupancies[3];
    int side;
    int castle;
    int enpassant;

    int ply;

    U64 repetitionTable[1000];
    int repetitionIndex;

    int pvLength[maxPly];
    int pvTable[maxPly][maxPly];
    int killerMoves[maxPly][2];

    int staticEval[maxPly];

    double improvingRate[maxPly];

    int followPv;
    int scorePv;

    //int depth;
    bool inCheck;
    U64 hashKey;

    int gamePhase;
} board;



// transposition table data structure
typedef struct {
    U64 hashKey;     // "almost" unique chess position identifier
    int16_t depth;       // current search depth
    int8_t flag;       // flag the type of node (fail-high(score >= beta)/fail-low(score < alpha))
    int16_t score;       // score (alpha/beta/PV)
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

    // UCI "movetime" command time counter
    int movetime;

    // UCI "time" command holder (ms)
    int time;

    // UCI "inc" command's time increment holder
    int inc;

    // UCI "starttime" command time holder
    int starttime;

    // UCI "stoptime" command time holder
    int stoptime;

    // variable to flag time control availability
    int timeset;

    // variable to flag when the time is up
    int stopped;
} time;


// search stack
typedef struct {
    int continuationHistory[2][64][64];
} SearchStack;




#endif //POTENTIAL_STRUCTS_H
