//
// Created by erena on 11.09.2024.
//

#ifndef POTENTIAL_PERF_H
#define POTENTIAL_PERF_H
#include "board.h"
#include "move.h"

extern U64 variant;
extern U64 nodes;

extern void perftRoot(int depth, board* position);
extern void perft(int depth, board* position);
extern void perftChild(int depth, board* position);

#endif //POTENTIAL_PERF_H
