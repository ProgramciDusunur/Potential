//
// Created by erena on 4.03.2025.
//

#ifndef POTENTIAL_SEE_H
#define POTENTIAL_SEE_H

#include "structs.h"
#include "fen.h"
#include "move.h"
#include "search.h"

void initSuits(void);
void testSEE(void);
void init_helper_bb(void);
void update_pinned(board *pos);

#endif //POTENTIAL_SEE_H
