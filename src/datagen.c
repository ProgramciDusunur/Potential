#include "datagen.h"

#define MAX_GAME_PLYS 400

_Atomic uint64_t total_fens_generated = 0;
_Atomic uint64_t games_played_count = 0;
_Atomic uint64_t target_fens_limit = 0;
uint64_t global_start_time = 0;
char datagen_book_path[1024] = "UHO_Lichess_4852_v1.epd"; // Default book

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit, int use_book, ThreadData *t);
void datagen_worker(int thread_id, uint64_t games_target, int nodes_limit, int use_book);

char **book_lines = NULL;
int book_size = 0;
int book_capacity = 0;
int book_loaded = 0;
pthread_mutex_t book_mutex = PTHREAD_MUTEX_INITIALIZER;

void load_book(const char* filename) {
    pthread_mutex_lock(&book_mutex);
    if (book_loaded) {
        pthread_mutex_unlock(&book_mutex);
        return;
    }
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
    pthread_mutex_unlock(&book_mutex);
}

const int phase_value[12] = { 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0 };

inline int get_phase(board *pos) {
    int phase = 0;
    for (int p = N; p <= Q; p++) phase += countBits(pos->bitboards[p]) * phase_value[p];
    for (int p = n; p <= q; p++) phase += countBits(pos->bitboards[p]) * phase_value[p];
    if (phase > 24) phase = 24;
    return phase;
}

bool filtering(board *pos, int score, uint16_t best_move) {
    bool should_filter = false;

    // 1. Filter positions with mate scores
    if (abs(score) >= mateFound) should_filter = true;

    // 2. Filter positions in check
    int in_check = isSquareAttacked((pos->side == white) ? getLS1BIndex(pos->bitboards[K]) :
                                             getLS1BIndex(pos->bitboards[k]), pos->side ^ 1, pos);
    if (in_check) should_filter = true;

    // 3. Filter positions where the best move is noisy (captures or promotions)
    if (best_move != 0 && isTactical(best_move)) should_filter = true;

    // 4. Phase Distribution Resampling
    if (!should_filter) {
        int phase = get_phase(pos);
        // Phase 24 -> 100% keep. Phase 0 -> ~15% keep.
        double keep_prob = 0.15 + (0.85 * (double)phase / 24.0); 
        double rand_val = (get_random_uint64_number() % 1000) / 1000.0;
        
        if (rand_val > keep_prob) {
            should_filter = true;
        }
    }

    return should_filter;
}

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit, int use_book, ThreadData *t) {
    board pos;
    if (use_book && book_size == 0) {
        load_book(datagen_book_path);
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

        // --- FILTERING ---
        bool skip_save = filtering(&pos, score, best_move);

        // Save if not filtered
        if (!skip_save && fen_count < MAX_GAME_PLYS) {
            snprintf(fen_list[fen_count++], 100, "%s", fen_str.str);
        }
        // --- FILTERING END ---

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

    return illegal ? 0 : fen_count;
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
        // Check game limit
        uint64_t current_game = atomic_load_explicit(&games_played_count, memory_order_relaxed);
        if (target_games > 0 && current_game >= target_games) {
            break;
        }

        // Check FEN limit
        uint64_t current_fens = atomic_load_explicit(&total_fens_generated, memory_order_relaxed);
        uint64_t f_limit = atomic_load_explicit(&target_fens_limit, memory_order_relaxed);
        if (f_limit > 0 && current_fens >= f_limit) {
            break;
        }

        atomic_fetch_add_explicit(&games_played_count, 1, memory_order_relaxed);

        uint64_t new_fens = play_selfgen_game(out_file, illegal_file, nodes_limit, use_book, t);
        uint64_t current_total = (uint64_t)atomic_fetch_add_explicit(&total_fens_generated, new_fens, memory_order_relaxed) + new_fens;
        
        if ((current_game + 1) % 100 == 0) {
            uint64_t elapsed = getTimeMiliSecond() - global_start_time;
            if (elapsed == 0) elapsed = 1;
            printf("Played %" PRIu64 " Games... (%" PRIu64 " FENs, %" PRIu64 " FEN/s)\n", current_game + 1, current_total, current_total * 1000 / elapsed);
        }
    }

    fclose(out_file);
    fclose(illegal_file);
}

void* datagen_worker_proxy(void* arg) {
    struct datagen_args *args = (struct datagen_args *)arg;
    datagen_worker(args->id, args->target, args->nodes, args->book);
    return NULL;
}
