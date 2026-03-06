#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    { 82, 337, 365, 477, 1025, 0 },
    { 94, 281, 297, 512,  936, 0 }
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
    {
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            98, 134, 61, 95, 68, 126, 34, -11,
            -6, 7, 26, 31, 65, 56, 25, -20,
            -14, 13, 6, 21, 23, 12, 17, -23,
            -27, -2, -5, 12, 17, 6, 10, -25,
            -26, -4, -4, -10, 3, 3, 33, -12,
            -35, -1, -20, -23, -15, 24, 38, -22,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -167, -89, -34, -49, 61, -97, -15, -107,
            -73, -41, 72, 36, 23, 62, 7, -17,
            -47, 60, 37, 65, 84, 129, 73, 44,
            -9, 17, 19, 53, 37, 69, 18, 22,
            -13, 4, 16, 13, 28, 19, 21, -8,
            -23, -9, 12, 10, 19, 17, 25, -16,
            -29, -53, -12, -3, -1, 18, -14, -19,
            -105, -21, -58, -33, -17, -28, -19, -23
        },
        {
            -29, 4, -82, -37, -25, -42, 7, -8,
            -26, 16, -18, -13, 30, 59, 18, -47,
            -16, 37, 43, 40, 35, 50, 37, -2,
            -4, 5, 19, 50, 37, 37, 7, -2,
            -6, 13, 13, 26, 34, 12, 10, 4,
            0, 15, 15, 15, 14, 27, 18, 10,
            4, 15, 16, 0, 7, 21, 33, 1,
            -33, -3, -14, -21, -13, -12, -39, -21
        },
        {
            32, 42, 32, 51, 63, 9, 31, 43,
            27, 32, 58, 62, 80, 67, 26, 44,
            -5, 19, 26, 36, 17, 45, 61, 16,
            -24, -11, 7, 26, 24, 35, -8, -20,
            -36, -26, -12, -1, 9, -7, 6, -23,
            -45, -25, -16, -17, 3, 0, -5, -33,
            -44, -16, -20, -9, -1, 11, -6, -71,
            -19, -13, 1, 17, 16, 7, -37, -26
        },
        {
            -28, 0, 29, 12, 59, 44, 43, 45,
            -24, -39, -5, 1, -16, 57, 28, 54,
            -13, -17, 7, 8, 29, 56, 47, 57,
            -27, -27, -16, -16, -1, 17, -2, 1,
            -9, -26, -9, -10, -2, -4, 3, -3,
            -14, 2, -11, -2, -5, 2, 14, 5,
            -35, -8, 11, 2, 8, 15, -3, 1,
            -1, -18, -9, 10, -15, -25, -31, -50
        },
        {
            -65, 23, 16, -15, -56, -34, 2, 13,
            29, -1, -20, -7, -8, -4, -38, -29,
            -9, 24, 2, -16, -20, 6, 22, -22,
            -17, -20, -12, -27, -30, -25, -14, -36,
            -49, -1, -27, -39, -46, -44, -33, -51,
            -14, -14, -22, -46, -44, -30, -15, -27,
            1, 7, -8, -64, -43, -16, 9, 8,
            -15, 36, 12, -54, 8, -28, 24, 14
        }
    },
    {
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            178, 173, 158, 134, 147, 132, 165, 187,
            94, 100, 85, 67, 56, 53, 82, 84,
            32, 24, 13, 5, -2, 4, 17, 17,
            13, 9, -3, -7, -7, -8, 3, -1,
            4, 7, -6, 1, 0, -5, -1, -8,
            13, 8, 8, 10, 13, 0, 2, -7,
            0, 0, 0, 0, 0, 0, 0, 0
        },
        {
            -58, -38, -13, -28, -31, -27, -63, -99,
            -25, -8, -25, -2, -9, -25, -24, -52,
            -24, -20, 10, 9, -1, -9, -19, -41,
            -17, 3, 22, 22, 22, 11, 8, -18,
            -18, -6, 16, 25, 16, 17, 4, -18,
            -23, -3, -1, 15, 10, -3, -20, -22,
            -42, -20, -10, -5, -2, -20, -23, -44,
            -29, -51, -23, -15, -22, -18, -50, -64
        },
        {
            -14, -21, -11, -8, -7, -9, -17, -24,
            -8, -4, 7, -12, -3, -13, -4, -14,
            2, -8, 0, -1, -2, 6, 0, 4,
            -3, 9, 12, 9, 14, 10, 3, 2,
            -6, 3, 13, 19, 7, 10, -3, -9,
            -12, -3, 8, 10, 13, 3, -7, -15,
            -14, -18, -7, -1, 4, -9, -15, -27,
            -23, -9, -23, -5, -9, -16, -5, -17
        },
        {
            13, 10, 18, 15, 12, 12, 8, 5,
            11, 13, 13, 11, -3, 3, 8, 3,
            7, 7, 7, 5, 4, -3, -5, -3,
            4, 3, 13, 1, 2, 1, -1, 2,
            3, 5, 8, 4, -5, -6, -8, -11,
            -4, 0, -5, -1, -7, -12, -8, -16,
            -6, -6, 0, 2, -9, -9, -11, -3,
            -9, 2, 3, -1, -5, -13, 4, -20
        },
        {
            -9, 22, 22, 27, 27, 19, 10, 20,
            -17, 20, 32, 41, 58, 25, 30, 0,
            -20, 6, 9, 49, 47, 35, 19, 9,
            3, 22, 24, 45, 57, 40, 57, 36,
            -18, 28, 19, 47, 31, 34, 39, 23,
            -16, -27, 15, 6, 9, 17, 10, 5,
            -22, -23, -30, -16, -16, -23, -36, -32,
            -33, -28, -22, -43, -5, -32, -20, -41
        },
        {
            -74, -35, -18, -18, -11, 15, 4, -17,
            -12, 17, 14, 17, 17, 38, 23, 11,
            10, 17, 23, 15, 20, 45, 44, 13,
            -8, 22, 24, 27, 26, 33, 26, 3,
            -18, -4, 21, 24, 27, 23, 9, -11,
            -19, -3, 11, 21, 23, 16, 7, -9,
            -27, -11, 4, 13, 14, 4, -5, -17,
            -53, -34, -21, -11, -28, -14, -24, -43
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

double compute_mse(TunerEntry *data, int count, double sigmoid_k) {
    double total_error = 0.0;
    for (int i = 0; i < count; i++) {
        int eval = evaluate(&data[i]);
        double predicted = sigmoid(sigmoid_k, eval);
        double error = data[i].result - predicted;
        total_error += error * error;
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

        for (int i = 0; i < count; i++) {
            TunerEntry *e = &data[i];
            int eval = evaluate(e);
            double sig = sigmoid(sigmoid_k, eval);
            double coeff = -2.0 * (e->result - sig) * sig * (1.0 - sig) * sigmoid_k * log(10.0) / 400.0 / count;
            if (e->side == 1) coeff = -coeff;

            double mg_weight = (double)e->phase / 24.0;
            double eg_weight = 1.0 - mg_weight;

            for (int sq = 0; sq < 64; sq++) {
                int piece = e->pieces[sq];
                if (piece == NO_PIECE) continue;

                if (piece <= K) {
                    grad_mat[0][piece]              += coeff * mg_weight;
                    grad_mat[1][piece]              += coeff * eg_weight;
                    grad[0][piece][sq]              += coeff * mg_weight;
                    grad[1][piece][sq]              += coeff * eg_weight;
                } else {
                    int pt = piece - 6;
                    int mirrored = mirror[sq];
                    grad_mat[0][pt]                 -= coeff * mg_weight;
                    grad_mat[1][pt]                 -= coeff * eg_weight;
                    grad[0][pt][mirrored]           -= coeff * mg_weight;
                    grad[1][pt][mirrored]           -= coeff * eg_weight;
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

        double mse = compute_mse(data, count, sigmoid_k);
        printf("Epoch %d: MSE = %.10f\n", epoch, mse);
    }

    center_psqt();

    double final_mse = compute_mse(data, count, sigmoid_k);
    printf("Tuning complete. Final MSE = %.10f\n", final_mse);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./tuner <datagen.txt>\n");
        return 1;
    }

    int count = 0;
    TunerEntry *data = load_data(argv[1], &count);
    if (!data) return 1;

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

    printf("Finding optimal K...\n");
    double sigmoid_k = find_optimal_K(data, count);
    double mse = compute_mse(data, count, sigmoid_k);
    printf("Optimal K = %.6f\n", sigmoid_k);
    printf("Initial MSE = %.10f\n\n", mse);

    printf("Starting tuning...\n");
    tune(data, count, sigmoid_k, 1000);

    print_psqt();

    free(data);
    return 0;
}

