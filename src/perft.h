//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_PERFT_H
#define POTENTIAL_PERFT_H

#pragma once

#include "structs.h"
#include "move.h"
#include "fen.h"


extern char* perftSuitFens[126];
extern U64 perftNodes;
extern U64 variant;

void perftChild(int depth, board* position);
void perftRoot(int depth, board* position);
void perft(int depth, board* position);
void perftSuite();


#endif //POTENTIAL_PERFT_H
