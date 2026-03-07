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
    { 228, 558, 608, 639, 1717, 0},
    { 512, 708, 933, 1406, 1786, 0}
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
            -61, -61, -61, -61, -61, -61, -61, -61,
            188, 374, 253, 269, 180, 189, 206, 345,
            -118, 57, 72, 57, 29, 74, 48, 13, 
            -154, -136, -28, -1, 23, -157, 132, -146,
            -176, -63, -92, 24, -35, -114, -4, -105,
            -109, -7, -43, 19, -8, -67, 15, 4,
            28, 19, 19, -39, -77, 1, 32, 35,
            -61, -61, -61, -61, -61, -61, -61, -61,
        },
        {   // Knight
            -318, -104, 62, 42, 208, 103, -128, -220,
            -110, 23, 190, 224, 119, 128, 70, 41,
            -46, 140, 98, 100, 77, 250, 45, -19,
            97, 86, 63, 160, 55, 75, 112, -51,
            -146, -109, 27, 13, 50, 99, 49, 61,
            13, -130, 121, -202, -27, 151, -76, -86,
            -81, -232, 154, 88, 39, -39, -240, -18, 
            -218, -80, -40, -317, -36, -199, 49, -156,
        },
        {   // Bishop
            -358, -80, 20, 97, 30, 5, -62, -57,
            -168, -115, -24, 30, 38, -116, -34, -173,
            -111, 45, 47, 62, 39, 53, -144, 45, 
            -170, 13, 9, -33, 77, 33, 90, -69,
            -97, -83, 7, -34, 56, 41, -56, 26,
            106, 68, 146, 115, 63, 136, 128, 46,
            -36, 183, 6, 190, 119, 299, 168, -188,
            -219, 170, 2, -245, -298, -65, 107, 159, 
        },
        {   // Rook
            74, 152, 23, 225, 235, 178, 81, 251,
            102, 134, 185, 184, 126, 240, 145, 186,
            169, 77, 22, 158, 1, 162, 76, 228,
            129, -4, 33, -1, 31, 30, -127, 119, 
            -62, -186, -198, -228, -301, -356, -195, 122,
            113, -275, -296, -325, -357, -326, -344, 21,
            129, -139, -289, -345, -365, -187, -338, 63,
            107, 98, 75, 182, 156, 186, 127, 170, 
        },
        {   // Queen
            -96, 147, -40, 117, -63, -6, 114, -114,
            126, 167, 202, -31, 119, 194, 168, -12,
            150, 45, 102, 153, 31, 119, 50, 28,
            53, 24, 30, -124, 24, -222, -84, -311,
            174, -3, 40, -161, -96, 64, -303, 66, 
            133, 179, 100, -1, 42, 56, -68, -119,
            32, -2, 196, 146, 170, 26, -45, -280,
            -294, -110, -114, 77, 253, 92, -762, -523,
        },
        {   // King
            -6, 82, 75, 44, 3, 25, 61, 72,
            78, 58, 25, 52, 5, 55, 21, 30, 
            1, 83, 61, 43, 108, 232, 305, -7,
            -120, 24, 47, 185, 149, 252, 137, 23,
            -255, -216, -32, -171, 118, -217, 62, -215,
            -249, -146, -92, -168, -80, -108, -104, -177,
            -179, -122, -47, -17, 127, 123, -20, -108, 
            -201, -91, 232, 0, 62, -46, 176, -83,
        }
    },
    {   // Endgame
        {   // Pawn
            -264, -264, -264, -264, -264, -264, -264, -264,
            569, 755, 739, 611, 589, 523, 532, 793,
            258, 360, 284, 131, 138, 71, 109, 328, 
            -262, 41, -83, -100, -139, -70, -86, 39,
            -162, -196, -63, -130, -91, -32, -294, 36,
            -59, -40, -85, -26, 14, 7, -12, -35,
            -13, -26, -99, -135, -372, -52, -14, 12,
            -264, -264, -264, -264, -264, -264, -264, -264,
        },
        {   // Knight
            -181, -31, 70, 86, 78, 78, -186, -222,
            -26, 52, -28, 162, 92, 35, -58, -150,
            7, -72, 179, 163, 181, 71, 58, -93, 
            -46, 170, 203, 226, 296, 153, 215, -5,
            -141, -175, 255, 283, 231, 223, 132, 64,
            -155, 6, 112, 114, 102, -35, -269, -285,
            -197, -206, -211, 136, 152, -307, -259, -167, 
            -152, -268, 268, -279, -46, -338, 155, -187,
        },
        {   // Bishop
            -211, -106, 231, 32, 49, 11, -191, 298,
            -166, 43, 107, 62, 81, -148, -27, -100,
            147, -138, 269, 42, 107, 196, -31, 214,
            -39, 154, -3, 72, 258, 2, 113, 112, 
            -40, 65, 244, 187, 112, 273, 80, 55,
            168, 17, 195, 85, 106, 77, -170, 57,
            -201, 49, -300, -423, 101, -302, 132, -224,
            -313, 56, 167, -338, -370, -438, -265, -261,
        },
        {   // Rook
            62, 2, -44, 14, 62, 91, 2, 51,
            142, 165, 161, 57, 109, 161, 153, 118,
            200, 187, 105, 110, 35, 135, 83, 200,
            145, 185, 130, 93, 120, 61, 160, 173,
            103, 110, 203, 184, -145, 150, 8, 110, 
            -407, -162, -122, -39, -366, -344, -407, -122,
            73, -182, -375, -384, -634, -573, -532, -29,
            61, 90, -311, 129, 174, 70, 160, 129,
        },
        {   // Queen
            182, 69, 149, 155, 126, 54, 103, 244, 
            137, 123, 186, 108, 147, 179, 184, 154,
            73, 128, 163, 129, 105, 222, 173, 163,
            129, 118, 178, 210, 234, 200, 230, 239,
            15, 182, 193, 229, 228, 188, 227, 208,
            -127, -17, 169, 198, 185, 68, 164, 93,
            -341, -74, 11, 38, 61, -220, -169, -175,
            -610, -1239, -1497, -1277, 88, -491, -1047, -223,        
        },
        {   // King
            -57, -220, -112, -234, -115, -114, -125, 31,
            -242, 12, -111, -156, -132, 86, 79, 59,
            -121, 116, 117, 192, 260, 347, 399, -84, 
            -193, 31, 189, 306, 295, 367, 280, 25,
            -281, -249, 210, 232, 283, 192, 168, -220,
            -356, -198, -16, 273, -7, 153, -144, -208,
            -331, -176, -58, 6, 97, -19, -24, -212,
            -397, -147, 247, 18, 78, -125, 284, -286
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

