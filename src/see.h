//
// Created by erena on 4.03.2025.
//

#ifndef POTENTIAL_SEE_H
#define POTENTIAL_SEE_H

#include "structs.h"
#include "fen.h"
#include "move.h"
#include "search.h"

int move_estimated_value(board *pos, uint16_t move);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
int SEE(board *pos, uint16_t move, int threshold);
void initSuits(void);
void testSEE(void);

#endif //POTENTIAL_SEE_H
