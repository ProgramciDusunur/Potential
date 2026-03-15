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

    const char *pipe = strstr(line, "|");
    if (pipe) {
        entry->result = atof(pipe + 1);
        if (strstr(pipe, "Illegal")) return 0;
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
    { 71, 353, 399, 527, 1107, 0},
    { 106, 371, 405, 657, 1268, 0}
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
            -73, -73, -73, -73, -73, -73, -73, -73,
            82, 51, 70, 108, 87, 13, -28, 33,
            6, 27, 40, 65, 60, 36, 44, 28,
            -6, 25, 24, 44, 53, 33, 31, -8,
            -18, 12, 12, 38, 31, 20, 23, -8,
            -15, 10, 12, 16, 22, 18, 46, -2,
            -31, 9, 0, -13, 0, 33, 44, -14,
            -73, -73, -73, -73, -73, -73, -73, -73,
        },
        {   // Knight
            -210, -130, -139, -77, 31, -239, -141, -155,
            -28, 9, 67, 8, 36, 66, -4, -35,
            -27, 84, 92, 103, 134, 100, 93, 0,
            10, 27, 47, 79, 63, 72, 31, 58,
            -9, -6, 35, 33, 39, 40, 20, -2,
            -16, 0, 16, 30, 42, 19, 19, -15,
            -36, -30, 10, 0, 6, 16, 2, -25,
            -34, -37, -28, -11, -7, -22, -27, -44,
        },
        {   // Bishop
            -78, -88, -28, -77, -12, -165, -126, -11,
            -34, 19, 19, -43, 9, -13, 35, -34,
            -15, 38, 31, 77, 49, 50, 24, 27,
            -5, 14, 52, 35, 46, 31, 17, 11,
            -11, 14, 12, 35, 34, 18, 21, -18,
            1, 8, 14, 20, 13, 24, 9, 7,
            2, 10, 20, -6, 14, 21, 29, 19,
            -10, -37, -22, -24, -33, -20, -21, 3,
        },
        {   // Rook
            52, 83, 61, 130, 151, 114, 47, 86,
            28, 21, 60, 103, 121, 112, 23, 13,
            -16, 23, 25, 56, 81, 64, 53, 16,
            -34, -45, -16, 0, 25, 3, -2, -27,
            -51, -61, -44, -27, -28, -33, -24, -47,
            -79, -44, -49, -53, -57, -55, -25, -66,
            -73, -35, -26, -31, -32, -27, -38, -94,
            -53, -43, -36, -16, -12, -33, -59, -61,
        },
        {   // Queen
            -80, -47, 52, 71, 28, 63, 41, 1,
            -32, -21, -11, 6, 7, 16, 14, 4,
            -29, 8, -8, 13, 55, 64, 25, 22,
            -11, -17, -8, 2, 12, 14, -9, 8,
            -2, -29, -5, -1, 0, 3, -9, -16,
            -22, 4, 6, -8, -4, -4, 10, -9,
            -30, -2, 11, 1, 4, 22, 6, -42,
            -26, -16, -7, 1, -12, -24, -6, -45,
        },
        {   // King
            100, 159, 152, 41, 273, 83, 28, -274,
            210, 33, 184, 286, 279, 96, -89, 19,
            15, 11, -33, 24, 23, 1, 51, -18,
            -114, -82, -166, -61, -49, -18, -55, -75,
            -123, -81, -128, -131, -129, -95, -98, -106,
            -3, -35, -109, -112, -98, -67, -38, -24,
            32, 9, 8, -44, -37, 17, 39, 49,
            54, 48, 30, -25, 24, 0, 73, 66,
        }
    },
    {   // Endgame
        {   // Pawn
            -176, -176, -176, -176, -176, -176, -176, -176,
            182, 202, 163, 107, 77, 141, 190, 183,
            104, 99, 73, 68, 57, 48, 85, 79,
            65, 50, 34, 18, 12, 18, 36, 51,
            45, 35, 26, 18, 14, 19, 38, 37,
            31, 37, 22, 24, 17, 22, 22, 26,
            42, 39, 33, 20, 35, 22, 23, 27,
            -176, -176, -176, -176, -176, -176, -176, -176,
        },
        {   // Knight
            -33, 6, 21, 28, -7, 133, 41, -153,
            -21, -8, -1, 62, 34, 11, 8, 3,
            -8, -3, 21, 39, 9, 10, 14, 10,
            -13, 24, 37, 34, 37, 44, 36, -24,
            -3, 21, 30, 41, 42, 33, 6, 18,
            -36, -3, 12, 19, 14, 14, 3, -18,
            -71, -12, -7, 14, 3, -9, -6, -47,
            -112, -44, -31, -41, -26, -27, -42, -127,
        },
        {   // Bishop
            35, 30, 7, 40, 1, 42, 19, -40,
            14, -17, 2, 18, 8, -2, 16, -20,
            -5, 8, 23, -14, 0, 24, 24, -10,
            1, 23, 4, 19, 25, 22, 12, -14,
            -1, -1, 15, 30, 28, 22, -4, 3,
            -13, 3, 18, 19, 22, 2, -13, -19,
            -30, -11, -10, 11, -5, -8, -21, -40,
            -55, -42, -32, -21, -17, -37, -30, -58,
        },
        {   // Rook
            10, 4, 12, -16, -21, -14, 5, -5,
            18, 22, 20, 1, -10, -13, 11, 18,
            30, 15, 22, 9, -5, -3, 1, 5,
            21, 34, 27, 25, 13, 14, 11, 21,
            -1, 20, 17, 18, 6, 18, 0, -9,
            -7, -14, -8, -6, 6, 7, -27, -20,
            -26, -36, -22, -16, -10, -21, -24, -26,
            -12, -14, 5, -10, -19, -17, -1, -32,
        },
        {   // Queen
            85, 74, -22, -32, 56, 30, 8, 2,
            -7, 26, 75, 59, 84, 77, 38, 47,
            23, 1, 47, 74, 67, 55, 62, 9,
            -5, -1, 64, 76, 81, 81, 76, 29,
            -76, 36, 10, 73, 50, 42, 57, 1,
            -56, -59, -45, 14, 7, 26, -36, -7,
            -53, -80, -102, -39, -45, -93, -104, -107,
            -47, -201, -103, -114, -69, -66, -97, -55,
        },
        {   // King
            -263, -94, -80, -46, -66, -18, 6, 9,
            -51, 2, -32, -48, -13, 16, 62, 13,
            1, 49, 59, 35, 43, 71, 39, 25,
            26, 54, 73, 49, 52, 44, 60, 30,
            20, 34, 55, 57, 52, 45, 46, 15,
            -28, 20, 39, 48, 44, 31, 18, -2,
            -36, -18, -6, 15, 21, -5, -20, -42,
            -122, -59, -39, -39, -54, -48, -59, -88
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
                double target = (w->data[i].side == 0) ? w->data[i].result : (1.0 - w->data[i].result);
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
                double target = (e->side == 0) ? e->result : (1.0 - e->result);
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
            double target = (data[i].side == 0) ? data[i].result : (1.0 - data[i].result);
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
    double min_lr = 0.01;
    double decay_rate = 0.5;
    int drop_every = 200; // Halve the learning rate every 200 epochs

    double beta1 = 0.9, beta2 = 0.999, epsilon = 1e-8;

    double grad[2][6][64] = {{{0}}};
    double m[2][6][64] = {{{0}}};
    double v[2][6][64] = {{{0}}};

    double grad_mat[2][6] = {{0}};
    double m_mat[2][6] = {{0}};
    double v_mat[2][6] = {{0}};

    for (int epoch = 1; epoch <= max_epochs; epoch++) {
        // Step decay for learning rate
        double current_lr = lr * pow(decay_rate, floor((epoch - 1) / (double)drop_every));
        if (current_lr < min_lr) current_lr = min_lr;

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
                material[ph][pc] -= (current_lr * m_hat_mat / (sqrt(v_hat_mat) + epsilon));

                // Update PSQT
                for (int sq = 0; sq < 64; sq++) {
                    m[ph][pc][sq] = beta1 * m[ph][pc][sq] + (1.0 - beta1) * grad[ph][pc][sq];
                    v[ph][pc][sq] = beta2 * v[ph][pc][sq] + (1.0 - beta2) * grad[ph][pc][sq] * grad[ph][pc][sq];

                    double m_hat = m[ph][pc][sq] / (1.0 - pow(beta1, epoch));
                    double v_hat = v[ph][pc][sq] / (1.0 - pow(beta2, epoch));

                    psqt[ph][pc][sq] -= (current_lr * m_hat / (sqrt(v_hat) + epsilon));
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

    printf("Tuning with Fixed K = 1.13 for stability...\n");
    tune(data, count, 1.13, 1000);

    print_psqt();

    stop_tuner_threads();
    free(data);
    return 0;
}

