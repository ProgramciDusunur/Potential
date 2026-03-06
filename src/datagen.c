#include "datagen.h"
#include "generate_fen.h"
#include "search.h"
#include "timeman.h"
#include "threads.h"
#include "perft.h"

#define MAX_GAME_PLYS 256

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit) {
    board pos;
    parseFEN(startPosition, &pos);
    
    for (int i = 0; i < 8; ++i) {
        moves moveList[1];
        moveGenerator(moveList, &pos);
        
        int legal_moves[256];
        int legal_count = 0;

        for (int j = 0; j < moveList->count; ++j) {
            struct copyposition cp;
            copyBoard(&pos, &cp);
            if (makeMove(moveList->moves[j], allMoves, &pos)) {
                legal_moves[legal_count++] = moveList->moves[j];
            }        
            takeBack(&pos, &cp);
        }
        
        if (legal_count == 0) return 0;
        
        int random_idx = get_random_uint64_number() % legal_count;
        int selected_move = legal_moves[random_idx];

        struct copyposition cp;
        copyBoard(&pos, &cp);
        makeMove(selected_move, allMoves, &pos);
    }
    
    pos.repetitionIndex = 0;
    pos.fifty = 0;
    pos.repetitionTable[pos.repetitionIndex++] = pos.hashKey;

    char fen_list[MAX_GAME_PLYS][100];
    int fen_count = 0;
    
    my_time time;
    initTimeControl(&time);
    time.isNodeLimit = 1;
    time.node_limit = nodes_limit;

    double result = 0.5;
    int game_over = 0;
    int illegal = 0;

    int win_adj_count = 0;
    int draw_adj_count = 0;

    while (fen_count < MAX_GAME_PLYS) {
        moves moveList[1];
        moveGenerator(moveList, &pos);
        
        int legal_moves = 0;
        for (int i = 0; i < moveList->count; i++) {
            struct copyposition cp;
            copyBoard(&pos, &cp);
            if (makeMove(moveList->moves[i], allMoves, &pos)) {
                legal_moves++;
            }
            takeBack(&pos, &cp);
        }
        
        if (legal_moves == 0) {
            int in_check = isSquareAttacked((pos.side == white) ? getLS1BIndex(pos.bitboards[K]) :
                                             getLS1BIndex(pos.bitboards[k]), pos.side, &pos);
            if (in_check) {
                result = pos.side == white ? 0.0 : 1.0; 
            } else {
                result = 0.5;
            }
            game_over = 1;
            break;
        }

        if (pos.fifty >= 100 || isMaterialDraw(&pos)) {
            result = 0.5;
            game_over = 1;
            break;
        }

        int rep_found = 0;
        for (int r = 0; r < pos.repetitionIndex - 1; r++) {
            if (pos.repetitionTable[r] == pos.hashKey) {
                rep_found = 1;
                break;
            }
        }
        if (rep_found) {
            result = 0.5;
            game_over = 1;
            break;
        }

        FenString fen_str = get_fen(&pos);
        snprintf(fen_list[fen_count++], 100, "%s", fen_str.str);

        resetTimeControl(&time);
        time.isNodeLimit = 1;
        time.node_limit = nodes_limit;
        
        setup_main_thread(&pos);
        thread_pool.threads[0]->pos.ply = 0;    
        thread_pool.threads[0]->pos.fifty = pos.fifty;
        
        start_helpers(&pos, 0, &time);
        int score = searchPosition(maxPly, true, thread_pool.threads[0], &time);
        wait_helpers();

        // Win Adjudication
        if (abs(score) > 1000) win_adj_count++; else win_adj_count = 0;
        if (win_adj_count >= 5) {
            result = score > 0 ? (pos.side == white ? 1.0 : 0.0) : (pos.side == white ? 0.0 : 1.0);
            game_over = 1;
            break;
        }

        // Draw Adjudication
        if (abs(score) < 10) draw_adj_count++; else draw_adj_count = 0;
        if (draw_adj_count >= 20) {
            result = 0.5;
            game_over = 1;
            break;
        }

        uint16_t best_move = thread_pool.threads[0]->pos.pvTable[0][0];

        if (best_move == 0) {
            illegal = 1;
            break;
        }
        
        struct copyposition cp;
        copyBoard(&pos, &cp);
        if (!makeMove(best_move, allMoves, &pos)) {
            illegal = 1;
            break;
        }
        
        pos.repetitionTable[pos.repetitionIndex++] = pos.hashKey;
    }

    if (!game_over && fen_count == MAX_GAME_PLYS) {
        result = 0.5; 
    }

    for (int i = 0; i < fen_count; ++i) {
        if (illegal) {
            fprintf(illegal_file, "%s | Illegal Move\n", fen_list[i]);
        } else {
            fprintf(out_file, "%s | %.1f\n", fen_list[i], result);
        }
    }

    return fen_count;
}
