#include "movepicker.h"

void init_mp(MovePicker *mp, uint16_t tt_move) {
    mp->tt_move = tt_move;
    mp->CURRENT_STAGE = STAGE_TT;
    mp->index = 0;
    mp->good_noisy_index = 0;
    mp->bad_noisy_index = 0;
    mp->quiet_index = 0;
    mp->bad_noisy.count = 0;
}

uint16_t get_next_move(MovePicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss) {
    switch (mp->CURRENT_STAGE) {
        case STAGE_TT:
            mp->CURRENT_STAGE = STAGE_GEN_NOISY;
            if (mp->tt_move != 0 && is_pseudo_legal(mp->tt_move, pos)) {
                return mp->tt_move;
            }
        // fallthrough
        case STAGE_GEN_NOISY:
            mp->CURRENT_STAGE = STAGE_GOOD_NOISY;
            legal_noisy_generator(&mp->good_noisy, pos);
            if (pos->followPv) enable_pv_scoring(&mp->good_noisy, pos);
            init_move_scores(&mp->good_noisy, move_scores, mp->tt_move, t, ss);
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
            mp->CURRENT_STAGE = STAGE_GEN_QUIET;
        // fallthrough
        case STAGE_GEN_QUIET:
            mp->CURRENT_STAGE = STAGE_QUIET;
            legal_quiet_generator(&mp->quiet, pos);
            if (pos->followPv) enable_pv_scoring(&mp->quiet, pos);
            init_move_scores(&mp->quiet, move_scores, mp->tt_move, t, ss);
            mp->quiet_index = 0;
        // fallthrough
        case STAGE_QUIET:
            while (mp->quiet_index < mp->quiet.count) {
                pick_next_move(mp->quiet_index, &mp->quiet, move_scores);
                
                uint16_t candidate_move = mp->quiet.moves[mp->quiet_index];
                mp->quiet_index++;

                if (candidate_move == mp->tt_move) {
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
    return 0;
}

void init_qs_mp(QSMovePicker *mp, bool should_do_evasions) {
    mp->CURRENT_STAGE = STAGE_QS_GEN_NOISY;
    mp->index = 0;
    mp->bad_noisy_index = 0;
    mp->quiet_index = 0;
    mp->bad_noisy.count = 0;
    mp->should_do_evasions = should_do_evasions;
}

uint16_t get_next_qs_move(QSMovePicker *mp, int *move_scores, board *pos, ThreadData *t, SearchStack *ss) {
    switch (mp->CURRENT_STAGE) {
        case STAGE_QS_GEN_NOISY:
            mp->CURRENT_STAGE = STAGE_QS_GOOD_NOISY;
            legal_noisy_generator(&mp->captures, pos);
            if (mp->should_do_evasions) {
                init_move_scores(&mp->captures, move_scores, 0, t, ss);
            } else {
                init_quiescence_scores(&mp->captures, move_scores, pos);
            }
            mp->index = 0;
        // fallthrough
        case STAGE_QS_GOOD_NOISY:
            while (mp->index < mp->captures.count) {
                pick_next_move(mp->index, &mp->captures, move_scores);
                uint16_t candidate_move = mp->captures.moves[mp->index];
                mp->index++;
                                
                if (!mp->should_do_evasions) {
                    return candidate_move;
                }
                
                if (move_scores[mp->index - 1] >= 0) {
                    return candidate_move;
                } else {
                    mp->bad_noisy.moves[mp->bad_noisy.count++] = candidate_move;
                }
            }
                        
            if (!mp->should_do_evasions) return 0;
            
            mp->CURRENT_STAGE = STAGE_QS_GEN_QUIET;
        // fallthrough
        case STAGE_QS_GEN_QUIET:
            mp->CURRENT_STAGE = STAGE_QS_QUIET;
            legal_quiet_generator(&mp->quiet, pos);
            init_move_scores(&mp->quiet, move_scores, 0, t, ss);
            mp->quiet_index = 0;
        // fallthrough
        case STAGE_QS_QUIET:
            if (mp->quiet_index < mp->quiet.count) {
                pick_next_move(mp->quiet_index, &mp->quiet, move_scores);
                uint16_t candidate_move = mp->quiet.moves[mp->quiet_index];
                mp->quiet_index++;
                return candidate_move;
            }
            mp->CURRENT_STAGE = STAGE_QS_BAD_NOISY;
        // fallthrough
        case STAGE_QS_BAD_NOISY:
            if (mp->bad_noisy_index < mp->bad_noisy.count) {
                uint16_t candidate_move = mp->bad_noisy.moves[mp->bad_noisy_index];
                mp->bad_noisy_index++;
                return candidate_move;
            }
            return 0;
    }
    return 0;
}