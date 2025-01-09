#include <stdio.h>



#include "uci.h"
#include "magic.h"
#include "move.h"
#include "table.h"
#include "fen.h"
#include "perft.h"



void initAll(void) {
    initLeaperAttacks();
    initMagicNumbers();
    initSlidersAttacks(bishop);
    initSlidersAttacks(rook);
    // init random keys for tranposition table
    initRandomKeys();
    // clear hash table
    clearHashTable();
    // init mask
    initEvaluationMasks();
    // init Late Move Reduction Table
    initializeLMRTable();
    // init tranposition table
    init_hash_table(64);
}

int main(int argc, char* argv[]) {
    initAll();
    int debug = 0;
    if (debug) {
        board position;
        parseFEN("5r2/P7/1Bk5/2P5/8/8/K7/8 b - - 88 192", &position);

        // create move list instance
        moves moveList[1], badQuiets[1];
        badQuiets->count = 0;

        // generate moves
        moveGenerator(moveList, &position);

        for (int count = 0; count < moveList->count; count++) {
            struct copyposition copyPosition;
            // preserve board state
            copyBoard(&position, &copyPosition);

            // increment ply
            position.ply++;

            // increment repetition index & store hash key
            position.repetitionIndex++;
            position.repetitionTable[position.repetitionIndex] = position.hashKey;

            // make sure to make only legal moves
            if (makeMove(moveList->moves[count], allMoves, &position) == 0) {
                // decrement ply
                position.ply--;

                // decrement repetition index
                position.repetitionIndex--;

                printMove(moveList->moves[count]);
                printf("\n");

                // skip to next move
                continue;
            }


            // decrement ply
            position.ply--;

            // decrement repetition index
            position.repetitionIndex--;

            // take move back
            takeBack(&position, &copyPosition);

        }


        //perftRoot(8, &position);
        //printf("Nodes: %llu", perftNodes);
    } else {
        uciProtocol(argc, argv);
    }
    return 0;
}
