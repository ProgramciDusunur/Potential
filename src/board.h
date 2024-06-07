//
// Created by erena on 29.05.2024.
//
#pragma once
#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H

#endif //CHESSENGINE_BOARD_H



#ifndef U64
#define U64 unsigned long long
#endif

#define maxPly 64


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

    int killerMoves[2][maxPly];
    int historyMoves[12][maxPly];

    int followPv;
    int scorePv;

    //int depth;
    U64 hashKey;
} board;