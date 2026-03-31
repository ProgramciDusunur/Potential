#include "movepicker.h"

void init_mp(MovePicker *mp, uint16_t tt_move) {
    mp->tt_move = tt_move;
    mp->CURRENT_STAGE = STAGE_TT;
    mp->index = 0;
    mp->good_noisy_index = 0;
    mp->quiet_index = 0;
    mp->bad_noisy_index = 0;
    mp->bad_noisy.count = 0;
}

uint16_t get_next_move(MovePicker *mp, moves *moveList, int *move_scores, board *pos, ThreadData *t, SearchStack *ss) {
    while (true) {
        switch (mp->CURRENT_STAGE) {
            case STAGE_TT:
                mp->CURRENT_STAGE = STAGE_GEN_NOISY;                                
                if (mp->tt_move != 0 && is_pseudo_legal(mp->tt_move, pos)) {
                    return mp->tt_move;
                }
            // fallthrough
            case STAGE_GEN_NOISY:
                mp->CURRENT_STAGE = STAGE_GOOD_NOISY;
                noisyGenerator(&mp->good_noisy, pos);
                if (pos->followPv) enable_pv_scoring(&mp->good_noisy, pos);
                score_noisy_moves(&mp->good_noisy, move_scores, t, ss);
                mp->good_noisy_index = 0;
            // fallthrough
            case STAGE_GOOD_NOISY:
                while (mp->good_noisy_index < mp->good_noisy.count) {
                    pick_next_move(mp->good_noisy_index, &mp->good_noisy, move_scores);
                    
                    uint16_t candidate_move = mp->good_noisy.moves[mp->good_noisy_index];
                    mp->good_noisy_index++;
                    
                    if (candidate_move == mp->tt_move) {
                        continue;
                    }
                                        
                    if (move_scores[mp->good_noisy_index - 1] >= 0) {
                        return candidate_move;
                    } else {
                        mp->bad_noisy.moves[mp->bad_noisy.count++] = candidate_move;
                        continue;
                    }
                }
                mp->CURRENT_STAGE = STAGE_ALL_REMAINING;
                mp->index = 0; // reset index for STAGE_ALL_REMAINING
                
            // fallthrough
            /*case STAGE_GEN_QUIET: // TODO: should test separately
                mp->CURRENT_STAGE = STAGE_QUIET;
                quietGenerator(&mp->generate_quiet, pos);
            // fallthrough
            case STAGE_QUIET:
                mp->CURRENT_STAGE = STAGE_ALL_REMAINING;*/
            // fallthrough
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
                    
                    if (getMoveCapture(candidate_move) || getMovePromote(candidate_move)) {
                        continue;
                    }

                    return candidate_move;
                }
                mp->CURRENT_STAGE = STAGE_BAD_NOISY;
                
            // fallthrough
            case STAGE_BAD_NOISY:
                if (mp->bad_noisy_index < mp->bad_noisy.count) {
                    uint16_t candidate_move = mp->bad_noisy.moves[mp->bad_noisy_index];
                    mp->bad_noisy_index++;
                    return candidate_move;
                }
                return 0;
        }
    }
}