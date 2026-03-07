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

#include <windows.h>

double get_time_ms() {
    return (double)GetTickCount64();
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

    int lines = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), f)) lines++;
    rewind(f);

    TunerEntry *entries = malloc(sizeof(TunerEntry) * lines);
    if (!entries) {
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(f);
        return NULL;
    }

    int loaded = 0, skipped = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (parse_entry(buf, &entries[loaded])) {
            loaded++;
        } else {
            skipped++;
        }
    }
    fclose(f);

    *count = loaded;
    printf("Loaded: %d positions, Skipped: %d lines\n", loaded, skipped);
    return entries;
}

int material[2][6] = {
    { 10, 5, 4, 27, 27, 0},
    { -5, -28, -12, -19, -79, 0}
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
            19, 19, 19, 19, 19, 19, 19, 19,
            65, 153, 56, 54, -6, 109, 48, -45,
            -38, -40, -58, -7, -14, -24, 22, -72,
            -21, -13, -11, -13, -2, -5, 3, -8,
            -14, -15, -24, -19, -18, -25, -6, -23,
            -17, -17, -20, -26, -27, -22, -17, -20,
            -20, -20, -17, -13, 9, -19, -13, -20,
            19, 19, 19, 19, 19, 19, 19, 19,
        },
        {   // Knight
            -148, -32, 34, -50, 118, -40, 42, -50,
            -16, 16, 86, 93, 18, -17, 20, 40,
            10, 70, -14, 10, -3, 50, 39, 101,
            26, -16, 9, -5, -20, 15, -17, 31,
            -34, 61, -24, -19, -14, -27, 11, -12,
            -18, -9, -14, -15, -12, -17, -17, -3,
            -8, -25, -41, -19, -24, -17, 10, -27,
            -48, -7, -28, -10, -29, -3, -15, 14,
        },
        {   // Bishop
            -159, 61, 14, 20, 32, 15, 64, -129,
            31, -4, 39, 44, 35, 38, 16, 10,
            -23, 94, -42, 16, 61, -22, -37, -31,
            -36, -15, 32, 8, -37, 32, -20, 23,
            8, 9, -9, -33, -4, -24, -8, -34,
            -31, 19, -9, -18, -19, -7, -16, -6,
            3, -22, 33, -17, -12, 56, -4, 11,
            19, -4, -21, 1, -2, 10, 36, 11,
        },
        {   // Rook
            43, 53, -68, 62, 36, 20, 42, 54,
            -10, 43, 44, 46, -12, 78, 37, 55,
            6, 27, 20, 47, 16, 25, 72, 27,
            -13, 0, 18, 37, 35, 46, -7, -9,
            -25, -42, -1, 35, 14, -34, 19, -70,
            -42, -40, -16, -21, -25, -33, -14, -54,
            -35, -64, -10, -27, -31, -33, -55, -18,
            -21, -29, -18, -34, -39, -38, -33, -33,
        },
        {   // Queen
            -84, 192, 222, 210, 160, 133, 84, -89,
            81, 97, 42, -9, 97, 50, -44, -89,
            82, 93, 47, 73, 80, -65, -10, 79,
            -38, -26, 22, -24, -22, 23, -26, -30,
            -18, -49, -21, -39, -21, -34, -25, -43,
            -82, -13, -38, -34, -33, -30, -59, -104,
            -32, -42, -24, -33, -35, -18, -85, -77,
            43, -93, -43, 21, -32, -76, -72, -59,
        },
        {   // King
            -51, 37, 30, -1, -42, -20, 16, 27,
            43, 13, -6, 7, 6, 10, -24, -15,
            5, 38, 16, -2, -6, 20, 36, -8,
            -3, -6, 2, -13, -16, -11, 0, -22,
            -35, 13, -13, -25, -32, -30, -19, -37,
            0, 51, -19, -32, -26, 3, 16, -13,
            15, -1, 1, -8, -4, 5, 28, 18,
            -1, 46, -2, 2, 0, 13, 1, 16,
        }
    },
    {   // Endgame
        {   // Pawn
            -19, -19, -19, -19, -19, -19, -19, -19,
            14, 48, 62, 50, 80, -13, 0, 38,
            -1, 35, 43, 23, 6, 20, 7, 39,
            4, 13, -1, -4, -12, 0, -4, -2,
            7, 1, 10, 1, 4, 10, -6, 13,
            5, 9, 4, 20, 19, 6, 3, -1,
            3, 4, -9, -16, -213, -3, -12, 6,
            -19, -19, -19, -19, -19, -19, -19, -19,
        },
        {   // Knight
            -41, -21, 4, -11, -14, -10, -46, -82,
            -8, 9, -8, 15, -5, -6, -7, -35,
            -7, -31, 27, 26, 16, -44, -2, -24,
            0, 20, 39, 39, 47, 28, 27, -1,
            -1, -35, 28, 36, 36, 34, 21, -1,
            -15, 14, 27, 10, 27, 14, -3, -97,
            -25, -3, 7, 39, 38, -3, -6, -27,
            -12, -89, 51, 2, 31, -1, 14, -47,
        },
        {   // Bishop
            -9, -16, 38, -3, -2, -4, -12, 38,
            -3, 1, 12, -7, 2, -8, 1, -9,
            30, -3, 41, -14, 1, 11, 5, 31,
            2, 14, -27, -11, 19, -33, 6, 7,
            -1, 8, 17, 1, 14, 19, 2, -4,
            18, 3, 13, 18, 8, 8, -2, -10,
            -9, -3, -2, -12, 16, -4, 6, -22,
            -18, 20, 21, 0, -4, -175, 0, -12,
        },
        {   // Rook
            19, 7, 3, 21, 18, 18, 14, 11,
            -2, 0, -10, -41, -37, 3, -2, -3,
            8, 5, -15, -12, -34, -13, -19, 3,
            4, -7, 7, -21, -13, -11, -14, 3,
            9, 11, 14, 3, -40, -10, -2, 7,
            2, 6, 0, 3, -1, -6, -3, -10,
            24, -1, -22, -11, -35, -3, -8, 18,
            -10, 19, -14, 23, 25, 25, 25, 26,
        },
        {   // Queen
            81, -56, 14, 20, 28, 17, 29, 109,
            2, -12, 51, -27, 12, 44, 49, 19,
            -1, -7, 28, -6, -30, 87, 38, 28,
            22, 41, 43, 75, 99, 65, 95, 104,
            31, 47, 58, 94, 93, 53, 92, 73,
            3, 18, 34, 63, 50, 47, 29, 24,
            -3, -4, 38, 65, 67, -4, -17, -13,
            -14, -374, -632, -867, 74, -13, -182, -22,
        },
        {   // King
            -83, -44, -27, -27, -20, 6, -5, -26,
            -21, 8, 5, 8, 8, 29, 14, 2,
            1, 8, 14, 6, 11, 36, 35, 4,
            -17, 13, 15, 18, 17, 24, 17, -6,
            -11, -13, 22, 15, 18, 14, 12, -20,
            -17, -3, 18, 31, 17, 16, -2, -14,
            -33, -4, 12, 21, 22, 10, 4, -26,
            -62, -2, 18, 18, 9, -1, 13, -52
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
                double predicted = sigmoid(w->sigmoid_k, eval);
                double error = w->data[i].result - predicted;
                w->total_error += error * error;
            }
        } else if (job == WORK_GRAD) {
            memset(w->grad, 0, sizeof(w->grad));
            memset(w->grad_mat, 0, sizeof(w->grad_mat));
            for (int i = w->start_index; i < w->end_index; i++) {
                TunerEntry *e = &w->data[i];
                int eval = evaluate(e);
                double sig = sigmoid(w->sigmoid_k, eval);
                double coeff = -2.0 * (e->result - sig) * sig * (1.0 - sig) * w->sigmoid_k * log(10.0) / 400.0 / w->count;
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
            double predicted = sigmoid(sigmoid_k, eval);
            double error = data[i].result - predicted;
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
    printf("\n// Tuned Material scores — paste into evaluation.c\n");
    printf("const int material_score[2][12] = {\n");
    for (int phase = 0; phase < 2; phase++) {
        printf("    { ");
        for (int piece = 0; piece < 6; piece++) printf("%d, ", material[phase][piece]);
        for (int piece = 0; piece < 5; piece++) printf("%d, ", -material[phase][piece]);
        printf("0 }");
        if (phase == 0) printf(",");
        printf("\n");
    }
    printf("};\n");

    printf("\n// Tuned PSQT tables — paste into evaluation.c\n");
    printf("const int positional_score[2][6][64] = {\n");
    for (int phase = 0; phase < 2; phase++) {
        printf("    {   // %s\n", phase == 0 ? "Opening" : "Endgame");
        for (int piece = 0; piece < 6; piece++) {
            printf("        {   // %s\n", piece_names[piece]);
            for (int rank = 0; rank < 8; rank++) {
                printf("            ");
                for (int file = 0; file < 8; file++) {
                    int sq = rank * 8 + file;
                    printf("%d", psqt[phase][piece][sq]);
                    if (sq < 63 || piece < 5 || phase < 1) printf(", ");
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./tuner <datagen_dir> [num_threads]\n");
        return 1;
    }

    if (argc >= 3) {
        num_threads = atoi(argv[2]);
        if (num_threads < 1) num_threads = 1;
        if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    }

    int count = 0;
    TunerEntry *data = NULL;
    
    DIR *dir;
    struct dirent *ent;
    const char *dir_name = argv[1];

    if ((dir = opendir(dir_name)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // Only process .txt files
            if (strstr(ent->d_name, ".txt")) {
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", dir_name, ent->d_name);

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
        printf("Failed to open directory: %s\n", dir_name);
        return 1;
    }

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

