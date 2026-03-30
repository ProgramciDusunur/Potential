#include "movepicker.h"

void init_mp(MovePicker *mp, uint16_t tt_move) {
    mp->tt_move = tt_move;
    mp->CURRENT_STAGE = STAGE_TT;
}

uint16_t next_move(uint16_t move, MovePicker *mp, moves *moveList, board *pos) {
    uint16_t next_move = 0;
    switch (mp->CURRENT_STAGE) {
        case STAGE_TT:
            if (mp->tt_move != 0) {
                next_move = mp->tt_move;
            }
            mp->CURRENT_STAGE = STAGE_TT;
            return next_move;
        case STAGE_ALL_REMAINING:
            moveGenerator(moveList, pos);



    }
    return next_move;
}