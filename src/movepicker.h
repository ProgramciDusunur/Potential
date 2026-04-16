#include "structs.h"
#include "values.h"
#include "move.h"
#include "search.h"

void init_mp(MovePicker *mp, uint16_t tt_move);
uint16_t get_next_move(MovePicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss);
void init_qs_mp(QSMovePicker *mp, bool should_do_evasions);
uint16_t get_next_qs_move(QSMovePicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss);