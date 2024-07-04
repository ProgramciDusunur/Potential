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
