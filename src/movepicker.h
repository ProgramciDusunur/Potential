#include "structs.h"
#include "values.h"
#include "move.h"
#include "search.h"

void init_mp(MovePicker *mp, uint16_t tt_move);
uint16_t get_next_move(MovePicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss);

void init_probcut_mp(ProbcutPicker *mp, uint16_t tt_move);
uint16_t get_next_probcut_move(ProbcutPicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss);