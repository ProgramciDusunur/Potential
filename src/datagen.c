#include "datagen.h"
#include "generate_fen.h"
#include "search.h"
#include "timeman.h"
#include "threads.h"
#include "perft.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#define MAX_GAME_PLYS 400

_Atomic uint64_t total_fens_generated = 0;
_Atomic uint64_t games_played_count = 0;
uint64_t global_start_time = 0;

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit, int use_book, ThreadData *t);
void datagen_worker(int thread_id, uint64_t games_target, int nodes_limit, int use_book);

char **book_lines = NULL;
int book_size = 0;
int book_capacity = 0;
int book_loaded = 0;

void load_book(const char* filename) {
    if (book_loaded) return;
    book_loaded = 1;

    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("info string Could not open book %s, falling back to random plies.\n", filename);
        return;
    }

    book_capacity = 10000;
    book_lines = (char **)malloc(book_capacity * sizeof(char *));

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) > 5) {
            if (book_size >= book_capacity) {
                book_capacity *= 2;
                book_lines = (char **)realloc(book_lines, book_capacity * sizeof(char *));
            }
            int len = strlen(line);
            book_lines[book_size] = (char *)malloc(len + 1);
            strcpy(book_lines[book_size], line);
            book_size++;
        }
    }
    fclose(f);
    printf("info string Loaded %d book positions.\n", book_size);
}

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit, int use_book, ThreadData *t) {
    board pos;
    if (use_book && book_size == 0) {
        load_book("UHO_Lichess_4852_v1.epd");
    }

    if (use_book && book_size > 0) {
        int random_idx = get_random_uint64_number() % book_size;
        parseFEN(book_lines[random_idx], &pos);
    } else {
        parseFEN(startPosition, &pos);
    }
    
    // Play 8 random plies to reduce draw rate and increase diversity
    for (int i = 0; i < 8; ++i) {
        moves moveList[1];
        moveGenerator(moveList, &pos);
        
        int legal_moves_arr[256];
        int legal_count = 0;

        for (int j = 0; j < moveList->count; ++j) {
            struct copyposition cp;
            copyBoard(&pos, &cp);
            if (makeMove(moveList->moves[j], allMoves, &pos)) {
                legal_moves_arr[legal_count++] = moveList->moves[j];
            }        
            takeBack(&pos, &cp);
        }
        
        if (legal_count == 0) return 0;
        
        int random_idx = get_random_uint64_number() % legal_count;
        int selected_move = legal_moves_arr[random_idx];

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
                                             getLS1BIndex(pos.bitboards[k]), pos.side ^ 1, &pos);
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
        time.is_datagen = true;
        
        memcpy(&t->pos, &pos, sizeof(board));
        t->pos.ply = 0;    
        t->pos.fifty = pos.fifty;
        
        // Reset node count per move so the engine searches properly
        atomic_store_explicit(&t->search_i.nodes_searched, 0, memory_order_relaxed);
        t->search_i.stopped = false;
        
        int score = searchPosition(maxPly, true, t, &time);

        uint16_t best_move = t->pos.pvTable[0][0];

        // Win Adjudication (300 cp corresponds to +3 pawns on the tuned P=100 scale)
        if (abs(score) > 300) win_adj_count++; else win_adj_count = 0;
        if (win_adj_count >= 4) {
            result = score > 0 ? (pos.side == white ? 1.0 : 0.0) : (pos.side == white ? 0.0 : 1.0);
            game_over = 1;
            break;
        }

        // Draw Adjudication
        if (abs(score) < 10) draw_adj_count++; else draw_adj_count = 0;
        if (draw_adj_count >= 8) {
            result = 0.5;
            game_over = 1;
            break;
        }

        // printf("DEBUG: score=%d, win_adj=%d, draw_adj=%d, fifty=%d, move=%d\n", score, win_adj_count, draw_adj_count, pos.fifty, fen_count);
        
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

void datagen_worker(int thread_id, uint64_t target_games, int nodes_limit, int use_book) {
    char out_filename[256];
    char illegal_filename[256];
    snprintf(out_filename, sizeof(out_filename), "datagen/datagen_%d.txt", thread_id);
    snprintf(illegal_filename, sizeof(illegal_filename), "datagen/illegal_%d.txt", thread_id);

    FILE *out_file = fopen(out_filename, "a");
    FILE *illegal_file = fopen(illegal_filename, "a");

    if (!out_file || !illegal_file) {
        if (out_file) fclose(out_file);
        if (illegal_file) fclose(illegal_file);
        return;
    }

    ThreadData *t = thread_pool.threads[thread_id];

    while (1) {
        uint64_t current_game = atomic_fetch_add_explicit(&games_played_count, 1, memory_order_relaxed);
        if (current_game >= target_games) {
            break;
        }

        uint64_t new_fens = play_selfgen_game(out_file, illegal_file, nodes_limit, use_book, t);
        uint64_t current_total = (uint64_t)atomic_fetch_add_explicit(&total_fens_generated, new_fens, memory_order_relaxed) + new_fens;
        
        uint64_t finished_games = current_game + 1;
        if (finished_games % 100 == 0) {
            uint64_t elapsed = getTimeMiliSecond() - global_start_time;
            if (elapsed == 0) elapsed = 1;
            printf("Played %" PRIu64 " Games... (%" PRIu64 " FENs, %" PRIu64 " FEN/s)\n", finished_games, current_total, current_total * 1000 / elapsed);
        }
    }

    fclose(out_file);
    fclose(illegal_file);
}

struct datagen_args {
    int id;
    uint64_t target;
    int nodes;
    int book;
};

void* datagen_worker_proxy(void* arg) {
    struct datagen_args *args = (struct datagen_args *)arg;
    datagen_worker(args->id, args->target, args->nodes, args->book);
    return NULL;
}
