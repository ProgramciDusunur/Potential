//
// Created by erena on 29.05.2024.
//
#pragma once


#include "board_constants.h"

#define maxPly 64


extern const int castlingRights[64];

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
    U64 hashKey;

    int gamePhase;
} board;

extern void printAttackedSquares(int whichSide, board* position);

