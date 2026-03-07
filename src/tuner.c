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

int material[2][6] = {
    { 300, 991, 1122, 1214, 2687, 0},
    { 296, 828, 918, 1714, 2835, 0}
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

int psqt[2][6][64] = {
    {   // Opening
        {   // Pawn
            -65, -65, -65, -65, -65, -65, -65, -65,
            119, 133, 78, 131, 70, 110, 110, 240,
            -59, -37, 22, 66, 84, 49, -60, 21,
            -66, -2, 20, 78, 125, -10, 62, -165,
            -142, -27, -29, 103, 55, -22, 0, -180,
            -91, 42, -4, 12, 72, -71, 122, -93,
            -52, 94, 12, -37, 6, 82, 166, -89,
            -65, -65, -65, -65, -65, -65, -65, -65,
        },
        {   // Knight
            -379, -191, -387, -212, 97, -502, -148, -177,
            -179, -69, 344, -364, -138, 271, 109, -67,
            -66, 154, -1, 181, 196, -73, 196, 64,
            109, 145, 134, 254, 169, 207, 105, 129,
            -8, -59, 147, 88, 125, 107, 74, 29,
            71, 30, 143, -52, 27, 137, 51, -30,
            -27, -79, 76, 83, 59, 12, -116, -10,
            -238, -25, -45, -146, -12, -79, -32, -176,
        },
        {   // Bishop
            -158, -298, -391, -482, -133, -654, -473, 1,
            -65, 92, 72, -568, -331, 36, 102, -56,
            -224, 105, -206, 88, -9, -288, -42, 23,
            -26, 129, 82, 104, 157, 135, 165, 25,
            12, -4, 134, 94, 108, 127, 41, 75,
            184, 170, 141, 158, 155, 160, 200, 110,
            -10, 215, 142, 146, 136, 273, 227, 9,
            -120, 38, 69, -54, -70, 35, 121, 128,
        },
        {   // Rook
            20, 123, -49, -662, 151, -156, 9, 179,
            152, 131, 157, 183, 218, 229, 84, 213,
            162, -22, -50, 252, 56, 14, -2, 221,
            115, 37, 160, 61, 9, 89, -1, 118,
            -5, -212, -97, -203, -162, -273, -135, 72,
            -10, -185, -193, -329, -123, -154, -187, 19,
            -80, -55, -202, -39, -83, -39, -124, -101,
            6, 30, 114, 177, 203, 151, 9, 44,
        },
        {   // Queen
            -44, 93, 144, -408, -420, 70, 34, -12,
            -19, 56, 49, -225, 95, 127, 184, 55,
            31, -84, 60, 86, 85, 159, 19, 79,
            14, 49, 71, 39, 110, -35, 1, -83,
            84, -14, 62, -23, 22, 49, -141, 17,
            89, 114, 17, 21, -2, 40, 33, -38,
            93, 64, 136, 77, 105, 66, 111, -189,
            -141, -30, 29, 90, 58, 10, -549, -577,
        },
        {   // King
            5, 93, 86, 55, 14, 36, 72, 83,
            89, 69, 36, 63, 16, 66, 32, 41,
            12, 94, 72, 54, 119, 243, 264, 4,
            -109, 35, 58, 196, 125, 162, 103, 34,
            -244, -50, -183, -204, -130, -206, -183, -204,
            -238, -12, -248, -358, -317, -244, -16, -93,
            -38, -35, -122, -139, -39, 106, 64, 50,
            -118, 44, 124, -62, 186, 67, 323, 197,
        }
    },
    {   // Endgame
        {   // Pawn
            -207, -207, -207, -207, -207, -207, -207, -207,
            464, 489, 514, 410, 271, 580, 436, 405,
            225, 174, 109, 55, 39, 113, 44, 93,
            -18, 21, -25, -75, -121, -33, -32, 133,
            20, -21, -16, -67, -81, 52, -116, 79,
            -106, -27, -71, 20, -25, 27, -67, -87,
            -179, -87, -55, 43, -25, -61, -44, -113,
            -207, -207, -207, -207, -207, -207, -207, -207,
        },
        {   // Knight
            -476, -62, 39, 25, 66, 15, -217, -293,
            -103, 21, -137, 155, -25, 78, -51, -175,
            74, -21, 88, 170, 216, 120, 10, -70,
            10, 22, 186, 144, 154, 175, 135, 96,
            -90, 46, 153, 95, 114, 216, 107, 34,
            -73, 67, 29, 142, 168, 27, 17, -162,
            -86, -123, -3, 47, 140, -134, -104, -126,
            -183, -48, 175, -200, -46, -197, -95, -218,
        },
        {   // Bishop
            -9, -88, 141, 50, 67, -28, -247, 107,
            -147, -69, 70, -64, 113, -54, 28, -105,
            213, 81, 155, 60, 55, 204, 88, 213,
            22, 104, 49, 50, 85, 84, 111, 144,
            -20, 83, 106, 106, -5, 134, 33, 73,
            -103, 17, 50, 42, 76, 12, -35, 97,
            -81, -152, -181, -90, -49, -109, -138, -64,
            -295, -129, 14, -108, -231, -154, -247, -150,
        },
        {   // Rook
            97, 40, 30, 245, -37, 35, 43, 72,
            62, 118, 140, 106, 131, 124, 86, 59,
            57, 109, 97, 106, 67, 32, 85, 10,
            85, 155, 89, 108, 51, 64, 49, 38,
            36, 28, 207, 82, 7, 123, 142, 52,
            -124, -32, -28, 28, -277, -213, -106, -37,
            -202, -135, -90, -207, -385, -298, -402, -7,
            -124, -55, -102, -79, -78, -61, -9, -199,
        },
        {   // Queen
            136, 20, 100, 71, 12, 5, 54, 195,
            15, -2, 102, 307, 209, 116, 86, 105,
            -23, 33, 114, 223, 181, 200, 99, 114,
            77, 69, 162, 267, 185, 312, 411, 391,
            74, 133, 140, 287, 252, 237, 414, 304,
            -183, -103, 213, 88, 219, 126, 172, 65,
            -309, -112, -139, -9, -106, -210, -260, -224,
            -601, -996, -1019, -678, -200, -514, -1096, -272,
        },
        {   // King
            -119, -282, -174, -296, -177, -176, -187, -31,
            -304, -50, -53, -42, -31, 57, 17, -3,
            -183, 67, 182, 180, 198, 70, 62, -29,
            -171, -31, 159, 114, 126, 121, 153, -36,
            -254, 23, 160, 147, 149, 139, 178, 86,
            -152, 52, 134, 286, 227, 243, 91, -27,
            -179, -19, 95, 114, 184, 20, 9, -24,
            -211, -49, -80, -56, 13, -113, -106, -161
        }
    }
};

int evaluate(TunerEntry *e) {
    int mg = 0, eg = 0;

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

    int eval = (mg * e->phase + eg * (24 - e->phase)) / 24;
    return e->side == 0 ? eval : -eval;
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
            int sum = 0;
            for (int sq = 0; sq < 64; sq++) {
                sum += psqt[ph][pc][sq];
            }
            int average = sum / 64;

            // Subtract average from all PSQT squares
            for (int sq = 0; sq < 64; sq++) {
                psqt[ph][pc][sq] -= average;
            }

            // Add average back to material (except King)
            if (pc != K) {
                material[ph][pc] += average;
            }
        }
    }
}

void print_psqt() {
    printf("\n>>> Saving results to tuned_params.txt...\n");
    FILE *f_params = fopen("tuned_params.txt", "w");
    if (!f_params) {
        printf("Warning: Could not open tuned_params.txt for writing.\n");
    }

    // Capture everything in a temporary buffer or just do file writing first
    // To be simple and robust, let's just do two passes or write to file first.
    
    #define WRITE_BOTH(fmt, ...) { \
        if (f_params) fprintf(f_params, fmt, ##__VA_ARGS__); \
    }

    WRITE_BOTH("\n// Tuned Material scores — paste into evaluation.c\n");
    WRITE_BOTH("const int material_score[2][12] = {\n");

    for (int phase = 0; phase < 2; phase++) {
        WRITE_BOTH("    { %d, %d, %d, %d, %d, 0, %d, %d, %d, %d, %d, 0 }%s\n", 
                 material[phase][0], material[phase][1], material[phase][2], material[phase][3], material[phase][4],
                 -material[phase][0], -material[phase][1], -material[phase][2], -material[phase][3], -material[phase][4],
                 phase == 0 ? "," : "");
    }
    WRITE_BOTH("};\n");

    WRITE_BOTH("\n// Tuned PSQT tables — paste into evaluation.c\n");
    WRITE_BOTH("const int positional_score[2][6][64] = {\n");
    for (int phase = 0; phase < 2; phase++) {
        WRITE_BOTH("    {   // %s\n", phase == 0 ? "Opening" : "Endgame");
        for (int piece = 0; piece < 6; piece++) {
            WRITE_BOTH("        {   // %s\n", piece_names[piece]);
            for (int rank = 0; rank < 8; rank++) {
                WRITE_BOTH("            ");
                for (int file = 0; file < 8; file++) {
                    int sq = rank * 8 + file;
                    WRITE_BOTH("%d", psqt[phase][piece][sq]);
                    if (sq < 63 || piece < 5 || phase < 1) {
                        WRITE_BOTH(", ");
                    }
                }
                WRITE_BOTH("\n");
            }
            WRITE_BOTH("        }%s\n", piece < 5 ? "," : "");
        }
        WRITE_BOTH("    }%s\n", phase == 0 ? "," : "");
    }
    WRITE_BOTH("};\n");

    if (f_params) {
        fclose(f_params);
        printf(">>> Results saved successfully.\n\n");
    }

    // Now print to console for the user to see immediately
    printf("// Tuned Material scores — paste into evaluation.c\n");
    printf("const int material_score[2][12] = {\n");
    for (int phase = 0; phase < 2; phase++) {
        printf("    { %d, %d, %d, %d, %d, 0, %d, %d, %d, %d, %d, 0 }%s\n", 
                 material[phase][0], material[phase][1], material[phase][2], material[phase][3], material[phase][4],
                 -material[phase][0], -material[phase][1], -material[phase][2], -material[phase][3], -material[phase][4],
                 phase == 0 ? "," : "");
    }
    printf("};\n\n");
    printf("// Check tuned_params.txt for PSQT tables to save console space.\n");
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
    double lr = 2.0;
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
                material[ph][pc] -= (int)(lr * m_hat_mat / (sqrt(v_hat_mat) + epsilon));

                // Update PSQT
                for (int sq = 0; sq < 64; sq++) {
                    m[ph][pc][sq] = beta1 * m[ph][pc][sq] + (1.0 - beta1) * grad[ph][pc][sq];
                    v[ph][pc][sq] = beta2 * v[ph][pc][sq] + (1.0 - beta2) * grad[ph][pc][sq] * grad[ph][pc][sq];

                    double m_hat = m[ph][pc][sq] / (1.0 - pow(beta1, epoch));
                    double v_hat = v[ph][pc][sq] / (1.0 - pow(beta2, epoch));

                    psqt[ph][pc][sq] -= (int)(lr * m_hat / (sqrt(v_hat) + epsilon));
                }
            }
        }
        if (epoch % 100 == 0) {
            double mse = compute_mse(data, count, sigmoid_k);
            static double last_time = 0;
            double current_time = get_time_ms();
            if (last_time == 0) last_time = current_time;
            double elapsed_sec = (current_time - last_time) / 1000.0;
            double eps = (elapsed_sec > 0) ? (100.0 / elapsed_sec) : 0;
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

    printf("Finding optimal K...\n");
    double sigmoid_k = find_optimal_K(data, count);
    double mse = compute_mse(data, count, sigmoid_k);
    printf("Optimal K = %.6f\n", sigmoid_k);
    printf("Initial MSE = %.10f\n\n", mse);

    printf("Starting tuning...\n");
    tune(data, count, sigmoid_k, 1000);

    print_psqt();

    stop_tuner_threads();
    free(data);
    return 0;
}

