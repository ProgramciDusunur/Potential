#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 16
int num_threads = 4;

enum { P, N, B, R, Q, K, p, n, b, r, q, k, NO_PIECE };

const int phase_value[12] = { 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0 };

int char_to_piece(char c) {
    switch (c) {
        case 'P': return P; case 'N': return N; case 'B': return B;
        case 'R': return R; case 'Q': return Q; case 'K': return K;
        case 'p': return p; case 'n': return n; case 'b': return b;
        case 'r': return r; case 'q': return q; case 'k': return k;
        default: return NO_PIECE;
    }
}

typedef struct {
    int pieces[64];
    int side;
    int phase;
    double result;
    int score;
} TunerEntry;

typedef enum { WORK_NONE, WORK_MSE, WORK_GRAD, WORK_QUIT } WorkType;

typedef struct {
    int id;
    TunerEntry *data;
    int count;
    int start_index;
    int end_index;
    double sigmoid_k;
    double total_error;
    double grad[2][6][64];
    double grad_mat[2][6];
} TunerWorker;

TunerWorker workers[MAX_THREADS];
pthread_t threads[MAX_THREADS];

pthread_mutex_t tuner_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  tuner_cond_workers = PTHREAD_COND_INITIALIZER;
pthread_cond_t  tuner_cond_main = PTHREAD_COND_INITIALIZER;
int active_workers = 0;
WorkType current_work = WORK_NONE;
int work_id = 0;

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

double get_time_ms() {
#ifdef _WIN32
    return (double)GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
#endif
}

void run_job(WorkType job) {
    pthread_mutex_lock(&tuner_mutex);
    current_work = job;
    active_workers = num_threads;
    work_id++;
    pthread_cond_broadcast(&tuner_cond_workers);
    while (active_workers > 0) {
        pthread_cond_wait(&tuner_cond_main, &tuner_mutex);
    }
    pthread_mutex_unlock(&tuner_mutex);
}

int parse_entry(const char *line, TunerEntry *entry) {
    for (int i = 0; i < 64; i++) entry->pieces[i] = NO_PIECE;

    int square = 0;
    int i = 0;

    while (line[i] && line[i] != ' ') {
        if (line[i] == '/') { i++; continue; }
        if (line[i] >= '1' && line[i] <= '8') {
            square += (line[i] - '0');
        } else {
            int piece = char_to_piece(line[i]);
            if (piece == NO_PIECE) return 0;
            entry->pieces[square] = piece;
            square++;
        }
        i++;
    }

    if (square != 64) return 0;

    while (line[i] == ' ') i++;
    entry->side = (line[i] == 'w') ? 0 : 1;

    entry->phase = 0;
    for (int sq = 0; sq < 64; sq++) {
        if (entry->pieces[sq] != NO_PIECE) {
            entry->phase += phase_value[entry->pieces[sq]];
        }
    }
    if (entry->phase > 24) entry->phase = 24;

    const char *first_pipe = strstr(line, "|");
    if (first_pipe) {
        const char *second_pipe = strstr(first_pipe + 1, "|");
        if (second_pipe) {
            entry->score = atoi(first_pipe + 1);
            entry->result = atof(second_pipe + 1);
            if (strstr(second_pipe, "Illegal")) return 0;
        } else {
            entry->result = atof(first_pipe + 1);
            entry->score = 0;
            if (strstr(first_pipe, "Illegal")) return 0;
        }
    } else {
        const char *bracket_start = strstr(line, "[");
        const char *bracket_end = strstr(line, "]");
        if (bracket_start && bracket_end && bracket_start < bracket_end) {
            char res_str[10] = {0};
            strncpy(res_str, bracket_start + 1, bracket_end - bracket_start - 1);
            entry->result = atof(res_str);
        } else {
            return 0; // No valid result format found
        }
    }

    return 1;
}

TunerEntry *load_data(const char *filename, int *count) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    printf("Loading data from %s...\n", filename);

    int capacity = 100000;
    int loaded = 0, skipped = 0;
    TunerEntry *entries = malloc(sizeof(TunerEntry) * capacity);
    
    char buf[1024];
    while (fgets(buf, sizeof(buf), f)) {
        if (loaded >= capacity) {
            capacity = (int)(capacity * 1.5);
            TunerEntry *new_entries = realloc(entries, sizeof(TunerEntry) * capacity);
            if (!new_entries) {
                fprintf(stderr, "Memory allocation failed at %d entries!\n", loaded);
                free(entries);
                fclose(f);
                return NULL;
            }
            entries = new_entries;
        }

        if (parse_entry(buf, &entries[loaded])) {
            loaded++;
        } else {
            skipped++;
        }

        if (loaded > 0 && loaded % 500000 == 0) {
            printf("...loaded %d positions\n", loaded);
        }
    }
    fclose(f);

    *count = loaded;
    printf("Successfully loaded: %d positions (skipped %d invalid lines)\n", loaded, skipped);
    return entries;
}

double material[2][6] = {
    { 65, 333, 374, 492, 1028, 0},
    { 84, 285, 316, 534, 1040, 0}
};

const int mirror[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
     8,  9, 10, 11, 12, 13, 14, 15,
     0,  1,  2,  3,  4,  5,  6,  7
};

double psqt[2][6][64] = {
    {   // Opening
        {   // Pawn
            -74, -74, -74, -74, -74, -74, -74, -74,
            95, 92, 41, 119, 84, 56, 35, 28,
            -3, 44, 31, 62, 50, 51, 49, 24,
            -15, 20, 19, 37, 41, 23, 29, -10,
            -25, 9, 12, 33, 31, 23, 22, -15,
            -24, 10, 10, 13, 22, 15, 39, -4,
            -35, 6, 1, -15, 2, 30, 40, -17,
            -74, -74, -74, -74, -74, -74, -74, -74,
        },
        {   // Knight
            -180, -165, -163, -84, 37, -245, -204, -110,
            -22, 2, 49, 4, 15, 88, -12, -23,
            6, 70, 40, 104, 120, 99, 87, 34,
            16, 26, 50, 68, 53, 79, 33, 49,
            -7, 12, 38, 31, 43, 43, 18, 4,
            -22, 7, 14, 35, 33, 22, 25, -3,
            -26, -7, -3, 6, 8, 26, -1, -13,
            -38, -36, -34, -20, -15, -22, -21, -18,
        },
        {   // Bishop
            -45, -53, -82, -100, -52, -160, -102, -22,
            -17, 36, 17, -46, -36, -11, 13, -28,
            -20, 35, 42, 66, 59, 50, 25, 24,
            -4, 16, 38, 40, 37, 37, 19, 20,
            -7, 22, 15, 30, 35, 19, 19, -11,
            6, 10, 23, 16, 15, 25, 15, 7,
            0, 11, 15, 3, 13, 23, 30, 10,
            -25, -2, -15, -5, -38, -19, -19, -16,
        },
        {   // Rook
            67, 87, 131, 119, 127, 175, 84, 65,
            16, 17, 45, 79, 68, 61, 10, 51,
            7, 1, 10, 44, 58, 52, 15, 12,
            -42, -34, -4, 17, 15, 15, -25, -31,
            -48, -48, -25, -21, -32, -43, -58, -51,
            -59, -34, -38, -46, -39, -43, -36, -55,
            -69, -39, -22, -13, -33, -23, -49, -90,
            -47, -38, -27, -13, -14, -39, -63, -55,
        },
        {   // Queen
            -89, -19, 37, 76, 92, 179, 31, -21,
            -15, -1, 4, -10, -5, 35, 13, -2,
            -12, 9, -11, 14, 17, 39, 21, 24,
            -22, -17, -8, -3, 1, 4, 0, 7,
            -13, -24, -17, -8, 0, -8, -10, -6,
            -13, -5, -11, -5, -1, -6, 9, -9,
            -16, 5, 4, -4, 3, 11, -14, -28,
            -15, -20, -4, -3, -13, -19, -75, -55,
        },
        {   // King
            3, -102, 93, 144, 81, 6, 112, -63,
            159, 91, 82, 78, -8, -28, 68, -37,
            143, 63, -4, -36, -55, -36, -7, -43,
            -11, 22, -27, -24, -68, -58, -50, -129,
            -34, -22, -108, -109, -82, -109, -97, -49,
            -40, -8, -55, -86, -76, -54, -9, 10,
            53, 33, 21, -31, -22, 24, 64, 65,
            50, 57, 39, -11, 34, 18, 89, 85,
        }
    },
    {   // Endgame
        {   // Pawn
            -172, -172, -172, -172, -172, -172, -172, -172,
            159, 159, 149, 105, 124, 123, 151, 157,
            101, 85, 74, 47, 55, 64, 79, 88,
            69, 49, 30, 25, 18, 31, 30, 52,
            51, 40, 25, 16, 14, 25, 31, 37,
            40, 37, 21, 28, 24, 28, 22, 28,
            51, 40, 30, 35, 27, 26, 23, 30,
            -172, -172, -172, -172, -172, -172, -172, -172,
        },
        {   // Knight
            -45, 45, 35, 37, -27, 94, 57, -121,
            -45, 16, 4, 38, 9, -3, 11, -36,
            0, -8, 16, 16, 4, 4, -5, -9,
            0, 11, 28, 25, 32, 24, 21, 0,
            2, 0, 28, 36, 25, 28, 16, -2,
            -12, 9, 18, 14, 22, 11, 2, -23,
            -49, -14, -8, 9, 10, -10, -16, -39,
            -74, -30, -27, -13, -19, -7, -34, -86,
        },
        {   // Bishop
            -13, 11, 37, 36, 7, 61, 31, -35,
            -13, -12, 2, 14, 31, 19, 7, -15,
            -6, 14, 2, -16, -8, 12, -14, -7,
            0, 21, 5, 19, 13, 16, 16, -14,
            -4, 4, 20, 23, 20, 7, 6, -7,
            -5, -2, 9, 23, 25, 9, -16, -16,
            -24, -12, 11, -1, -1, -19, -15, -35,
            -15, -29, -30, -25, -21, -26, -25, -52,
        },
        {   // Rook
            -2, -10, -31, -27, -31, -48, -18, -9,
            13, 26, 17, 3, 7, 1, 22, 3,
            16, 25, 26, 9, -3, 5, 17, 14,
            24, 30, 25, 7, 7, 13, 18, 23,
            3, 17, 6, 13, 16, 25, 22, 6,
            -12, -15, -11, -1, -5, -4, -6, -25,
            -23, -15, -24, -20, -10, -11, -7, -32,
            -13, -6, -4, -7, -11, -4, 7, -19,
        },
        {   // Queen
            98, 29, 6, -48, -19, -94, 14, 31,
            -7, -4, 58, 87, 81, 26, 31, 48,
            6, -2, 71, 90, 101, 34, 101, -9,
            8, 27, 65, 72, 85, 88, 64, 22,
            -20, 27, 34, 46, 49, 55, 52, -22,
            -48, -44, 14, 17, -3, 22, -40, -75,
            -55, -86, -47, -26, -39, -59, -58, -102,
            -65, -96, -138, -103, -74, -112, -90, -75,
        },
        {   // King
            -74, 29, -30, -43, -26, 51, 40, 71,
            -60, 14, 4, -18, 16, 73, 34, 24,
            -22, 18, 16, 24, 37, 61, 60, 28,
            -7, 1, 23, 17, 36, 45, 44, 35,
            -2, 6, 31, 34, 36, 43, 36, -7,
            -13, -1, 13, 27, 29, 21, -1, -30,
            -45, -26, -17, 6, 0, -19, -43, -49,
            -117, -63, -45, -43, -69, -52, -69, -92
        }
    }
};

int evaluate(TunerEntry *e) {
    double mg = 0, eg = 0;

    for (int sq = 0; sq < 64; sq++) {
        int piece = e->pieces[sq];
        if (piece == NO_PIECE) continue;

        if (piece <= K) {
            mg += material[0][piece] + psqt[0][piece][sq];
            eg += material[1][piece] + psqt[1][piece][sq];
        } else {
            int pt = piece - 6;
            int mirrored = mirror[sq];
            mg -= material[0][pt] + psqt[0][pt][mirrored];
            eg -= material[1][pt] + psqt[1][pt][mirrored];
        }
    }

    double eval = (mg * e->phase + eg * (24.0 - e->phase)) / 24.0;
    int final_eval = (int)round(eval);
    return e->side == 0 ? final_eval : -final_eval;
}

double sigmoid(double sigmoid_k, int eval) {
    return 1.0 / (1.0 + pow(10.0, -sigmoid_k * eval / 400.0));
}

void* tuner_worker_thread(void* arg) {
    TunerWorker *w = (TunerWorker*)arg;
    int last_work_id = 0;
    while (1) {
        pthread_mutex_lock(&tuner_mutex);
        while (work_id == last_work_id) {
            pthread_cond_wait(&tuner_cond_workers, &tuner_mutex);
        }
        last_work_id = work_id;
        WorkType job = current_work;
        pthread_mutex_unlock(&tuner_mutex);

        if (job == WORK_QUIT) break;
        
        if (job == WORK_MSE) {
            w->total_error = 0.0;
            for (int i = w->start_index; i < w->end_index; i++) {
                int eval = evaluate(&w->data[i]);
                double wdl_target = (w->data[i].side == 0) ? w->data[i].result : (1.0 - w->data[i].result);
                double target = 0.75 * wdl_target + 0.25 * sigmoid(w->sigmoid_k, w->data[i].score);
                double predicted = sigmoid(w->sigmoid_k, eval);
                double error = target - predicted;
                w->total_error += error * error;
            }
        } else if (job == WORK_GRAD) {
            memset(w->grad, 0, sizeof(w->grad));
            memset(w->grad_mat, 0, sizeof(w->grad_mat));
            for (int i = w->start_index; i < w->end_index; i++) {
                TunerEntry *e = &w->data[i];
                int eval = evaluate(e);
                double wdl_target = (e->side == 0) ? e->result : (1.0 - e->result);
                double target = 0.75 * wdl_target + 0.25 * sigmoid(w->sigmoid_k, e->score);
                double sig = sigmoid(w->sigmoid_k, eval);
                double coeff = -2.0 * (target - sig) * sig * (1.0 - sig) * w->sigmoid_k * log(10.0) / 400.0 / w->count;
                if (e->side == 1) coeff = -coeff;

                double mg_weight = (double)e->phase / 24.0;
                double eg_weight = 1.0 - mg_weight;

                for (int sq = 0; sq < 64; sq++) {
                    int piece = e->pieces[sq];
                    if (piece == NO_PIECE) continue;

                    if (piece <= K) {
                        w->grad_mat[0][piece]              += coeff * mg_weight;
                        w->grad_mat[1][piece]              += coeff * eg_weight;
                        w->grad[0][piece][sq]              += coeff * mg_weight;
                        w->grad[1][piece][sq]              += coeff * eg_weight;
                    } else {
                        int pt = piece - 6;
                        int mirrored = mirror[sq];
                        w->grad_mat[0][pt]                 -= coeff * mg_weight;
                        w->grad_mat[1][pt]                 -= coeff * eg_weight;
                        w->grad[0][pt][mirrored]           -= coeff * mg_weight;
                        w->grad[1][pt][mirrored]           -= coeff * eg_weight;
                    }
                }
            }
        }

        pthread_mutex_lock(&tuner_mutex);
        active_workers--;
        if (active_workers == 0) {
            pthread_cond_signal(&tuner_cond_main);
        }
        pthread_mutex_unlock(&tuner_mutex);
    }
    return NULL;
}

double compute_mse(TunerEntry *data, int count, double sigmoid_k) {
    if (num_threads == 1) {
        double total_error = 0.0;
        for (int i = 0; i < count; i++) {
            int eval = evaluate(&data[i]);
            double wdl_target = (data[i].side == 0) ? data[i].result : (1.0 - data[i].result);
            double target = 0.75 * wdl_target + 0.25 * sigmoid(sigmoid_k, data[i].score);
            double predicted = sigmoid(sigmoid_k, eval);
            double error = target - predicted;
            total_error += error * error;
        }
        return total_error / count;
    }

    for (int i = 0; i < num_threads; i++) workers[i].sigmoid_k = sigmoid_k;
    run_job(WORK_MSE);

    double total_error = 0.0;
    for (int i = 0; i < num_threads; i++) {
        total_error += workers[i].total_error;
    }
    return total_error / count;
}

double find_optimal_K(TunerEntry *data, int count) {
    double lo = 0.1, hi = 10.0;
    int sample_size = count > 100000 ? 100000 : count;

    for (int iter = 0; iter < 20; iter++) {
        double mid1 = lo + (hi - lo) / 3.0;
        double mid2 = hi - (hi - lo) / 3.0;

        if (compute_mse(data, sample_size, mid1) < compute_mse(data, sample_size, mid2))
            hi = mid2;
        else
            lo = mid1;
    }
    return (lo + hi) / 2.0;
}
const char *piece_names[6] = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };

void center_psqt() {
    for (int ph = 0; ph < 2; ph++) {
        for (int pc = 0; pc < 6; pc++) {
            double sum = 0;
            for (int sq = 0; sq < 64; sq++) {
                sum += psqt[ph][pc][sq];
            }
            double average = sum / 64.0;

            for (int sq = 0; sq < 64; sq++) {
                psqt[ph][pc][sq] -= average;
            }

            if (pc != K) {
                material[ph][pc] += average;
            }
        }
    }
}

void print_psqt() {
    printf("\n// Tuned Material scores — paste into evaluation.c\n");
    printf("const int material_score[2][12] = {\n");

    for (int phase = 0; phase < 2; phase++) {
        printf("    { %d, %d, %d, %d, %d, 0, %d, %d, %d, %d, %d, 0 }%s\n", 
                 (int)round(material[phase][0]), (int)round(material[phase][1]), (int)round(material[phase][2]), 
                 (int)round(material[phase][3]), (int)round(material[phase][4]),
                 -(int)round(material[phase][0]), -(int)round(material[phase][1]), -(int)round(material[phase][2]), 
                 -(int)round(material[phase][3]), -(int)round(material[phase][4]),
                 phase == 0 ? "," : "");
    }
    printf("};\n\n");

    printf("// Tuned PSQT tables — paste into evaluation.c\n");
    printf("const int positional_score[2][6][64] = {\n");
    for (int phase = 0; phase < 2; phase++) {
        printf("    {   // %s\n", phase == 0 ? "Opening" : "Endgame");
        for (int piece = 0; piece < 6; piece++) {
            printf("        {   // %s\n", piece_names[piece]);
            for (int rank = 0; rank < 8; rank++) {
                printf("            ");
                for (int file = 0; file < 8; file++) {
                    int sq = rank * 8 + file;
                    printf("%d", (int)round(psqt[phase][piece][sq]));
                    if (sq < 63 || piece < 5 || phase < 1) {
                        printf(", ");
                    }
                }
                printf("\n");
            }
            printf("        }%s\n", piece < 5 ? "," : "");
        }
        printf("    }%s\n", phase == 0 ? "," : "");
    }
    printf("};\n");
}

void init_tuner_threads(TunerEntry *data, int count) {
    int chunk_size = count / num_threads;
    for (int i = 0; i < num_threads; i++) {
        workers[i].id = i;
        workers[i].data = data;
        workers[i].count = count;
        workers[i].start_index = i * chunk_size;
        workers[i].end_index = (i == num_threads - 1) ? count : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, tuner_worker_thread, &workers[i]);
    }
}

void stop_tuner_threads() {
    run_job(WORK_QUIT);
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

void tune(TunerEntry *data, int count, double sigmoid_k, int max_epochs) {
    double lr = 1.0;
    double beta1 = 0.9, beta2 = 0.999, epsilon = 1e-8;

    double grad[2][6][64] = {{{0}}};
    double m[2][6][64] = {{{0}}};
    double v[2][6][64] = {{{0}}};

    double grad_mat[2][6] = {{0}};
    double m_mat[2][6] = {{0}};
    double v_mat[2][6] = {{0}};

    for (int epoch = 1; epoch <= max_epochs; epoch++) {
        memset(grad, 0, sizeof(grad));
        memset(grad_mat, 0, sizeof(grad_mat));

        for (int i = 0; i < num_threads; i++) workers[i].sigmoid_k = sigmoid_k;
        run_job(WORK_GRAD);

        for (int i = 0; i < num_threads; i++) {
            for (int ph = 0; ph < 2; ph++) {
                for (int pc = 0; pc < 6; pc++) {
                    grad_mat[ph][pc] += workers[i].grad_mat[ph][pc];
                    for (int sq = 0; sq < 64; sq++) {
                        grad[ph][pc][sq] += workers[i].grad[ph][pc][sq];
                    }
                }
            }
        }

        for (int ph = 0; ph < 2; ph++) {
            for (int pc = 0; pc < 6; pc++) {
                // Update Material
                m_mat[ph][pc] = beta1 * m_mat[ph][pc] + (1.0 - beta1) * grad_mat[ph][pc];
                v_mat[ph][pc] = beta2 * v_mat[ph][pc] + (1.0 - beta2) * grad_mat[ph][pc] * grad_mat[ph][pc];
                double m_hat_mat = m_mat[ph][pc] / (1.0 - pow(beta1, epoch));
                double v_hat_mat = v_mat[ph][pc] / (1.0 - pow(beta2, epoch));
                material[ph][pc] -= (lr * m_hat_mat / (sqrt(v_hat_mat) + epsilon));

                // Update PSQT
                for (int sq = 0; sq < 64; sq++) {
                    m[ph][pc][sq] = beta1 * m[ph][pc][sq] + (1.0 - beta1) * grad[ph][pc][sq];
                    v[ph][pc][sq] = beta2 * v[ph][pc][sq] + (1.0 - beta2) * grad[ph][pc][sq] * grad[ph][pc][sq];

                    double m_hat = m[ph][pc][sq] / (1.0 - pow(beta1, epoch));
                    double v_hat = v[ph][pc][sq] / (1.0 - pow(beta2, epoch));

                    psqt[ph][pc][sq] -= (lr * m_hat / (sqrt(v_hat) + epsilon));
                }
            }
        }
        if (epoch % 10 == 0) {
            double mse = compute_mse(data, count, sigmoid_k);
            static double last_time = 0;
            double current_time = get_time_ms();
            if (last_time == 0) last_time = current_time;
            double elapsed_sec = (current_time - last_time) / 1000.0;
            double eps = (elapsed_sec > 0) ? (10.0 / elapsed_sec) : 0; // Adjusted for 10 epochs
            printf("Epoch %d: MSE = %.10f | Speed: %.1f epochs/s\n", epoch, mse, eps);
            last_time = current_time;
        }
    }

    center_psqt();

    double final_mse = compute_mse(data, count, sigmoid_k);
    printf("Tuning complete. Final MSE = %.10f\n", final_mse);
}

#ifndef _WIN32
#include <sys/stat.h>
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./tuner <data_path> [num_threads]\n");
        return 1;
    }

    if (argc >= 3) {
        num_threads = atoi(argv[2]);
        if (num_threads < 1) num_threads = 1;
        if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    }

    int count = 0;
    TunerEntry *data = NULL;
    const char *path = argv[1];

#ifndef _WIN32
    struct stat st;
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
        // It's a regular file
        data = load_data(path, &count);
    } else {
        // Try as directory
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(path)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (strstr(ent->d_name, ".txt")) {
                    char filepath[1024];
                    snprintf(filepath, sizeof(filepath), "%s/%s", path, ent->d_name);
                    int file_count = 0;
                    TunerEntry *file_data = load_data(filepath, &file_count);
                    if (file_data && file_count > 0) {
                        data = realloc(data, (count + file_count) * sizeof(TunerEntry));
                        memcpy(data + count, file_data, file_count * sizeof(TunerEntry));
                        free(file_data);
                        count += file_count;
                    }
                }
            }
            closedir(dir);
        } else {
            printf("Failed to open: %s\n", path);
            return 1;
        }
    }
#else
    // Windows logic (simplified or keep as is if it works for user)
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".txt")) {
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", path, ent->d_name);
                int file_count = 0;
                TunerEntry *file_data = load_data(filepath, &file_count);
                if (file_data && file_count > 0) {
                    data = realloc(data, (count + file_count) * sizeof(TunerEntry));
                    memcpy(data + count, file_data, file_count * sizeof(TunerEntry));
                    free(file_data);
                    count += file_count;
                }
            }
        }
        closedir(dir);
    } else {
        // If opendir fails on Windows, try as file
        data = load_data(path, &count);
        if (!data) {
            printf("Failed to open: %s\n", path);
            return 1;
        }
    }
#endif

    if (!data || count == 0) {
        printf("No data loaded!\n");
        return 1;
    }

    int wins = 0, draws = 0, losses = 0;
    for (int i = 0; i < count; i++) {
        if (data[i].result == 1.0) wins++;
        else if (data[i].result == 0.5) draws++;
        else losses++;
    }
    printf("Stats: %d Win, %d Draw, %d Loss (Total: %d)\n", wins, draws, losses, count);

    int phase_sum = 0;
    for (int i = 0; i < count; i++) phase_sum += data[i].phase;
    printf("Average Phase: %.1f / 24\n\n", (double)phase_sum / count);

    printf("\nInitializing %d tuner threads...\n", num_threads);
    init_tuner_threads(data, count);

    printf("Calculating optimal K based on %.1f Avg Phase data...\n", (double)phase_sum / count);
    double optimal_k = find_optimal_K(data, count);
    printf("Optimal K for this dataset is %.5f\n", optimal_k);

    printf("Tuning with Dynamic K = %.5f...\n", optimal_k);
    tune(data, count, optimal_k, 1000);

    print_psqt();

    stop_tuner_threads();
    free(data);
    return 0;
}

