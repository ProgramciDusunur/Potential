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
    { 13, 9, -12, 2, 1, 0 },
    { -12, -40, -10, -20, -69, 0 }
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
            65, 153, 56, 54, -6, 137, 48, -45,
            -38, -40, -75, 32, -6, -24, 22, -72,
            -33, -27, -18, -16, 11, -20, 5, -11,
            -12, -27, -32, -20, -23, -28, -21, -36,
            -12, -26, -33, -38, -32, -26, -19, -26,
            -20, -29, 9, -8, 0, -14, -13, -30,
            19, 19, 19, 19, 19, 19, 19, 19,
        },
        {   // Knight
            -149, -33, 33, -51, 117, -41, 41, -51,
            -17, 15, 128, 92, 102, -18, 19, 39,
            9, 69, -16, 27, -4, 49, 45, 100,
            25, -7, 8, 10, -21, 52, -18, 30,
            -43, 60, -15, -20, -15, -28, 10, -50,
            -19, -10, -33, -16, -13, -26, -18, -44,
            -9, -26, -42, -35, -41, -18, 9, -28,
            -49, -12, -29, -11, -39, -4, -22, 13,
        },
        {   // Bishop
            -157, 63, 16, 22, 34, 17, 66, -251,
            33, -2, 41, 46, 37, 40, 18, 12,
            -41, 96, -40, 18, 94, -27, -35, -29,
            -34, -15, 34, 10, -38, 34, -39, 51,
            10, 11, -31, -31, -2, -30, -6, -32,
            -57, 32, -7, -22, -19, -5, -14, -4,
            5, -20, 35, -24, -10, 58, -1, 13,
            21, -2, -22, 3, 0, 5, 38, 13,
        },
        {   // Rook
            43, 53, -68, 62, 36, 20, 42, 54,
            -10, 43, 44, 46, -12, 78, 37, 55,
            6, 30, 20, 47, 16, 25, 72, 27,
            -13, 0, 18, 37, 35, 46, -7, -9,
            -25, -42, -1, 55, 14, -34, 19, -70,
            -42, -40, -16, -21, -25, -33, -14, -54,
            -31, -64, -10, -27, -31, -33, -55, -18,
            -20, -41, -18, -46, -53, -41, -35, -35,
        },
        {   // Queen
            -149, 192, 254, 210, 177, 133, 84, -174,
            132, 97, 65, -9, 97, 50, -44, -89,
            82, 93, 47, 97, 106, -65, -10, 79,
            -38, -26, 22, -24, -1, 25, -26, -30,
            2, -49, -12, -52, -9, -34, -55, -63,
            -82, -8, -38, -44, -57, -30, -59, -104,
            -32, -42, -20, -40, -35, -18, -85, -77,
            43, -109, -43, -12, -46, -76, -72, -59,
        },
        {   // King
            -52, 36, 29, -2, -43, -21, 15, 26,
            42, 12, -7, 6, 5, 9, -25, -16,
            4, 37, 15, -3, -7, 19, 35, -9,
            -4, -7, 1, -14, -17, -12, -1, -23,
            -36, 12, -14, -26, -33, -31, -20, -38,
            -1, 50, -20, -33, -27, 2, 15, -14,
            14, -2, 0, -19, -14, 14, 27, 17,
            -2, 49, -2, -12, 27, 12, 5, 15,
        }
    },
    {   // Endgame
        {   // Pawn
            -18, -18, -18, -18, -18, -18, -18, -18,
            15, 79, 119, 51, 92, 25, 1, 39,
            0, 36, 48, 33, 13, 21, 8, 46,
            -29, 6, 41, -13, -21, -9, -7, -21,
            10, -23, 11, -23, 27, 15, -64, 11,
            2, 20, 25, 40, 15, 5, 1, -8,
            4, 21, -57, -27, -212, -53, -7, 6,
            -18, -18, -18, -18, -18, -18, -18, -18,
        },
        {   // Knight
            -41, -21, 4, -11, -14, -10, -46, -82,
            -8, 9, -8, 15, 8, -6, -7, -35,
            -7, -31, 27, 26, 16, -44, -2, -24,
            0, 20, 39, 39, 47, 28, 27, -1,
            -1, -35, 33, 42, 33, 34, 21, -1,
            -15, 14, 16, 10, 27, 14, -3, -97,
            -25, -3, 7, 50, 59, -3, -6, -27,
            -12, -89, 52, 2, 31, -1, 4, -47,
        },
        {   // Bishop
            -9, -16, 38, -3, -2, -4, -12, -19,
            -3, 1, 12, -7, 2, -8, 1, -9,
            30, -3, 41, -14, 3, 11, 5, 40,
            2, 14, -27, -11, 19, -33, 6, 7,
            -1, 8, 17, 1, 14, 19, 2, -4,
            10, 3, 13, 18, 8, 8, -2, -10,
            -9, -3, -2, -27, 16, -4, -1, -22,
            -18, 20, 31, 0, -4, -175, 0, -12,
        },
        {   // Rook
            18, 13, 2, 20, 17, 17, 13, 10,
            -3, -1, -11, -42, -38, 2, -3, -4,
            7, 4, -16, -13, -41, -14, -20, 2,
            3, -8, 6, -22, -14, -12, -15, 2,
            8, 10, 13, 9, -41, -11, -3, 6,
            1, 5, -1, 2, -2, -7, -4, -11,
            30, -2, -23, -12, -36, -4, -9, 17,
            -29, 32, -15, 19, 36, 34, 46, 72,
        },
        {   // Queen
            81, -56, 14, 20, 28, 17, 29, 92,
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
            -17, -3, 18, 33, 15, 27, -2, -14,
            -33, -4, 12, 21, 23, 16, 8, -26,
            -62, -2, 20, 35, -25, -1, 13, -52
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
    tune(data, count, sigmoid_k, 10000);

    print_psqt();

    free(data);
    return 0;
}

