#include "structs.h"
#include "values.h"
#include "move.h"
#include "search.h"

void init_mp(MovePicker *mp, uint16_t tt_move);
void init_qs_mp(QSearchPicker *mp, uint16_t tt_move);
uint16_t get_next_move(MovePicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss);
uint16_t qs_get_next_move(QSearchPicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss, bool evasions_available);