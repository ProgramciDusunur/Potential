#include "structs.h"
#include "values.h"
#include "move.h"
#include "search.h"

void init_mp(MovePicker *mp, uint16_t tt_move);
uint16_t get_next_move(MovePicker *mp, moves *moveList, int *move_scores, board *pos, ThreadData *t, SearchStack *ss);