//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_FEN_H
#define POTENTIAL_FEN_H

#include "board_constants.h"
#include "structs.h"
#include "bit_manipulation.h"
#include "table.h"
#include <string.h>
#include "evaluation.h"

#pragma once

#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define emptyBoard "8/8/8/8/8/8/8/8 w - - 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40"

void parseFEN(char *fen, board* position);


#endif //POTENTIAL_FEN_H
