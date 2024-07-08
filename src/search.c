//
// Created by erena on 31.05.2024.
//



#include "search.h"


//int counterMoves[2][maxPly][maxPly];


// position repetition detection
static inline int isRepetition(board* position) {
    // loop over repetition indicies range
    for (int index = 0; index < position->repetitionIndex; index++) {
        // if we found the hash key same with a current
        if (position->repetitionTable[index] == position->hashKey) {
            // we found a repetition
            return 1;
        }
    }

    // if no repetition found
    return 0;
}

void initializeLMRTable() {
    for (int i = 1; i < maxPly; ++i) {
        for (int j = 1; j < maxPly; ++j) {
            lmrTable[i][j] = round(1.2 + log(i) * log(j) * 0.6);
        }
    }
}

int getLmrReduction(int depth, int moveNumber, bool isPv) {
    int reduction = lmrTable[depth][moveNumber];
    return reduction;
}
