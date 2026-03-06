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
    { 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
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
            0, 0, 0, 0, 0, 0, 0, 0,
            98, 134, 54, 95, 68, 126, 34, -11,
            -57, -87, -69, -20, -84, -62, 3, -118,
            -74, -30, -67, -35, -44, -62, -52, -40,
            -26, -46, -36, -24, -31, -36, -42, -32,
            -39, -39, -37, -10, -29, -38, -31, -41,
            -47, -55, -45, -39, -55, -50, -48, -47,
            0, 0, 0, 0, 0, 0, 0, 0,
        },
        {   // Knight
            -205, -89, -34, -111, 61, -97, -15, -107,
            -73, -41, 72, 36, 23, -7, -37, -17,
            -47, 60, 37, 16, -62, 129, -11, 44,
            -31, -106, -48, -79, -109, 60, -103, -26,
            -105, 4, -84, -96, -94, -91, -46, -107,
            -128, -66, -113, -46, -90, -103, -74, -122,
            -65, -82, -98, -97, -96, -81, -47, -84,
            -105, -135, -43, -67, -63, -60, -130, -43,
        },
        {   // Bishop
            -216, 4, -82, -37, -25, -42, 7, -310,
            -26, -61, -18, -13, 30, 59, -41, -47,
            -163, 37, -155, 6, 35, -86, 32, -190,
            -80, -67, -14, -80, -97, 20, -105, -2,
            -49, -48, -54, -50, -61, -61, -56, -91,
            -116, -49, -66, -72, -78, -64, -98, -78,
            -34, -99, 10, -88, -96, -1, -88, -39,
            -38, -26, -106, -56, -59, -103, -21, -46,
        },
        {   // Rook
            32, 42, 32, 51, 63, 9, 31, 43,
            27, 32, 58, 62, 80, 67, 26, 44,
            -5, 19, 26, 36, 17, 45, 61, 16,
            -24, -11, 7, 26, 24, 35, -8, -20,
            -36, -26, -12, -1, 9, -7, 6, -111,
            -88, -51, -26, -25, -36, -15, -25, -74,
            -99, -39, 10, -9, -1, 11, -44, -86,
            -106, -90, 6, -24, -58, -40, -74, -98,
        },
        {   // Queen
            -601, 0, 29, 12, 59, 44, -176, -541,
            -24, -103, -211, 1, -16, -248, -339, -383,
            -13, -21, -229, 8, 27, -418, -315, -197,
            -314, -339, -254, -358, -411, -347, -351, -374,
            -362, -344, -293, -385, -385, -354, -425, -385,
            -377, -380, -365, -367, -356, -385, -350, -380,
            -308, -343, -386, -387, -383, -361, -361, -353,
            -233, -385, -373, -393, -387, -387, -348, -335,
        },
        {   // King
            -65, 23, 16, -15, -56, -34, 2, 13,
            29, -1, -20, -7, -8, -4, -38, -29,
            -9, 24, 2, -16, -20, 6, 22, -22,
            -17, -20, -12, -27, -30, -25, -14, -36,
            -49, -1, -27, -39, -46, -44, -33, -51,
            -14, 72, -33, -46, -44, -11, 2, -27,
            1, 7, 45, -18, -28, -22, -9, 8,
            -15, 36, 1, -7, -38, -21, -14, 14,
        }
    },
    {   // Endgame
        {   // Pawn
            0, 0, 0, 0, 0, 0, 0, 0,
            178, 173, 158, 134, 147, 132, 165, 187,
            94, 100, 85, 67, 56, 53, 82, 84,
            -64, 24, 13, 5, -15, 4, -68, 17,
            13, -27, 69, -29, 115, 57, -119, 38,
            4, 127, 21, 114, 51, -33, 46, -8,
            -9, -18, -147, -15, -194, -161, -26, -48,
            0, 0, 0, 0, 0, 0, 0, 0,
        },
        {   // Knight
            -58, -38, -13, -28, -31, -27, -63, -99,
            -25, -8, -25, -2, -9, -25, -24, -52,
            -24, -20, 10, 9, -1, -9, -19, -41,
            -17, 3, 22, 22, 22, 11, 8, -18,
            -18, -6, 16, 25, 16, 17, 4, -18,
            -58, -3, -1, 15, 10, -3, -20, -114,
            -42, -20, -10, -5, 60, -20, -23, -44,
            -29, -168, 117, -15, 49, -18, -181, -64,
        },
        {   // Bishop
            -14, -21, -11, -8, -7, -9, -17, -24,
            -8, -4, 7, -12, -3, -13, -4, -14,
            2, -8, 0, -1, -2, 6, 0, 4,
            -3, 9, 12, 9, 14, 10, 1, 2,
            -6, 3, 13, 19, 7, 10, -3, -9,
            -12, -3, 8, 10, 13, 3, -7, -15,
            -14, -18, -7, -12, 4, -9, -15, -27,
            -23, 78, 94, -5, -9, -224, -5, -17,
        },
        {   // Rook
            13, 10, 18, 15, 12, 12, 8, 5,
            11, 13, 13, 11, -3, 3, 8, 3,
            7, 7, 7, 5, 4, -3, -5, -3,
            4, 3, 13, 1, 2, 1, -1, 2,
            3, 5, 8, 4, -5, -6, -8, -11,
            -4, 0, -5, -1, -7, -12, -8, -16,
            -6, -6, 0, 2, -9, -9, -11, -3,
            -231, 2, 3, -1, -5, -13, 15, -151,
        },
        {   // Queen
            -9, 22, 22, 27, 27, 19, 10, 20,
            -17, 20, 32, 41, 58, 25, 30, 0,
            -20, 6, 9, 49, 47, 35, 19, 9,
            3, 22, 24, 45, 57, 40, 57, 36,
            -18, 28, 19, 47, 31, 34, 39, 23,
            -16, -27, 15, 6, 9, 17, 10, 5,
            -22, -23, -30, -16, -16, -23, -36, -32,
            -33, -393, -651, -886, -5, -32, -201, -41,
        },
        {   // King
            -74, -35, -18, -18, -11, 15, 4, -17,
            -12, 17, 14, 17, 17, 38, 23, 11,
            10, 17, 23, 15, 20, 45, 44, 13,
            -8, 22, 24, 27, 26, 33, 26, 3,
            -18, -4, 21, 24, 27, 23, 9, -11,
            -19, -3, 11, 21, 23, 16, 7, -9,
            -27, -11, 36, 13, 14, 4, -5, -17,
            -53, -34, -21, 142, -199, -14, -24, -43
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

