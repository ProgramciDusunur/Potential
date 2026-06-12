#include "spsa.h"
#include "search.h"

bool SPSA_MODE = false;

// ═══════════════════════════════════════════════════════════
//  Time management parameters (defined in uci.c)
// ═══════════════════════════════════════════════════════════
extern double DEF_TIME_MULTIPLIER;
extern double DEF_INC_MULTIPLIER;
extern double MAX_TIME_MULTIPLIER;
extern double HARD_LIMIT_MULTIPLIER;
extern double SOFT_LIMIT_MULTIPLIER;
extern int FP_HIST_MULT;
extern int FP_HIST_DIVISOR;
extern int LMP_HIST_MULT;
extern int LMP_HIST_DIVISOR;
extern double TM_BEST_MOVE_SCALE_0;
extern double TM_BEST_MOVE_SCALE_1;
extern double TM_BEST_MOVE_SCALE_2;
extern double TM_BEST_MOVE_SCALE_3;
extern double TM_BEST_MOVE_SCALE_4;
extern double TM_EVAL_SCALE_0;
extern double TM_EVAL_SCALE_1;
extern double TM_EVAL_SCALE_2;
extern double TM_EVAL_SCALE_3;
extern double TM_EVAL_SCALE_4;
extern double TM_COMPLEXITY_BASE;
extern double TM_COMPLEXITY_DIVISOR;
extern double TM_COMPLEXITY_MULT;
extern double TM_NODE_FRACTION_BASE;
extern double TM_NODE_MULTIPLIER;

// ═══════════════════════════════════════════════════════════
//  Search parameters (defined in search.c)
// ═══════════════════════════════════════════════════════════
extern int RFP_MARGIN;
extern int RFP_IMPROVING_MARGIN;
extern int NMP_BASE_REDUCTION;
extern int NMP_DEPTH_MULTIPLIER;
extern int NMP_REDUCTION_DEPTH_MULT;
extern int NMP_EVAL_MULT;
extern int NMP_FAILED_HIGH_HIST_BASE;
extern int NMP_FAILED_HIGH_HIST_MULT;
extern int NMP_EVAL_BETA_MARGIN;
extern int NMP_VERIFICATION_MARGIN;
extern int NMP_REDUCTION_DIVISOR;
extern int NMP_EVAL_DIVISOR;
extern int RFP_CORRPLEXITY_MULT;
extern int ASP_WINDOW_BASE;
extern double ASP_WINDOW_MULTIPLIER;
extern double LMR_TABLE_BASE_NOISY;
extern double LMR_TABLE_NOISY_MULT;
extern double LMR_TABLE_BASE_QUIET;
extern double LMR_TABLE_QUIET_MULT;

// SEE
extern int QS_SEE_THRESHOLD;
extern int SEE_MOVE_ORDERING_THRESHOLD;
extern int SEE_QUIET_THRESHOLD;
extern int SEE_NOISY_THRESHOLD;
extern int MOVE_ORDER_HIST_MULT;
extern int SEE_PRUNING_HIST_MULT;
extern int SEE_QUIET_HIST_MULT;
extern int SEE_PIECE_VALUES[];

// LMR Scalars
extern int DEEPER_LMR_MARGIN;
extern int QUIET_HISTORY_LMR_MULT;
extern int QUIET_HISTORY_LMR_DIVISOR;
extern int QUIET_HISTORY_LMR_MINIMUM_SCALAR;
extern int QUIET_HISTORY_LMR_MAXIMUM_SCALAR;
extern int PAWN_HISTORY_LMR_MULT;
extern int PAWN_HISTORY_LMR_DIVISOR;
extern int PAWN_HISTORY_LMR_MINIMUM_SCALAR;
extern int PAWN_HISTORY_LMR_MAXIMUM_SCALAR;
extern int NOISY_HISTORY_LMR_MULT;
extern int NOISY_HISTORY_LMR_DIVISOR;
extern int QUIET_NON_PV_LMR_SCALAR;
extern int CUT_NODE_LMR_SCALAR;
extern int TT_PV_LMR_SCALAR;
extern int TT_PV_FAIL_LOW_LMR_SCALAR;
extern int TT_CAPTURE_LMR_SCALAR;
extern int GOOD_EVAL_LMR_SCALAR;
extern int GOOD_EVAL_LMR_MARGIN;
extern int FUTILITY_LMR_BASE;
extern int FUTILITY_LMR_MULT;
extern int FUTILITY_LMR_SCALAR;
extern int IMPROVING_LMR_SCALAR;
extern int IMPROVING_FAIL_HIGH_MARGIN;
extern int GIVES_CHECK_LMR_SCALAR;
extern int LMR_DEPTH_HIST_MULT;



// Probcut
extern int PROBCUT_BETA_MARGIN;
extern int PROBCUT_IMPROVING_MARGIN;
extern int PROBCUT_SEE_NOISY_THRESHOLD;
extern int PROBCUT_NOISY_HIST_MULT;
extern int PROBCUT_FP_BASE;
extern int PROBCUT_FP_MULT;
extern int SPROBCUT_BETA_MARGIN;

// Futility & Razoring
extern int FP_MARGIN;
extern int FUTILITY_PRUNING_OFFSET[];
extern int BNFP_MARGIN;
extern int QUIET_HISTORY_PRUNING_MARGIN;
extern int RAZORING_FULL_MARGIN;
extern int RAZORING_DEPTH_SCALE;
extern int RAZORING_VERIFY_MARGIN;

// History Bonuses
extern int QUIET_HIST_BONUS_BASE;
extern int QUIET_HIST_BONUS_DEPTH;
extern int QUIET_HIST_BONUS_MAX;
extern int QUIET_HIST_MALUS_BASE;
extern int QUIET_HIST_MALUS_DEPTH;
extern int QUIET_HIST_FAILED_LOW_BONUS;
extern int QUIET_HIST_FAILED_LOW_MALUS;
extern int QUIET_HIST_MALUS_MAX;
extern int HISTORY_RED_MULT;
extern int HISTORY_RED_DIVISOR;
extern int CONTHIST_MULT;

extern int CONTHIST_BONUS_BASE;
extern int CONTHIST_BONUS_DEPTH;
extern int CONTHIST_BONUS_MAX;
extern int CONTHIST_MALUS_BASE;
extern int CONTHIST_MALUS_DEPTH;
extern int CONTHIST_FAILED_LOW_BONUS;
extern int CONTHIST_FAILED_LOW_MALUS;
extern int CONTHIST_MALUS_MAX;

extern int LDSE_BASE_MARGIN;
extern int LDSE_CORRECTION_MULT;
extern int LDSE_CORRECTION_DIVISOR;

extern int PAWNHIST_BONUS_BASE;
extern int PAWNHIST_BONUS_DEPTH;
extern int PAWNHIST_BONUS_MAX;
extern int PAWNHIST_MALUS_BASE;
extern int PAWNHIST_MALUS_DEPTH;
extern int PAWNHIST_FAILED_LOW_BONUS;
extern int PAWNHIST_FAILED_LOW_MALUS;
extern int PAWNHIST_MALUS_MAX;

extern int CAPTHIST_BONUS_BASE;
extern int CAPTHIST_BONUS_DEPTH;
extern int CAPTHIST_BONUS_MAX;
extern int CAPTHIST_MALUS_BASE;
extern int CAPTHIST_MALUS_DEPTH;
extern int CAPTHIST_MALUS_MAX;

extern int BAD_QUIET_INDEX_SCALE;

// ═══════════════════════════════════════════════════════════
//  Correction History parameters (defined in history.c)
// ═══════════════════════════════════════════════════════════
extern int PAWN_CORRHIST_WEIGHT_SCALE;
extern int PAWN_CORRHIST_GRAIN;
extern int MINOR_CORRHIST_WEIGHT_SCALE;
extern int MINOR_CORRHIST_GRAIN;
extern int MAJOR_CORRHIST_WEIGHT_SCALE;
extern int MAJOR_CORRHIST_GRAIN;
extern int NON_PAWN_CORRHIST_WEIGHT_SCALE;
extern int NON_PAWN_CORRHIST_GRAIN;
extern int KRP_CORRHIST_WEIGHT_SCALE;
extern int KRP_CORRHIST_GRAIN;
extern int CONT_CORRHIST_WEIGHT_SCALE;
extern int CONT_CORRHIST_GRAIN;

/*██████████████████████████████████████████████████████████████*\
  ██                                                          ██
  ██                 SPSA  Parameter Registry                 ██
  ██                                                          ██
\*██████████████████████████████████████████████████████████████*/


SPSAParam spsa_params[MAX_SPSA_PARAMS];
int spsa_count = 0;


static void spsa_add_int(const char *name, int *ptr, int def, int min, int max, double c, double r) {
    spsa_params[spsa_count++] = (SPSAParam){name, 0, ptr, (double)def, (double)min, (double)max, c, r};
}

static void spsa_add_double(const char *name, double *ptr, double def, double min, double max, double c, double r) {
    spsa_params[spsa_count++] = (SPSAParam){name, 1, ptr, def, min, max, c, r};
}


void spsa_init(void) {
    if (!SPSA_MODE) return;
    spsa_count = 0;

    // ── Reverse Futility Pruning ──
    spsa_add_int("RFP_MARGIN",                  &RFP_MARGIN,                52,     20,    100,   5.00, 0.002);
    spsa_add_int("RFP_IMPROVING_MARGIN",        &RFP_IMPROVING_MARGIN,      45,     15,     80,   4.00, 0.002);
    spsa_add_int("RFP_CORRPLEXITY_MULT",        &RFP_CORRPLEXITY_MULT,      20,      5,     50,   4.00, 0.002);

    // ── Null Move Pruning ──
    spsa_add_int("NMP_BASE_REDUCTION",          &NMP_BASE_REDUCTION,      5120,   3000,   7000,  50.00, 0.002);
    spsa_add_int("NMP_DEPTH_MULTIPLIER",        &NMP_DEPTH_MULTIPLIER,     256,    128,    512,  25.00, 0.002);
    spsa_add_int("NMP_REDUCTION_DEPTH_MULT",    &NMP_REDUCTION_DEPTH_MULT,  16,      4,     64,   3.00, 0.002);
    spsa_add_int("NMP_EVAL_MULT",               &NMP_EVAL_MULT,            128,     32,    256,  12.00, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_BASE",   &NMP_FAILED_HIGH_HIST_BASE,100,     50,    200,  10.00, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_MULT",   &NMP_FAILED_HIGH_HIST_MULT, 50,     10,    100,   5.00, 0.002);
    spsa_add_int("NMP_EVAL_BETA_MARGIN",        &NMP_EVAL_BETA_MARGIN,      75,     20,    150,  10.00, 0.002);
    spsa_add_int("NMP_VERIFICATION_MARGIN",     &NMP_VERIFICATION_MARGIN,   30,     10,     80,   5.00, 0.002);
    spsa_add_int("NMP_REDUCTION_DIVISOR",       &NMP_REDUCTION_DIVISOR,  16384,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("NMP_EVAL_DIVISOR",            &NMP_EVAL_DIVISOR,       51200,  25600, 102400, 5000.0, 0.002);

    // ── Aspiration Windows ──
    spsa_add_int("ASP_WINDOW_BASE",             &ASP_WINDOW_BASE,            9,      3,     25,   2.00, 0.002);
    spsa_add_double("ASP_WINDOW_MULTIPLIER",    &ASP_WINDOW_MULTIPLIER,    1.8,    1.2,    3.0,   0.15, 0.002);

    // ── LMR Table Parameters ──
    spsa_add_double("LMR_TABLE_BASE_NOISY",     &LMR_TABLE_BASE_NOISY,    0.38,   0.10,   1.00,  0.03, 0.002);
    spsa_add_double("LMR_TABLE_NOISY_MULT",     &LMR_TABLE_NOISY_MULT,  128.00,  64.00, 256.00,  8.00, 0.002);
    spsa_add_double("LMR_TABLE_BASE_QUIET",     &LMR_TABLE_BASE_QUIET,    1.01,   0.50,   2.00,  0.05, 0.002);
    spsa_add_double("LMR_TABLE_QUIET_MULT",     &LMR_TABLE_QUIET_MULT,  128.00,  64.00, 256.00,  8.00, 0.002);

    // ── SEE ──
    spsa_add_int("QS_SEE_THRESHOLD",            &QS_SEE_THRESHOLD,            0,   -50,     50,   5.00, 0.002);
    spsa_add_int("SEE_MOVE_ORDERING_THRESHOLD", &SEE_MOVE_ORDERING_THRESHOLD,-82, -150,      0,   8.00, 0.002);
    spsa_add_int("SEE_QUIET_THRESHOLD",         &SEE_QUIET_THRESHOLD,       -67,  -120,    -20,   5.00, 0.002);
    spsa_add_int("SEE_NOISY_THRESHOLD",         &SEE_NOISY_THRESHOLD,       -32,   -80,      0,   4.00, 0.002);
    spsa_add_int("MOVE_ORDER_HIST_MULT",        &MOVE_ORDER_HIST_MULT,      512,   128,   1024,  40.00, 0.002);
    spsa_add_int("SEE_PRUNING_HIST_MULT",       &SEE_PRUNING_HIST_MULT,     512,   128,   1024,  40.00, 0.002);
    spsa_add_int("SEE_QUIET_HIST_MULT",         &SEE_QUIET_HIST_MULT,       128,    32,    512,  15.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_PAWN",        &SEE_PIECE_VALUES[0],       100,    50,    200,  10.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_KNIGHT",      &SEE_PIECE_VALUES[1],       300,   150,    500,  30.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_BISHOP",      &SEE_PIECE_VALUES[2],       300,   150,    500,  30.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_ROOK",        &SEE_PIECE_VALUES[3],       500,   300,    800,  50.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_QUEEN",       &SEE_PIECE_VALUES[4],      1200,   800,   1800, 120.00, 0.002);

    // ── LMR Scalars ──
    spsa_add_int("DEEPER_LMR_MARGIN",                 &DEEPER_LMR_MARGIN,               35,     10,     80,   4.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MULT",            &QUIET_HISTORY_LMR_MULT,           4,      1,     16,   1.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_DIVISOR",         &QUIET_HISTORY_LMR_DIVISOR,  16384,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MINIMUM_SCALAR",  &QUIET_HISTORY_LMR_MINIMUM_SCALAR, 3072, 1024, 6144, 100.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MAXIMUM_SCALAR",  &QUIET_HISTORY_LMR_MAXIMUM_SCALAR, 3072, 1024, 6144, 100.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MULT",             &PAWN_HISTORY_LMR_MULT,            4,      1,     16,   1.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_DIVISOR",          &PAWN_HISTORY_LMR_DIVISOR,   16384,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MINIMUM_SCALAR",   &PAWN_HISTORY_LMR_MINIMUM_SCALAR, 3072, 1024, 6144, 100.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MAXIMUM_SCALAR",   &PAWN_HISTORY_LMR_MAXIMUM_SCALAR, 3072, 1024, 6144, 100.00, 0.002);
    spsa_add_int("NOISY_HISTORY_LMR_MULT",            &NOISY_HISTORY_LMR_MULT,         128,     32,    512,  15.00, 0.002);
    spsa_add_int("NOISY_HISTORY_LMR_DIVISOR",         &NOISY_HISTORY_LMR_DIVISOR,  1310720, 655360, 2621440, 10000.0, 0.002);
    spsa_add_int("QUIET_NON_PV_LMR_SCALAR",           &QUIET_NON_PV_LMR_SCALAR,       1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("CUT_NODE_LMR_SCALAR",               &CUT_NODE_LMR_SCALAR,           2048,    512,   4096, 100.00, 0.002);
    spsa_add_int("TT_PV_LMR_SCALAR",                  &TT_PV_LMR_SCALAR,              1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_PV_FAIL_LOW_LMR_SCALAR",         &TT_PV_FAIL_LOW_LMR_SCALAR,     1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_CAPTURE_LMR_SCALAR",             &TT_CAPTURE_LMR_SCALAR,         1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("GOOD_EVAL_LMR_SCALAR",              &GOOD_EVAL_LMR_SCALAR,          1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("GOOD_EVAL_LMR_MARGIN",              &GOOD_EVAL_LMR_MARGIN,           365,    150,    600,  30.00, 0.002);
    spsa_add_int("FUTILITY_LMR_BASE",                 &FUTILITY_LMR_BASE,              164,     50,    300,  15.00, 0.002);
    spsa_add_int("FUTILITY_LMR_MULT",                 &FUTILITY_LMR_MULT,               82,     30,    150,  10.00, 0.002);
    spsa_add_int("FUTILITY_LMR_SCALAR",               &FUTILITY_LMR_SCALAR,           1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("IMPROVING_LMR_SCALAR",              &IMPROVING_LMR_SCALAR,          1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("IMPROVING_FAIL_HIGH_MARGIN",        &IMPROVING_FAIL_HIGH_MARGIN,     100,     50,    200,  15.00, 0.002);
    spsa_add_int("GIVES_CHECK_LMR_SCALAR",            &GIVES_CHECK_LMR_SCALAR,        1024,    256,   2048,  50.00, 0.002);
    spsa_add_int("LMR_DEPTH_HIST_MULT",               &LMR_DEPTH_HIST_MULT,           2048,   1024,   8192,  50.00, 0.002);
    spsa_add_int("LMP_HIST_MULT",                     &LMP_HIST_MULT,                  256,     64,   1024,  25.00, 0.002);
    spsa_add_int("LMP_HIST_DIVISOR",                  &LMP_HIST_DIVISOR,             16384,   8192,  32768, 1000.0, 0.002);



    // ── Probcut ──
    spsa_add_int("PROBCUT_BETA_MARGIN",         &PROBCUT_BETA_MARGIN,            150,     50,    300,  15.00, 0.002);
    spsa_add_int("PROBCUT_IMPROVING_MARGIN",    &PROBCUT_IMPROVING_MARGIN,        30,      0,     80,   4.00, 0.002);
    spsa_add_int("PROBCUT_SEE_NOISY_THRESHOLD", &PROBCUT_SEE_NOISY_THRESHOLD,    100,      0,    250,  10.00, 0.002);
    spsa_add_int("PROBCUT_NOISY_HIST_MULT",     &PROBCUT_NOISY_HIST_MULT,        128,     32,    512,  15.00, 0.002);
    spsa_add_int("PROBCUT_FP_BASE",             &PROBCUT_FP_BASE,                164,     50,    300,  16.00, 0.002);
    spsa_add_int("PROBCUT_FP_MULT",             &PROBCUT_FP_MULT,                100,     20,    200,  10.00, 0.002);
    spsa_add_int("SPROBCUT_BETA_MARGIN",        &SPROBCUT_BETA_MARGIN,           350,    150,    600,  30.00, 0.002);

    // ── Futility & Razoring ──
    spsa_add_int("FP_MARGIN",                   &FP_MARGIN,                       82,     30,    150,   8.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_1",   &FUTILITY_PRUNING_OFFSET[1],      82,     30,    150,   8.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_2",   &FUTILITY_PRUNING_OFFSET[2],      41,     10,    100,   4.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_3",   &FUTILITY_PRUNING_OFFSET[3],      20,      5,     50,   2.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_4",   &FUTILITY_PRUNING_OFFSET[4],      10,      0,     30,   1.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_5",   &FUTILITY_PRUNING_OFFSET[5],       5,      0,     20,   1.00, 0.002);
    spsa_add_int("BNFP_MARGIN",                 &BNFP_MARGIN,                     71,     20,    150,   7.00, 0.002);
    spsa_add_int("QUIET_HISTORY_PRUNING_MARGIN",&QUIET_HISTORY_PRUNING_MARGIN,  2048,   1024,   4096, 200.00, 0.002);
    spsa_add_int("FP_HIST_MULT",                &FP_HIST_MULT,                   512,    128,   2048,  50.00, 0.002);
    spsa_add_int("FP_HIST_DIVISOR",             &FP_HIST_DIVISOR,              16384,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("LDSE_BASE_MARGIN",            &LDSE_BASE_MARGIN,                25,      5,     80,   4.00, 0.002);
    spsa_add_int("LDSE_CORRECTION_MULT",        &LDSE_CORRECTION_MULT,           768,    256,   2048,  40.00, 0.002);
    spsa_add_int("LDSE_CORRECTION_DIVISOR",     &LDSE_CORRECTION_DIVISOR,      98304,  49152, 196608, 1000.0, 0.002);
    
    spsa_add_int("RAZORING_FULL_MARGIN",        &RAZORING_FULL_MARGIN,           200,     80,    400,  15.00, 0.002);
    spsa_add_int("RAZORING_DEPTH_SCALE",        &RAZORING_DEPTH_SCALE,            15,      5,     40,   2.00, 0.002);
    spsa_add_int("RAZORING_VERIFY_MARGIN",      &RAZORING_VERIFY_MARGIN,         120,     40,    250,  12.00, 0.002);

    // ── History Bonuses ──
    spsa_add_int("QUIET_HIST_BONUS_BASE",       &QUIET_HIST_BONUS_BASE,           10,      0,     50,   2.00, 0.002);
    spsa_add_int("QUIET_HIST_BONUS_DEPTH",      &QUIET_HIST_BONUS_DEPTH,         200,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_BONUS_MAX",        &QUIET_HIST_BONUS_MAX,          4096,   1024,   8192, 200.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_BASE",       &QUIET_HIST_MALUS_BASE,           10,      0,     50,   2.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_DEPTH",      &QUIET_HIST_MALUS_DEPTH,         200,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_FAILED_LOW_BONUS", &QUIET_HIST_FAILED_LOW_BONUS,    200,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_FAILED_LOW_MALUS", &QUIET_HIST_FAILED_LOW_MALUS,    200,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_MAX",        &QUIET_HIST_MALUS_MAX,          4096,   1024,   8192, 200.00, 0.002);
    spsa_add_int("HISTORY_RED_MULT",            &HISTORY_RED_MULT,              1024,      0,   4096,  50.00, 0.002);
    spsa_add_int("HISTORY_RED_DIVISOR",         &HISTORY_RED_DIVISOR,       16777216,  8388608, 33554432, 100000.0, 0.002);
    spsa_add_int("CONTHIST_MULT",               &CONTHIST_MULT,                 1024,      0,   4096,  50.00, 0.002);

    spsa_add_int("CONTHIST_BONUS_BASE",         &CONTHIST_BONUS_BASE,             10,      0,     50,   2.00, 0.002);
    spsa_add_int("CONTHIST_BONUS_DEPTH",        &CONTHIST_BONUS_DEPTH,           200,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_BONUS_MAX",          &CONTHIST_BONUS_MAX,            4096,   1024,   8192, 200.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_BASE",         &CONTHIST_MALUS_BASE,             10,      0,     50,   2.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_DEPTH",        &CONTHIST_MALUS_DEPTH,           200,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_FAILED_LOW_BONUS",   &CONTHIST_FAILED_LOW_BONUS,      200,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_FAILED_LOW_MALUS",   &CONTHIST_FAILED_LOW_MALUS,      200,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_MAX",          &CONTHIST_MALUS_MAX,            4096,   1024,   8192, 200.00, 0.002);

    spsa_add_int("PAWNHIST_BONUS_BASE",         &PAWNHIST_BONUS_BASE,             10,      0,     50,   2.00, 0.002);
    spsa_add_int("PAWNHIST_BONUS_DEPTH",        &PAWNHIST_BONUS_DEPTH,           200,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_BONUS_MAX",          &PAWNHIST_BONUS_MAX,            4096,   1024,   8192, 200.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_BASE",         &PAWNHIST_MALUS_BASE,             10,      0,     50,   2.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_DEPTH",        &PAWNHIST_MALUS_DEPTH,           200,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_FAILED_LOW_BONUS",   &PAWNHIST_FAILED_LOW_BONUS,      200,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_FAILED_LOW_MALUS",   &PAWNHIST_FAILED_LOW_MALUS,      200,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_MAX",          &PAWNHIST_MALUS_MAX,            4096,   1024,   8192, 200.00, 0.002);

    spsa_add_int("CAPTHIST_BONUS_BASE",         &CAPTHIST_BONUS_BASE,             10,      0,     50,   2.00, 0.002);
    spsa_add_int("CAPTHIST_BONUS_DEPTH",        &CAPTHIST_BONUS_DEPTH,           200,     50,    500,  20.00, 0.002);
    spsa_add_int("CAPTHIST_BONUS_MAX",          &CAPTHIST_BONUS_MAX,            4096,   1024,   8192, 200.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_BASE",         &CAPTHIST_MALUS_BASE,             10,      0,     50,   2.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_DEPTH",        &CAPTHIST_MALUS_DEPTH,           200,     50,    500,  20.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_MAX",          &CAPTHIST_MALUS_MAX,            4096,   1024,   8192, 200.00, 0.002);

    spsa_add_int("BAD_QUIET_INDEX_SCALE",       &BAD_QUIET_INDEX_SCALE,           30,      5,     80,   3.00, 0.002);

    // ── Correction History ──
    spsa_add_int("PAWN_CORRHIST_WEIGHT_SCALE",       &PAWN_CORRHIST_WEIGHT_SCALE,          256,     64,    512,  25.00, 0.002);
    spsa_add_int("PAWN_CORRHIST_GRAIN",              &PAWN_CORRHIST_GRAIN,                 256,     64,    512,  25.00, 0.002);
    
    spsa_add_int("MINOR_CORRHIST_WEIGHT_SCALE",      &MINOR_CORRHIST_WEIGHT_SCALE,         256,     64,    512,  25.00, 0.002);
    spsa_add_int("MINOR_CORRHIST_GRAIN",             &MINOR_CORRHIST_GRAIN,                256,     64,    512,  25.00, 0.002);
    
    spsa_add_int("MAJOR_CORRHIST_WEIGHT_SCALE",      &MAJOR_CORRHIST_WEIGHT_SCALE,         256,     64,    512,  25.00, 0.002);
    spsa_add_int("MAJOR_CORRHIST_GRAIN",             &MAJOR_CORRHIST_GRAIN,                256,     64,    512,  25.00, 0.002);
    
    spsa_add_int("NON_PAWN_CORRHIST_WEIGHT_SCALE",   &NON_PAWN_CORRHIST_WEIGHT_SCALE,      256,     64,    512,  25.00, 0.002);
    spsa_add_int("NON_PAWN_CORRHIST_GRAIN",          &NON_PAWN_CORRHIST_GRAIN,             256,     64,    512,  25.00, 0.002);
    
    spsa_add_int("KRP_CORRHIST_WEIGHT_SCALE",        &KRP_CORRHIST_WEIGHT_SCALE,           256,     64,    512,  25.00, 0.002);
    spsa_add_int("KRP_CORRHIST_GRAIN",               &KRP_CORRHIST_GRAIN,                  256,     64,    512,  25.00, 0.002);
    
    spsa_add_int("CONT_CORRHIST_WEIGHT_SCALE",       &CONT_CORRHIST_WEIGHT_SCALE,          256,     64,    512,  25.00, 0.002);
    spsa_add_int("CONT_CORRHIST_GRAIN",              &CONT_CORRHIST_GRAIN,                 256,     64,    512,  25.00, 0.002);

    // ── Time Management ──
    spsa_add_double("DEF_TIME_MULTIPLIER",      &DEF_TIME_MULTIPLIER,          0.054,  0.020,  0.120,  0.005, 0.002);
    spsa_add_double("DEF_INC_MULTIPLIER",       &DEF_INC_MULTIPLIER,           0.850,  0.400,  1.500,  0.050, 0.002);
    spsa_add_double("MAX_TIME_MULTIPLIER",      &MAX_TIME_MULTIPLIER,          0.760,  0.300,  1.500,  0.050, 0.002);
    spsa_add_double("HARD_LIMIT_MULTIPLIER",    &HARD_LIMIT_MULTIPLIER,        3.040,  1.500,  5.000,  0.150, 0.002);
    spsa_add_double("SOFT_LIMIT_MULTIPLIER",    &SOFT_LIMIT_MULTIPLIER,        0.760,  0.300,  1.500,  0.050, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_0",     &TM_BEST_MOVE_SCALE_0,         2.43,   1.50,   3.50,  0.20, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_1",     &TM_BEST_MOVE_SCALE_1,         1.35,   0.80,   2.00,  0.10, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_2",     &TM_BEST_MOVE_SCALE_2,         1.09,   0.50,   1.50,  0.10, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_3",     &TM_BEST_MOVE_SCALE_3,         0.88,   0.40,   1.20,  0.08, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_4",     &TM_BEST_MOVE_SCALE_4,         0.68,   0.20,   1.00,  0.08, 0.002);
    spsa_add_double("TM_EVAL_SCALE_0",          &TM_EVAL_SCALE_0,              1.25,   0.80,   1.80,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_1",          &TM_EVAL_SCALE_1,              1.15,   0.70,   1.60,  0.09, 0.002);
    spsa_add_double("TM_EVAL_SCALE_2",          &TM_EVAL_SCALE_2,              1.00,   0.50,   1.50,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_3",          &TM_EVAL_SCALE_3,              0.94,   0.40,   1.40,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_4",          &TM_EVAL_SCALE_4,              0.88,   0.40,   1.40,  0.10, 0.002);
    spsa_add_double("TM_COMPLEXITY_BASE",       &TM_COMPLEXITY_BASE,           0.77,   0.30,   1.20,  0.08, 0.002);
    spsa_add_double("TM_COMPLEXITY_DIVISOR",    &TM_COMPLEXITY_DIVISOR,      400.00, 200.00, 800.00, 40.00, 0.002);
    spsa_add_double("TM_COMPLEXITY_MULT",       &TM_COMPLEXITY_MULT,           0.60,   0.20,   1.50,  0.10, 0.002);
    spsa_add_double("TM_NODE_FRACTION_BASE",    &TM_NODE_FRACTION_BASE,        1.50,   1.00,   2.50,  0.15, 0.002);
    spsa_add_double("TM_NODE_MULTIPLIER",       &TM_NODE_MULTIPLIER,           1.35,   0.80,   2.20,  0.10, 0.002);

    printf("info string SPSA: %d parameters registered\n", spsa_count);
}


void spsa_print_uci_options(void) {
    if (!SPSA_MODE) return;
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
    if (!SPSA_MODE) return 0;
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

            // update LMR table
            initializeLMRTable();
            return 1;
        }
    }
    return 0;
}


void spsa_print_params(void) {
    if (!SPSA_MODE) return;
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
