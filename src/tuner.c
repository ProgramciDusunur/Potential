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
    { 58, 321, 352, 462, 934, 0},
    { 73, 250, 277, 494, 1002, 0}
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
            117, 101, 40, 126, 102, 35, 25, 6,
            -3, 40, 27, 63, 58, 56, 44, 22,
            -12, 21, 15, 34, 40, 20, 28, -10,
            -23, 5, 10, 37, 27, 22, 20, -12,
            -23, 7, 8, 13, 20, 7, 33, 5,
            -32, 4, -1, -14, 4, 28, 37, -9,
            -73, -73, -73, -73, -73, -73, -73, -73,
        },
        {   // Knight
            -183, -181, -173, -57, 18, -250, -216, -59,
            -39, 5, 52, 24, 21, 99, 11, -9,
            0, 52, 42, 90, 108, 117, 93, 43,
            4, 20, 38, 62, 47, 77, 30, 46,
            -9, 17, 29, 25, 37, 43, 20, 5,
            -27, 7, 11, 36, 30, 20, 19, 2,
            -23, -12, 1, 5, 10, 23, 19, -8,
            -73, -34, -27, -26, -15, -11, -17, -9,
        },
        {   // Bishop
            -39, -62, -52, -112, -165, -210, -99, -18,
            -1, 31, 29, -82, -35, 12, 19, -36,
            -6, 44, 26, 54, 54, -2, 23, 24,
            6, 27, 39, 52, 35, 38, 17, 20,
            5, 25, 23, 27, 33, 20, 14, 0,
            5, 19, 27, 18, 17, 25, 17, 14,
            0, 13, 24, 5, 17, 33, 30, 18,
            -5, 8, -9, 3, -29, -11, -8, -10,
        },
        {   // Rook
            68, 112, 140, 117, 109, 50, 125, 43,
            28, 11, 32, 87, 53, 36, 0, 25,
            12, -9, 14, 48, 52, 45, 23, 2,
            -40, -27, -8, 19, 38, 26, -27, -23,
            -39, -48, -36, -28, -23, -39, -52, -40,
            -47, -28, -42, -46, -26, -36, -32, -51,
            -62, -42, -29, -11, -32, -16, -41, -87,
            -38, -29, -21, -12, -6, -37, -63, -42,
        },
        {   // Queen
            -50, 27, 47, -330, 111, 154, 11, -2,
            2, 15, 12, -18, 1, 51, 14, -13,
            1, 3, -1, 8, 37, 53, 28, 31,
            -14, -6, 5, -1, 0, 9, 5, 5,
            -6, -17, -14, -11, -6, -3, -7, 2,
            -6, 0, -11, -3, 9, -4, 11, 6,
            -9, 4, 5, 1, 5, 12, -18, -17,
            3, -5, 2, 4, -3, -7, -61, -56,
        },
        {   // King
            -71, -109, 221, 158, 100, 11, 100, 22,
            39, 119, 154, 47, 76, -105, 36, -12,
            168, 33, -50, -91, -76, -32, -28, -98,
            11, 32, 21, 15, -43, -74, -129, -177,
            -23, 12, -84, -88, -78, -140, -117, -32,
            -31, 2, -30, -71, -75, -50, -23, 15,
            32, 29, 27, -23, -20, 26, 69, 68,
            19, 54, 37, -9, 32, 21, 95, 89,
        }
    },
    {   // Endgame
        {   // Pawn
            -169, -169, -169, -169, -169, -169, -169, -169,
            131, 153, 136, 107, 121, 103, 148, 144,
            97, 81, 75, 49, 61, 64, 79, 88,
            71, 47, 33, 21, 14, 29, 25, 56,
            54, 45, 23, 12, 15, 30, 28, 39,
            46, 43, 27, 27, 24, 33, 20, 29,
            57, 48, 31, 31, 22, 31, 23, 29,
            -169, -169, -169, -169, -169, -169, -169, -169,
        },
        {   // Knight
            -36, 36, 36, 37, -21, 67, 33, -160,
            -37, 22, -1, 35, 2, -3, -13, -52,
            -12, 4, 13, 25, 9, 0, -16, -6,
            21, 18, 38, 19, 27, 19, 21, -6,
            28, -11, 33, 39, 25, 28, 21, -13,
            -2, 13, 23, 11, 26, 17, 10, -19,
            -53, 1, -15, 12, 16, -9, -31, -27,
            -45, -20, -38, -5, -14, -3, -24, -94,
        },
        {   // Bishop
            12, 8, 24, 46, 22, 52, 31, -77,
            -20, 5, 0, 20, 6, 16, -11, -10,
            -1, 3, 14, -19, -10, 0, 4, -11,
            -3, 21, 1, 10, 7, 14, 23, -14,
            -16, 4, 16, 29, 21, 3, 12, -12,
            -2, 0, 12, 27, 24, 16, -5, -17,
            -16, -12, 1, 0, 2, -25, -14, -30,
            -9, -31, -17, -27, -10, -19, -21, -46,
        },
        {   // Rook
            -9, -25, -40, -30, -29, -23, -34, -13,
            5, 31, 22, 2, 20, 20, 28, 5,
            8, 23, 22, 0, -7, 8, 9, 11,
            32, 31, 22, 5, -7, 3, 22, 22,
            9, 19, 10, 15, 13, 27, 21, -10,
            -6, -16, 1, 1, -11, -6, -4, -11,
            -21, -3, -16, -20, -7, -5, -10, -10,
            -16, -12, -6, -6, -15, -7, 5, -37,
        },
        {   // Queen
            74, -30, -14, 314, -22, -45, 22, 23,
            -25, -10, 47, 92, 65, 59, 19, 42,
            -7, 15, 69, 87, 87, 0, 76, -16,
            -10, 10, 53, 79, 110, 59, 59, 26,
            -29, 0, 44, 50, 60, 50, 46, -26,
            -66, -33, 23, 17, -35, 8, -35, -96,
            -54, -79, -27, -30, -29, -59, -47, -102,
            -46, -84, -137, -113, -85, -106, -219, -73,
        },
        {   // King
            -55, 51, 5, 7, -27, 78, -2, 24,
            -50, 23, 2, -15, 20, 80, 23, 16,
            -33, 11, 0, 10, 37, 73, 103, 60,
            -12, -17, -2, -6, 29, 57, 78, 61,
            -8, -7, 14, 21, 35, 48, 43, -12,
            -19, -4, 2, 15, 24, 21, 3, -35,
            -28, -30, -20, -3, -7, -32, -54, -51,
            -105, -47, -46, -47, -74, -55, -82, -90
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

    printf("Tuning with Fixed K = 1.13 for stability...\n");
    tune(data, count, 1.13, 1000);

    print_psqt();

    stop_tuner_threads();
    free(data);
    return 0;
}

