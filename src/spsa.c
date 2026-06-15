#include "spsa.h"
#include "search.h"
#include "evaluation.h"


// ═══════════════════════════════════════════════════════════
//  Time management parameters (defined in uci.c)
// ═══════════════════════════════════════════════════════════
extern TUNE_DOUBLE DEF_TIME_MULTIPLIER;
extern TUNE_DOUBLE DEF_INC_MULTIPLIER;
extern TUNE_DOUBLE MAX_TIME_MULTIPLIER;
extern TUNE_DOUBLE HARD_LIMIT_MULTIPLIER;
extern TUNE_DOUBLE SOFT_LIMIT_MULTIPLIER;
extern TUNE_INT FP_HIST_MULT;
extern TUNE_INT FP_HIST_DIVISOR;
extern TUNE_INT LMP_HIST_MULT;
extern TUNE_INT LMP_HIST_DIVISOR;
extern TUNE_DOUBLE TM_BEST_MOVE_SCALE_0;
extern TUNE_DOUBLE TM_BEST_MOVE_SCALE_1;
extern TUNE_DOUBLE TM_BEST_MOVE_SCALE_2;
extern TUNE_DOUBLE TM_BEST_MOVE_SCALE_3;
extern TUNE_DOUBLE TM_BEST_MOVE_SCALE_4;
extern TUNE_DOUBLE TM_EVAL_SCALE_0;
extern TUNE_DOUBLE TM_EVAL_SCALE_1;
extern TUNE_DOUBLE TM_EVAL_SCALE_2;
extern TUNE_DOUBLE TM_EVAL_SCALE_3;
extern TUNE_DOUBLE TM_EVAL_SCALE_4;
extern TUNE_DOUBLE TM_COMPLEXITY_BASE;
extern TUNE_DOUBLE TM_COMPLEXITY_DIVISOR;
extern TUNE_DOUBLE TM_COMPLEXITY_MULT;
extern TUNE_DOUBLE TM_NODE_FRACTION_BASE;
extern TUNE_DOUBLE TM_NODE_MULTIPLIER;

// ═══════════════════════════════════════════════════════════
//  Material Evaluation parameters (defined in evaluation.c)
// ═══════════════════════════════════════════════════════════
extern TUNE_INT MG_PAWN_MAT;
extern TUNE_INT MG_KNIGHT_MAT;
extern TUNE_INT MG_BISHOP_MAT;
extern TUNE_INT MG_ROOK_MAT;
extern TUNE_INT MG_QUEEN_MAT;
extern TUNE_INT EG_PAWN_MAT;
extern TUNE_INT EG_KNIGHT_MAT;
extern TUNE_INT EG_BISHOP_MAT;
extern TUNE_INT EG_ROOK_MAT;
extern TUNE_INT EG_QUEEN_MAT;
// ═══════════════════════════════════════════════════════════
//  Search parameters (defined in search.c)
// ═══════════════════════════════════════════════════════════
extern TUNE_INT RFP_MARGIN;
extern TUNE_INT RFP_IMPROVING_MARGIN;
extern TUNE_INT NMP_BASE_REDUCTION;
extern TUNE_INT NMP_DEPTH_MULTIPLIER;
extern TUNE_INT NMP_REDUCTION_DEPTH_MULT;
extern TUNE_INT NMP_EVAL_MULT;
extern TUNE_INT NMP_FAILED_HIGH_HIST_BASE;
extern TUNE_INT NMP_FAILED_HIGH_HIST_MULT;
extern TUNE_INT NMP_FAILED_HIGH_HIST_DIVISOR;
extern TUNE_INT NMP_EVAL_BETA_MARGIN;
extern TUNE_INT NMP_VERIFICATION_MARGIN;
extern TUNE_INT NMP_REDUCTION_DIVISOR;
extern TUNE_INT NMP_EVAL_DIVISOR;
extern TUNE_INT RFP_CORRPLEXITY_MULT;
extern TUNE_INT RFP_CORRPLEXITY_DIVISOR;
extern TUNE_INT ASP_WINDOW_BASE;
extern TUNE_INT ASP_WINDOW_DIVISOR;
extern TUNE_DOUBLE ASP_WINDOW_MULTIPLIER;
extern TUNE_DOUBLE LMR_TABLE_BASE_NOISY;
extern TUNE_DOUBLE LMR_TABLE_NOISY_MULT;
extern TUNE_DOUBLE LMR_TABLE_BASE_QUIET;
extern TUNE_DOUBLE LMR_TABLE_QUIET_MULT;

// SEE
extern TUNE_INT QS_SEE_THRESHOLD;
extern TUNE_INT QS_FP_SEE_THRESHOLD;
extern TUNE_INT SEE_QUIET_HIST_DIVISOR;
extern TUNE_INT SEE_MOVE_ORDERING_THRESHOLD;
extern TUNE_INT SEE_QUIET_THRESHOLD;
extern TUNE_INT SEE_NOISY_THRESHOLD;
extern TUNE_INT MOVE_ORDER_HIST_MULT;
extern TUNE_INT SEE_PRUNING_HIST_MULT;
extern TUNE_INT SEE_QUIET_HIST_MULT;
extern TUNE_INT SEE_PIECE_VALUES[];

// LMR Scalars
extern TUNE_INT DEEPER_LMR_MARGIN;
extern TUNE_INT QUIET_HISTORY_LMR_MULT;
extern TUNE_INT QUIET_HISTORY_LMR_DIVISOR;
extern TUNE_INT QUIET_HISTORY_LMR_MINIMUM_SCALAR;
extern TUNE_INT QUIET_HISTORY_LMR_MAXIMUM_SCALAR;
extern TUNE_INT PAWN_HISTORY_LMR_MULT;
extern TUNE_INT PAWN_HISTORY_LMR_DIVISOR;
extern TUNE_INT PAWN_HISTORY_LMR_MINIMUM_SCALAR;
extern TUNE_INT PAWN_HISTORY_LMR_MAXIMUM_SCALAR;
extern TUNE_INT NOISY_HISTORY_LMR_MULT;
extern TUNE_INT NOISY_HISTORY_LMR_DIVISOR;
extern TUNE_INT QUIET_NON_PV_LMR_SCALAR;
extern TUNE_INT CUT_NODE_LMR_SCALAR;
extern TUNE_INT TT_PV_LMR_SCALAR;
extern TUNE_INT TT_PV_FAIL_LOW_LMR_SCALAR;
extern TUNE_INT TT_CAPTURE_LMR_SCALAR;
extern TUNE_INT GOOD_EVAL_LMR_SCALAR;
extern TUNE_INT GOOD_EVAL_LMR_MARGIN;
extern TUNE_INT FUTILITY_LMR_BASE;
extern TUNE_INT FUTILITY_LMR_MULT;
extern TUNE_INT FUTILITY_LMR_SCALAR;
extern TUNE_INT IMPROVING_LMR_SCALAR;
extern TUNE_INT IMPROVING_FAIL_HIGH_MARGIN;
extern TUNE_INT GIVES_CHECK_LMR_SCALAR;
extern TUNE_INT CUT_NODE_LMR_NO_TT_SCALAR;
extern TUNE_INT TT_PV_LMR_PV_NODE_SCALAR;
extern TUNE_INT TT_PV_LMR_IMPROVING_SCALAR;
extern TUNE_INT LMR_DEPTH_HIST_MULT;
extern TUNE_INT LMR_DEPTH_HIST_DIVISOR;

// Move Ordering
extern TUNE_INT MAIN_HIST_WEIGHT;
extern TUNE_INT MAIN_HIST_DIVISOR;
extern TUNE_INT CONTHIST_1_WEIGHT;
extern TUNE_INT CONTHIST_1_DIVISOR;
extern TUNE_INT CONTHIST_2_WEIGHT;
extern TUNE_INT CONTHIST_2_DIVISOR;
extern TUNE_INT CONTHIST_4_WEIGHT;
extern TUNE_INT CONTHIST_4_DIVISOR;
extern TUNE_INT PAWN_HIST_WEIGHT;
extern TUNE_INT PAWN_HIST_DIVISOR;

// LMP
extern TUNE_INT LMP_HIST_LIMIT;
extern TUNE_INT LMP_BASE;
extern TUNE_INT LMP_MULTIPLIER;

// Singular Extensions
extern TUNE_INT TRIPLE_EXT_HIST_DIVISOR;
extern TUNE_INT TRIPLE_EXT_NOISY_BONUS;
extern TUNE_INT TRIPLE_EXT_QUIET_TT_BONUS;
extern TUNE_INT MULTI_LOW_DEPTH_EXT_MARGIN;
extern TUNE_INT QUADRUPLE_EXT_NOISY_BONUS;


// Probcut
extern TUNE_INT PROBCUT_BETA_MARGIN;
extern TUNE_INT PROBCUT_IMPROVING_MARGIN;
extern TUNE_INT PROBCUT_SEE_NOISY_THRESHOLD;
extern TUNE_INT PROBCUT_NOISY_HIST_MULT;
extern TUNE_INT PROBCUT_NOISY_HIST_DIVISOR;
extern TUNE_INT PROBCUT_REDUCTION_MULTIPLIER;
extern TUNE_INT PROBCUT_CUTNODE_SCALAR;
extern TUNE_INT PROBCUT_FP_BASE;
extern TUNE_INT PROBCUT_FP_MULT;
extern TUNE_INT SPROBCUT_BETA_MARGIN;

// Futility & Razoring
extern TUNE_INT FP_MARGIN;
extern TUNE_INT FUTILITY_PRUNING_OFFSET[];
extern TUNE_INT BNFP_MARGIN;
extern TUNE_INT QUIET_HISTORY_PRUNING_MARGIN;
extern TUNE_INT RAZORING_FULL_MARGIN;
extern TUNE_INT RAZORING_DEPTH_SCALE;
extern TUNE_INT RAZORING_VERIFY_MARGIN;

// History Bonuses
extern TUNE_INT QUIET_HIST_BONUS_BASE;
extern TUNE_INT QUIET_HIST_BONUS_DEPTH;
extern TUNE_INT QUIET_HIST_BONUS_MAX;
extern TUNE_INT QUIET_HIST_MALUS_BASE;
extern TUNE_INT QUIET_HIST_MALUS_DEPTH;
extern TUNE_INT QUIET_HIST_FAILED_LOW_BONUS;
extern TUNE_INT QUIET_HIST_FAILED_LOW_MALUS;
extern TUNE_INT QUIET_HIST_MALUS_MAX;
extern TUNE_INT HISTORY_RED_MULT;
extern TUNE_INT HISTORY_RED_DIVISOR;
extern TUNE_INT CONTHIST_MULT;

extern TUNE_INT CONTHIST_BONUS_BASE;
extern TUNE_INT CONTHIST_BONUS_DEPTH;
extern TUNE_INT CONTHIST_BONUS_MAX;
extern TUNE_INT CONTHIST_MALUS_BASE;
extern TUNE_INT CONTHIST_MALUS_DEPTH;
extern TUNE_INT CONTHIST_FAILED_LOW_BONUS;
extern TUNE_INT CONTHIST_FAILED_LOW_MALUS;
extern TUNE_INT CONTHIST_MALUS_MAX;

extern TUNE_INT LDSE_BASE_MARGIN;
extern TUNE_INT LDSE_CORRECTION_MULT;
extern TUNE_INT LDSE_CORRECTION_DIVISOR;

extern TUNE_INT PAWNHIST_BONUS_BASE;
extern TUNE_INT PAWNHIST_BONUS_DEPTH;
extern TUNE_INT PAWNHIST_BONUS_MAX;
extern TUNE_INT PAWNHIST_MALUS_BASE;
extern TUNE_INT PAWNHIST_MALUS_DEPTH;
extern TUNE_INT PAWNHIST_FAILED_LOW_BONUS;
extern TUNE_INT PAWNHIST_FAILED_LOW_MALUS;
extern TUNE_INT PAWNHIST_MALUS_MAX;

extern TUNE_INT CAPTHIST_BONUS_BASE;
extern TUNE_INT CAPTHIST_BONUS_DEPTH;
extern TUNE_INT CAPTHIST_BONUS_MAX;
extern TUNE_INT CAPTHIST_MALUS_BASE;
extern TUNE_INT CAPTHIST_MALUS_DEPTH;
extern TUNE_INT CAPTHIST_MALUS_MAX;

extern TUNE_INT BAD_QUIET_INDEX_SCALE;

// ═══════════════════════════════════════════════════════════
//  Correction History parameters (defined in history.c)
// ═══════════════════════════════════════════════════════════
extern TUNE_INT PAWN_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT PAWN_CORRHIST_GRAIN;
extern TUNE_INT MINOR_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT MINOR_CORRHIST_GRAIN;
extern TUNE_INT MAJOR_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT MAJOR_CORRHIST_GRAIN;
extern TUNE_INT NON_PAWN_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT NON_PAWN_CORRHIST_GRAIN;
extern TUNE_INT KRP_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT KRP_CORRHIST_GRAIN;
extern TUNE_INT CONT_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT CONT_CORRHIST_GRAIN;

/*██████████████████████████████████████████████████████████████*\
  ██                                                          ██
  ██                 SPSA  Parameter Registry                 ██
  ██                                                          ██
\*██████████████████████████████████████████████████████████████*/


SPSAParam spsa_params[MAX_SPSA_PARAMS];
int spsa_count = 0;

static void spsa_add_int(const char *name, const void *ptr, int def, int min, int max, double c, double r) {
    spsa_params[spsa_count++] = (SPSAParam){name, 0, (void *)ptr, (double)def, (double)min, (double)max, c, r};
}

static void spsa_add_double(const char *name, const void *ptr, double def, double min, double max, double c, double r) {
    spsa_params[spsa_count++] = (SPSAParam){name, 1, (void *)ptr, def, min, max, c, r};
}


void spsa_init(void) {
    if (!SPSA_ACTIVE) return;
    spsa_count = 0;

    // ── Reverse Futility Pruning ──
    spsa_add_int("RFP_MARGIN",                  &RFP_MARGIN,                54,     20,    100,   5.00, 0.002);
    spsa_add_int("RFP_IMPROVING_MARGIN",        &RFP_IMPROVING_MARGIN,      41,     15,     80,   4.00, 0.002);
    spsa_add_int("RFP_CORRPLEXITY_MULT",        &RFP_CORRPLEXITY_MULT,      9467,   2560,  25600, 2048.0, 0.002);
    spsa_add_int("RFP_CORRPLEXITY_DIVISOR",     &RFP_CORRPLEXITY_DIVISOR,    506,    128,   2048,  128.0, 0.002);

    // ── Null Move Pruning ──
    spsa_add_int("NMP_BASE_REDUCTION",          &NMP_BASE_REDUCTION,      5161,   3000,   7000,  50.00, 0.002);
    spsa_add_int("NMP_DEPTH_MULTIPLIER",        &NMP_DEPTH_MULTIPLIER,     253,    128,    512,  25.00, 0.002);
    spsa_add_int("NMP_REDUCTION_DEPTH_MULT",    &NMP_REDUCTION_DEPTH_MULT,  8254,   2048,  32768, 1536.0, 0.002);
    spsa_add_int("NMP_EVAL_MULT",               &NMP_EVAL_MULT,            130,     32,    256,  12.00, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_BASE",   &NMP_FAILED_HIGH_HIST_BASE,99,     50,    200,  10.00, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_MULT",   &NMP_FAILED_HIGH_HIST_MULT, 28913,  5120,  51200, 2560.0, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_DIVISOR",&NMP_FAILED_HIGH_HIST_DIVISOR, 548,  128,   2048,  128.0, 0.002);
    spsa_add_int("NMP_EVAL_BETA_MARGIN",        &NMP_EVAL_BETA_MARGIN,      79,     20,    150,  10.00, 0.002);
    spsa_add_int("NMP_VERIFICATION_MARGIN",     &NMP_VERIFICATION_MARGIN,   30,     10,     80,   5.00, 0.002);
    spsa_add_int("NMP_REDUCTION_DIVISOR",       &NMP_REDUCTION_DIVISOR,  8311886, 4194304, 16777216, 512000.0, 0.002);
    spsa_add_int("NMP_EVAL_DIVISOR",            &NMP_EVAL_DIVISOR,       47369,  25600, 102400, 5000.0, 0.002);

    // ── Aspiration Windows ──
    spsa_add_int("ASP_WINDOW_BASE",             &ASP_WINDOW_BASE,            2078,   1536,  12800, 1024.00, 0.002);
    spsa_add_int("ASP_WINDOW_DIVISOR",          &ASP_WINDOW_DIVISOR,          573,    128,   2048,  128.00, 0.002);
    spsa_add_double("ASP_WINDOW_MULTIPLIER",    &ASP_WINDOW_MULTIPLIER,    1.9590951422792378,    1.2,    3.0,   0.15, 0.002);

    // ── LMR Table Parameters ──
    spsa_add_double("LMR_TABLE_BASE_NOISY",     &LMR_TABLE_BASE_NOISY,    0.3823922091988891,   0.10,   1.00,  0.03, 0.002);
    spsa_add_double("LMR_TABLE_NOISY_MULT",     &LMR_TABLE_NOISY_MULT,  131.97109521413975,  64.00, 256.00,  8.00, 0.002);
    spsa_add_double("LMR_TABLE_BASE_QUIET",     &LMR_TABLE_BASE_QUIET,    0.9905532666670359,   0.50,   2.00,  0.05, 0.002);
    spsa_add_double("LMR_TABLE_QUIET_MULT",     &LMR_TABLE_QUIET_MULT,  128.73672916932676,  64.00, 256.00,  8.00, 0.002);

    // ── SEE ──
    spsa_add_int("QS_SEE_THRESHOLD",            &QS_SEE_THRESHOLD,            6,   -50,     50,   5.00, 0.002);
    spsa_add_int("SEE_MOVE_ORDERING_THRESHOLD", &SEE_MOVE_ORDERING_THRESHOLD,-61, -150,      0,   8.00, 0.002);
    spsa_add_int("SEE_QUIET_THRESHOLD",         &SEE_QUIET_THRESHOLD,       -66,  -120,    -20,   5.00, 0.002);
    spsa_add_int("SEE_NOISY_THRESHOLD",         &SEE_NOISY_THRESHOLD,       -31,   -80,      0,   4.00, 0.002);
    spsa_add_int("MOVE_ORDER_HIST_MULT",        &MOVE_ORDER_HIST_MULT,      478,   128,   1024,  40.00, 0.002);
    spsa_add_int("SEE_PRUNING_HIST_MULT",       &SEE_PRUNING_HIST_MULT,     526,   128,   1024,  40.00, 0.002);
    spsa_add_int("SEE_QUIET_HIST_MULT",         &SEE_QUIET_HIST_MULT,       128,    32,    512,  15.00, 0.002);
    spsa_add_int("SEE_QUIET_HIST_DIVISOR",      &SEE_QUIET_HIST_DIVISOR,  12288,  4096,  32768, 1000.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_PAWN",        &SEE_PIECE_VALUES[0],       88,    50,    200,  10.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_KNIGHT",      &SEE_PIECE_VALUES[1],       322,   150,    500,  30.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_BISHOP",      &SEE_PIECE_VALUES[2],       284,   150,    500,  30.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_ROOK",        &SEE_PIECE_VALUES[3],       479,   300,    800,  50.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_QUEEN",       &SEE_PIECE_VALUES[4],      1346,   800,   1800, 120.00, 0.002);

    // ── LMR Scalars ──
    spsa_add_int("DEEPER_LMR_MARGIN",                 &DEEPER_LMR_MARGIN,               36,     10,     80,   4.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MULT",            &QUIET_HISTORY_LMR_MULT,           1873,    512,   8192, 512.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_DIVISOR",         &QUIET_HISTORY_LMR_DIVISOR,  8128799,  4194304, 16777216, 512000.0, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MINIMUM_SCALAR",  &QUIET_HISTORY_LMR_MINIMUM_SCALAR, 3197, 1024, 6144, 100.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MAXIMUM_SCALAR",  &QUIET_HISTORY_LMR_MAXIMUM_SCALAR, 3226, 1024, 6144, 100.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MULT",             &PAWN_HISTORY_LMR_MULT,            1117,    512,   8192, 512.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_DIVISOR",          &PAWN_HISTORY_LMR_DIVISOR,   8718261,  4194304, 16777216, 512000.0, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MINIMUM_SCALAR",   &PAWN_HISTORY_LMR_MINIMUM_SCALAR, 2989, 1024, 6144, 100.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MAXIMUM_SCALAR",   &PAWN_HISTORY_LMR_MAXIMUM_SCALAR, 3116, 1024, 6144, 100.00, 0.002);
    spsa_add_int("NOISY_HISTORY_LMR_MULT",            &NOISY_HISTORY_LMR_MULT,         107,     32,    512,  15.00, 0.002);
    spsa_add_int("NOISY_HISTORY_LMR_DIVISOR",         &NOISY_HISTORY_LMR_DIVISOR,  1314544, 655360, 2621440, 10000.0, 0.002);
    spsa_add_int("QUIET_NON_PV_LMR_SCALAR",           &QUIET_NON_PV_LMR_SCALAR,       986,    256,   2048,  50.00, 0.002);
    spsa_add_int("CUT_NODE_LMR_SCALAR",               &CUT_NODE_LMR_SCALAR,           1934,    512,   4096, 100.00, 0.002);
    spsa_add_int("TT_PV_LMR_SCALAR",                  &TT_PV_LMR_SCALAR,              1069,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_PV_FAIL_LOW_LMR_SCALAR",         &TT_PV_FAIL_LOW_LMR_SCALAR,     1106,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_CAPTURE_LMR_SCALAR",             &TT_CAPTURE_LMR_SCALAR,         972,    256,   2048,  50.00, 0.002);
    spsa_add_int("GOOD_EVAL_LMR_SCALAR",              &GOOD_EVAL_LMR_SCALAR,          994,    256,   2048,  50.00, 0.002);
    spsa_add_int("GOOD_EVAL_LMR_MARGIN",              &GOOD_EVAL_LMR_MARGIN,           311,    150,    600,  30.00, 0.002);
    spsa_add_int("FUTILITY_LMR_BASE",                 &FUTILITY_LMR_BASE,              168,     50,    300,  15.00, 0.002);
    spsa_add_int("FUTILITY_LMR_MULT",                 &FUTILITY_LMR_MULT,               81,     30,    150,  10.00, 0.002);
    spsa_add_int("FUTILITY_LMR_SCALAR",               &FUTILITY_LMR_SCALAR,           967,    256,   2048,  50.00, 0.002);
    spsa_add_int("IMPROVING_LMR_SCALAR",              &IMPROVING_LMR_SCALAR,          1079,    256,   2048,  50.00, 0.002);
    spsa_add_int("IMPROVING_FAIL_HIGH_MARGIN",        &IMPROVING_FAIL_HIGH_MARGIN,     90,     50,    200,  15.00, 0.002);
    spsa_add_int("GIVES_CHECK_LMR_SCALAR",            &GIVES_CHECK_LMR_SCALAR,        1031,    256,   2048,  50.00, 0.002);
    spsa_add_int("CUT_NODE_LMR_NO_TT_SCALAR",         &CUT_NODE_LMR_NO_TT_SCALAR,     1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_PV_LMR_PV_NODE_SCALAR",          &TT_PV_LMR_PV_NODE_SCALAR,      1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_PV_LMR_IMPROVING_SCALAR",        &TT_PV_LMR_IMPROVING_SCALAR,    1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("LMR_DEPTH_HIST_MULT",               &LMR_DEPTH_HIST_MULT,           2016,   1024,   8192,  50.00, 0.002);
    spsa_add_int("LMR_DEPTH_HIST_DIVISOR",      &LMR_DEPTH_HIST_DIVISOR,   16777216, 4194304, 67108864, 50000.00, 0.002);
    spsa_add_int("LMP_HIST_MULT",                     &LMP_HIST_MULT,                  257,     64,   1024,  25.00, 0.002);
    spsa_add_int("LMP_HIST_DIVISOR",                  &LMP_HIST_DIVISOR,             16984,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("LMP_BASE",                           &LMP_BASE,                      4096,   1024,  16384, 200.00, 0.002);
    spsa_add_int("LMP_MULTIPLIER",                     &LMP_MULTIPLIER,                3072,   1024,  16384, 200.00, 0.002);



    // ── Probcut ──
    spsa_add_int("PROBCUT_BETA_MARGIN",         &PROBCUT_BETA_MARGIN,            152,     50,    300,  15.00, 0.002);
    spsa_add_int("PROBCUT_IMPROVING_MARGIN",    &PROBCUT_IMPROVING_MARGIN,        38,      0,     80,   4.00, 0.002);
    spsa_add_int("PROBCUT_SEE_NOISY_THRESHOLD", &PROBCUT_SEE_NOISY_THRESHOLD,    104,      0,    250,  10.00, 0.002);
    spsa_add_int("PROBCUT_NOISY_HIST_MULT",     &PROBCUT_NOISY_HIST_MULT,        101,     32,    512,  15.00, 0.002);
    spsa_add_int("PROBCUT_NOISY_HIST_DIVISOR",  &PROBCUT_NOISY_HIST_DIVISOR, 1310720, 500000, 3000000, 100000.00, 0.002);
    spsa_add_int("PROBCUT_REDUCTION_MULTIPLIER",&PROBCUT_REDUCTION_MULTIPLIER,   256,     64,   1024,  30.00, 0.002);
    spsa_add_int("PROBCUT_CUTNODE_SCALAR",      &PROBCUT_CUTNODE_SCALAR,        1024,    256,   2048, 100.00, 0.002);
    spsa_add_int("PROBCUT_FP_BASE",             &PROBCUT_FP_BASE,                162,     50,    300,  16.00, 0.002);
    spsa_add_int("PROBCUT_FP_MULT",             &PROBCUT_FP_MULT,                101,     20,    200,  10.00, 0.002);
    spsa_add_int("SPROBCUT_BETA_MARGIN",        &SPROBCUT_BETA_MARGIN,           394,    150,    600,  30.00, 0.002);

    // ── Futility & Razoring ──
    spsa_add_int("FP_MARGIN",                   &FP_MARGIN,                       73,     30,    150,   8.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_1",   &FUTILITY_PRUNING_OFFSET[1],      88,     30,    150,   8.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_2",   &FUTILITY_PRUNING_OFFSET[2],      46,     10,    100,   4.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_3",   &FUTILITY_PRUNING_OFFSET[3],      21,      5,     50,   2.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_4",   &FUTILITY_PRUNING_OFFSET[4],      10,      0,     30,   1.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_5",   &FUTILITY_PRUNING_OFFSET[5],       5,      0,     20,   1.00, 0.002);
    spsa_add_int("BNFP_MARGIN",                 &BNFP_MARGIN,                     78,     20,    150,   7.00, 0.002);
    spsa_add_int("QUIET_HISTORY_PRUNING_MARGIN",&QUIET_HISTORY_PRUNING_MARGIN,  1763,   1024,   4096, 200.00, 0.002);
    spsa_add_int("FP_HIST_MULT",                &FP_HIST_MULT,                   489,    128,   2048,  50.00, 0.002);
    spsa_add_int("FP_HIST_DIVISOR",             &FP_HIST_DIVISOR,              15486,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("LDSE_BASE_MARGIN",            &LDSE_BASE_MARGIN,                20,      5,     80,   4.00, 0.002);
    spsa_add_int("LDSE_CORRECTION_MULT",        &LDSE_CORRECTION_MULT,           733,    256,   2048,  40.00, 0.002);
    spsa_add_int("LDSE_CORRECTION_DIVISOR",     &LDSE_CORRECTION_DIVISOR,      98643,  49152, 196608, 1000.0, 0.002);
    
    spsa_add_int("RAZORING_FULL_MARGIN",        &RAZORING_FULL_MARGIN,           209,     80,    400,  15.00, 0.002);
    spsa_add_int("RAZORING_DEPTH_SCALE",        &RAZORING_DEPTH_SCALE,            15,      5,     40,   2.00, 0.002);
    spsa_add_int("RAZORING_VERIFY_MARGIN",      &RAZORING_VERIFY_MARGIN,         124,     40,    250,  12.00, 0.002);

    // ── History Bonuses ──
    spsa_add_int("QUIET_HIST_BONUS_BASE",       &QUIET_HIST_BONUS_BASE,           8,      0,     50,   2.00, 0.002);
    spsa_add_int("QUIET_HIST_BONUS_DEPTH",      &QUIET_HIST_BONUS_DEPTH,         179,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_BONUS_MAX",        &QUIET_HIST_BONUS_MAX,          4089,   1024,   8192, 200.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_BASE",       &QUIET_HIST_MALUS_BASE,           9,      0,     50,   2.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_DEPTH",      &QUIET_HIST_MALUS_DEPTH,         194,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_FAILED_LOW_BONUS", &QUIET_HIST_FAILED_LOW_BONUS,    214,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_FAILED_LOW_MALUS", &QUIET_HIST_FAILED_LOW_MALUS,    206,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_MAX",        &QUIET_HIST_MALUS_MAX,          3954,   1024,   8192, 200.00, 0.002);
    spsa_add_int("HISTORY_RED_MULT",            &HISTORY_RED_MULT,              1045,      0,   4096,  50.00, 0.002);
    spsa_add_int("HISTORY_RED_DIVISOR",         &HISTORY_RED_DIVISOR,       16803645,  8388608, 33554432, 100000.0, 0.002);
    spsa_add_int("CONTHIST_MULT",               &CONTHIST_MULT,                 1098,      0,   4096,  50.00, 0.002);

    spsa_add_int("CONTHIST_BONUS_BASE",         &CONTHIST_BONUS_BASE,             9,      0,     50,   2.00, 0.002);
    spsa_add_int("CONTHIST_BONUS_DEPTH",        &CONTHIST_BONUS_DEPTH,           164,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_BONUS_MAX",          &CONTHIST_BONUS_MAX,            3781,   1024,   8192, 200.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_BASE",         &CONTHIST_MALUS_BASE,             12,      0,     50,   2.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_DEPTH",        &CONTHIST_MALUS_DEPTH,           216,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_FAILED_LOW_BONUS",   &CONTHIST_FAILED_LOW_BONUS,      203,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_FAILED_LOW_MALUS",   &CONTHIST_FAILED_LOW_MALUS,      175,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_MAX",          &CONTHIST_MALUS_MAX,            4250,   1024,   8192, 200.00, 0.002);

    spsa_add_int("PAWNHIST_BONUS_BASE",         &PAWNHIST_BONUS_BASE,             9,      0,     50,   2.00, 0.002);
    spsa_add_int("PAWNHIST_BONUS_DEPTH",        &PAWNHIST_BONUS_DEPTH,           220,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_BONUS_MAX",          &PAWNHIST_BONUS_MAX,            4110,   1024,   8192, 200.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_BASE",         &PAWNHIST_MALUS_BASE,             8,      0,     50,   2.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_DEPTH",        &PAWNHIST_MALUS_DEPTH,           228,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_FAILED_LOW_BONUS",   &PAWNHIST_FAILED_LOW_BONUS,      175,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_FAILED_LOW_MALUS",   &PAWNHIST_FAILED_LOW_MALUS,      206,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_MAX",          &PAWNHIST_MALUS_MAX,            3742,   1024,   8192, 200.00, 0.002);

    spsa_add_int("CAPTHIST_BONUS_BASE",         &CAPTHIST_BONUS_BASE,             9,      0,     50,   2.00, 0.002);
    spsa_add_int("CAPTHIST_BONUS_DEPTH",        &CAPTHIST_BONUS_DEPTH,           192,     50,    500,  20.00, 0.002);
    spsa_add_int("CAPTHIST_BONUS_MAX",          &CAPTHIST_BONUS_MAX,            4079,   1024,   8192, 200.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_BASE",         &CAPTHIST_MALUS_BASE,             12,      0,     50,   2.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_DEPTH",        &CAPTHIST_MALUS_DEPTH,           241,     50,    500,  20.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_MAX",          &CAPTHIST_MALUS_MAX,            4225,   1024,   8192, 200.00, 0.002);

    spsa_add_int("BAD_QUIET_INDEX_SCALE",       &BAD_QUIET_INDEX_SCALE,           27,      5,     80,   3.00, 0.002);

    // ── Correction History ──
    spsa_add_int("PAWN_CORRHIST_WEIGHT_SCALE",       &PAWN_CORRHIST_WEIGHT_SCALE,          250,     64,    512,  25.00, 0.002);
    spsa_add_int("PAWN_CORRHIST_GRAIN",              &PAWN_CORRHIST_GRAIN,                 236,     64,    512,  25.00, 0.002);
    
    spsa_add_int("MINOR_CORRHIST_WEIGHT_SCALE",      &MINOR_CORRHIST_WEIGHT_SCALE,         266,     64,    512,  25.00, 0.002);
    spsa_add_int("MINOR_CORRHIST_GRAIN",             &MINOR_CORRHIST_GRAIN,                256,     64,    512,  25.00, 0.002);
    
    spsa_add_int("MAJOR_CORRHIST_WEIGHT_SCALE",      &MAJOR_CORRHIST_WEIGHT_SCALE,         241,     64,    512,  25.00, 0.002);
    spsa_add_int("MAJOR_CORRHIST_GRAIN",             &MAJOR_CORRHIST_GRAIN,                292,     64,    512,  25.00, 0.002);
    
    spsa_add_int("NON_PAWN_CORRHIST_WEIGHT_SCALE",   &NON_PAWN_CORRHIST_WEIGHT_SCALE,      246,     64,    512,  25.00, 0.002);
    spsa_add_int("NON_PAWN_CORRHIST_GRAIN",          &NON_PAWN_CORRHIST_GRAIN,             296,     64,    512,  25.00, 0.002);
    
    spsa_add_int("KRP_CORRHIST_WEIGHT_SCALE",        &KRP_CORRHIST_WEIGHT_SCALE,           255,     64,    512,  25.00, 0.002);
    spsa_add_int("KRP_CORRHIST_GRAIN",               &KRP_CORRHIST_GRAIN,                  276,     64,    512,  25.00, 0.002);
    
    spsa_add_int("CONT_CORRHIST_WEIGHT_SCALE",       &CONT_CORRHIST_WEIGHT_SCALE,          259,     64,    512,  25.00, 0.002);
    spsa_add_int("CONT_CORRHIST_GRAIN",              &CONT_CORRHIST_GRAIN,                 223,     64,    512,  25.00, 0.002);

    // ── Move Ordering ──
    spsa_add_int("MAIN_HIST_WEIGHT",            &MAIN_HIST_WEIGHT,           1024,      0,   2048, 100.00, 0.002);
    spsa_add_int("MAIN_HIST_DIVISOR",           &MAIN_HIST_DIVISOR,          1024,    256,   4096, 200.00, 0.002);
    spsa_add_int("CONTHIST_1_WEIGHT",           &CONTHIST_1_WEIGHT,          1024,      0,   2048, 100.00, 0.002);
    spsa_add_int("CONTHIST_1_DIVISOR",          &CONTHIST_1_DIVISOR,         1024,    256,   4096, 200.00, 0.002);
    spsa_add_int("CONTHIST_2_WEIGHT",           &CONTHIST_2_WEIGHT,          1024,      0,   2048, 100.00, 0.002);
    spsa_add_int("CONTHIST_2_DIVISOR",          &CONTHIST_2_DIVISOR,         1024,    256,   4096, 200.00, 0.002);
    spsa_add_int("CONTHIST_4_WEIGHT",           &CONTHIST_4_WEIGHT,          1024,      0,   2048, 100.00, 0.002);
    spsa_add_int("CONTHIST_4_DIVISOR",          &CONTHIST_4_DIVISOR,         1024,    256,   4096, 200.00, 0.002);
    spsa_add_int("PAWN_HIST_WEIGHT",            &PAWN_HIST_WEIGHT,           1024,      0,   2048, 100.00, 0.002);
    spsa_add_int("PAWN_HIST_DIVISOR",           &PAWN_HIST_DIVISOR,          1024,    256,   4096, 200.00, 0.002);

    // ── LMP ──
    spsa_add_int("LMP_HIST_LIMIT",              &LMP_HIST_LIMIT,             6144,   1024,  16384, 500.00, 0.002);

    // ── Singular Extensions ──
    spsa_add_int("TRIPLE_EXT_HIST_DIVISOR",     &TRIPLE_EXT_HIST_DIVISOR,   16384,   4096,  65536, 2000.0, 0.002);
    spsa_add_int("TRIPLE_EXT_NOISY_BONUS",      &TRIPLE_EXT_NOISY_BONUS,       80,      0,    300,  15.00, 0.002);
    spsa_add_int("TRIPLE_EXT_QUIET_TT_BONUS",   &TRIPLE_EXT_QUIET_TT_BONUS,  100,      0,    300,  15.00, 0.002);
    spsa_add_int("MULTI_LOW_DEPTH_EXT_MARGIN",  &MULTI_LOW_DEPTH_EXT_MARGIN,    0,      0,    200,  10.00, 0.002);
    spsa_add_int("QUADRUPLE_EXT_NOISY_BONUS",   &QUADRUPLE_EXT_NOISY_BONUS,  170,      0,    500,  25.00, 0.002);
    // ── Time Management ──
    spsa_add_double("DEF_TIME_MULTIPLIER",      &DEF_TIME_MULTIPLIER,          0.04938995246391795,  0.020,  0.120,  0.005, 0.002);
    spsa_add_double("DEF_INC_MULTIPLIER",       &DEF_INC_MULTIPLIER,           0.8405102606746936,  0.400,  1.500,  0.050, 0.002);
    spsa_add_double("MAX_TIME_MULTIPLIER",      &MAX_TIME_MULTIPLIER,          0.7246779074876138,  0.300,  1.500,  0.050, 0.002);
    spsa_add_double("HARD_LIMIT_MULTIPLIER",    &HARD_LIMIT_MULTIPLIER,        3.053871909473535,  1.500,  5.000,  0.150, 0.002);
    spsa_add_double("SOFT_LIMIT_MULTIPLIER",    &SOFT_LIMIT_MULTIPLIER,        0.7196915470792545,  0.300,  1.500,  0.050, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_0",     &TM_BEST_MOVE_SCALE_0,         2.5555571922259923,   1.50,   3.50,  0.20, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_1",     &TM_BEST_MOVE_SCALE_1,         1.1955498737991779,   0.80,   2.00,  0.10, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_2",     &TM_BEST_MOVE_SCALE_2,         1.0760960818994232,   0.50,   1.50,  0.10, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_3",     &TM_BEST_MOVE_SCALE_3,         0.7526356909041305,   0.40,   1.20,  0.08, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_4",     &TM_BEST_MOVE_SCALE_4,         0.7738562009710757,   0.20,   1.00,  0.08, 0.002);
    spsa_add_double("TM_EVAL_SCALE_0",          &TM_EVAL_SCALE_0,              1.128557318263815,   0.80,   1.80,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_1",          &TM_EVAL_SCALE_1,              1.0420180311672922,   0.70,   1.60,  0.09, 0.002);
    spsa_add_double("TM_EVAL_SCALE_2",          &TM_EVAL_SCALE_2,              1.002343849632795,   0.50,   1.50,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_3",          &TM_EVAL_SCALE_3,              0.8968381433793842,   0.40,   1.40,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_4",          &TM_EVAL_SCALE_4,              0.7774009861966706,   0.40,   1.40,  0.10, 0.002);
    spsa_add_double("TM_COMPLEXITY_BASE",       &TM_COMPLEXITY_BASE,           0.7486596930255364,   0.30,   1.20,  0.08, 0.002);
    spsa_add_double("TM_COMPLEXITY_DIVISOR",    &TM_COMPLEXITY_DIVISOR,      427.2484226337165, 200.00, 800.00, 40.00, 0.002);
    spsa_add_double("TM_COMPLEXITY_MULT",       &TM_COMPLEXITY_MULT,           0.7452172363887481,   0.20,   1.50,  0.10, 0.002);
    spsa_add_double("TM_NODE_FRACTION_BASE",    &TM_NODE_FRACTION_BASE,        1.5568874256577494,   1.00,   2.50,  0.15, 0.002);
    spsa_add_double("TM_NODE_MULTIPLIER",       &TM_NODE_MULTIPLIER,           1.255447002486582,   0.80,   2.20,  0.10, 0.002);

    // ── Material Evaluation ──
    spsa_add_int("MG_PAWN_MAT", &MG_PAWN_MAT, 67, 50, 200, 10.0, 0.002);
    spsa_add_int("MG_KNIGHT_MAT", &MG_KNIGHT_MAT, 365, 200, 500, 25.0, 0.002);
    spsa_add_int("MG_BISHOP_MAT", &MG_BISHOP_MAT, 409, 200, 500, 25.0, 0.002);
    spsa_add_int("MG_ROOK_MAT", &MG_ROOK_MAT, 524, 300, 800, 35.0, 0.002);
    spsa_add_int("MG_QUEEN_MAT", &MG_QUEEN_MAT, 1100, 700, 1500, 50.0, 0.002);

    spsa_add_int("EG_PAWN_MAT", &EG_PAWN_MAT, 105, 50, 200, 10.0, 0.002);
    spsa_add_int("EG_KNIGHT_MAT", &EG_KNIGHT_MAT, 362, 200, 500, 25.0, 0.002);
    spsa_add_int("EG_BISHOP_MAT", &EG_BISHOP_MAT, 404, 200, 500, 25.0, 0.002);
    spsa_add_int("EG_ROOK_MAT", &EG_ROOK_MAT, 710, 400, 1000, 35.0, 0.002);
    spsa_add_int("EG_QUEEN_MAT", &EG_QUEEN_MAT, 1279, 800, 1800, 50.0, 0.002);

    printf("info string SPSA: %d parameters registered\n", spsa_count);
}


void spsa_print_uci_options(void) {
    if (!SPSA_ACTIVE) return;
    for (int i = 0; i < spsa_count; i++) {
        SPSAParam *p = &spsa_params[i];
        if (p->is_double) {
            // expose doubles as string (spin only supports int)
            printf("option name %s type string default %.4f\n", p->name, p->def);
        } else {
            printf("option name %s type spin default %d min %d max %d\n",
                   p->name, (int)p->def, (int)p->min, (int)p->max);
        }
    }
}


int spsa_set_option(const char *input) {
    if (!SPSA_ACTIVE) return 0;
    for (int i = 0; i < spsa_count; i++) {
        SPSAParam *p = &spsa_params[i];
        char prefix[512];
        snprintf(prefix, sizeof(prefix), "setoption name %s value ", p->name);

        if (strncmp(input, prefix, strlen(prefix)) == 0) {
            if (p->is_double) {
                double val = atof(input + strlen(prefix));
                if (val < p->min) val = p->min;
                if (val > p->max) val = p->max;
                *(double *)p->ptr = val;
                printf("info string %s = %.4f\n", p->name, val);
            } else {
                int raw = atoi(input + strlen(prefix));
                if (raw < (int)p->min) raw = (int)p->min;
                if (raw > (int)p->max) raw = (int)p->max;
                *(int *)p->ptr = raw;
                printf("info string %s = %d\n", p->name, raw);
            }

            // update evaluation tables and LMR table
            init_tables();
            initializeLMRTable();
            return 1;
        }
    }
    return 0;
}


void spsa_print_params(void) {
    if (!SPSA_ACTIVE) return;
    for (int i = 0; i < spsa_count; i++) {
        SPSAParam *p = &spsa_params[i];
        if (p->is_double) {
            printf("%s, float, %.4f, %.4f, %.4f, %.4f, %.4f\n",
                   p->name, p->def, p->min, p->max, p->c_end, p->r_end);
        } else {
            printf("%s, int, %.1f, %.1f, %.1f, %.2f, %.4f\n",
                   p->name, (double)p->def, (double)p->min, (double)p->max,
                   p->c_end, p->r_end);
        }
    }
}
