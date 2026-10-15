#ifndef POTENTIAL_PAWN_PAWN_H
#define POTENTIAL_PAWN_PAWN_H

#include "structs.h"
#include "board_constants.h"
#include <stdint.h>

#define PAWN_PAWN_INPUTS 4560
#define MAX_ACTIVE_PAWN_PAIRS 128

void init_pawn_pawn_masks(void);
void get_all_pawn_pawn_features(const board *pos, int *w_feats, int *w_count, int *b_feats, int *b_count);

#endif
