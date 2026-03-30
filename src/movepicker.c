#include "movepicker.h"

void init_mp(MovePicker *mp, uint16_t tt_move) {
    mp->tt_move = tt_move;
    mp->CURRENT_STAGE = STAGE_TT;
    mp->index = 0;
}

uint16_t get_next_move(MovePicker *mp, moves *moveList, int *move_scores, board *pos, ThreadData *t, SearchStack *ss) {
    while (true) {
        switch (mp->CURRENT_STAGE) {
            
            case STAGE_TT:
                mp->CURRENT_STAGE = STAGE_ALL_REMAINING;
                
                
                if (mp->tt_move != 0 && is_pseudo_legal(mp->tt_move, pos)) {
                    return mp->tt_move;
                }
                break;
            case STAGE_ALL_REMAINING:                
                if (mp->index == 0) {
                    moveGenerator(moveList, pos);

                    // if we are now following PV line
                    if (pos->followPv)
                        // enable PV move scoring
                        enable_pv_scoring(moveList, pos);

                    init_move_scores(moveList, move_scores, mp->tt_move, t, ss);
                }
                if (mp->index < moveList->count) {
                    pick_next_move(mp->index, moveList, move_scores);
                    
                    uint16_t candidate_move = moveList->moves[mp->index];
                    mp->index++;
                    if (candidate_move == mp->tt_move) {
                        continue;
                    }
                    return candidate_move;
                }
                return 0;
        }
    }
}