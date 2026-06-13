#include "spsa.h"
#include "search.h"


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
extern TUNE_INT NMP_EVAL_BETA_MARGIN;
extern TUNE_INT NMP_VERIFICATION_MARGIN;
extern TUNE_INT NMP_REDUCTION_DIVISOR;
extern TUNE_INT NMP_EVAL_DIVISOR;
extern TUNE_INT RFP_CORRPLEXITY_MULT;
extern TUNE_INT ASP_WINDOW_BASE;
extern TUNE_DOUBLE ASP_WINDOW_MULTIPLIER;
extern TUNE_DOUBLE LMR_TABLE_BASE_NOISY;
extern TUNE_DOUBLE LMR_TABLE_NOISY_MULT;
extern TUNE_DOUBLE LMR_TABLE_BASE_QUIET;
extern TUNE_DOUBLE LMR_TABLE_QUIET_MULT;

// SEE
extern TUNE_INT QS_SEE_THRESHOLD;
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
extern TUNE_INT LMR_DEPTH_HIST_MULT;



// Probcut
extern TUNE_INT PROBCUT_BETA_MARGIN;
extern TUNE_INT PROBCUT_IMPROVING_MARGIN;
extern TUNE_INT PROBCUT_SEE_NOISY_THRESHOLD;
extern TUNE_INT PROBCUT_NOISY_HIST_MULT;
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
    spsa_add_int("RFP_MARGIN",                  &RFP_MARGIN,                53,     20,    100,   5.00, 0.002);
    spsa_add_int("RFP_IMPROVING_MARGIN",        &RFP_IMPROVING_MARGIN,      48,     15,     80,   4.00, 0.002);
    spsa_add_int("RFP_CORRPLEXITY_MULT",        &RFP_CORRPLEXITY_MULT,      23,      5,     50,   4.00, 0.002);

    // ── Null Move Pruning ──
    spsa_add_int("NMP_BASE_REDUCTION",          &NMP_BASE_REDUCTION,      5077,   3000,   7000,  50.00, 0.002);
    spsa_add_int("NMP_DEPTH_MULTIPLIER",        &NMP_DEPTH_MULTIPLIER,     245,    128,    512,  25.00, 0.002);
    spsa_add_int("NMP_REDUCTION_DEPTH_MULT",    &NMP_REDUCTION_DEPTH_MULT,  18,      4,     64,   3.00, 0.002);
    spsa_add_int("NMP_EVAL_MULT",               &NMP_EVAL_MULT,            95,     32,    256,  12.00, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_BASE",   &NMP_FAILED_HIGH_HIST_BASE,110,     50,    200,  10.00, 0.002);
    spsa_add_int("NMP_FAILED_HIGH_HIST_MULT",   &NMP_FAILED_HIGH_HIST_MULT, 60,     10,    100,   5.00, 0.002);
    spsa_add_int("NMP_EVAL_BETA_MARGIN",        &NMP_EVAL_BETA_MARGIN,      96,     20,    150,  10.00, 0.002);
    spsa_add_int("NMP_VERIFICATION_MARGIN",     &NMP_VERIFICATION_MARGIN,   40,     10,     80,   5.00, 0.002);
    spsa_add_int("NMP_REDUCTION_DIVISOR",       &NMP_REDUCTION_DIVISOR,  17119,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("NMP_EVAL_DIVISOR",            &NMP_EVAL_DIVISOR,       42002,  25600, 102400, 5000.0, 0.002);

    // ── Aspiration Windows ──
    spsa_add_int("ASP_WINDOW_BASE",             &ASP_WINDOW_BASE,            3,      3,     25,   2.00, 0.002);
    spsa_add_double("ASP_WINDOW_MULTIPLIER",    &ASP_WINDOW_MULTIPLIER,    2.2156573076828328,    1.2,    3.0,   0.15, 0.002);

    // ── LMR Table Parameters ──
    spsa_add_double("LMR_TABLE_BASE_NOISY",     &LMR_TABLE_BASE_NOISY,    0.42707829406943654,   0.10,   1.00,  0.03, 0.002);
    spsa_add_double("LMR_TABLE_NOISY_MULT",     &LMR_TABLE_NOISY_MULT,  131.61636345641526,  64.00, 256.00,  8.00, 0.002);
    spsa_add_double("LMR_TABLE_BASE_QUIET",     &LMR_TABLE_BASE_QUIET,    0.968544856385441,   0.50,   2.00,  0.05, 0.002);
    spsa_add_double("LMR_TABLE_QUIET_MULT",     &LMR_TABLE_QUIET_MULT,  144.99523678587622,  64.00, 256.00,  8.00, 0.002);

    // ── SEE ──
    spsa_add_int("QS_SEE_THRESHOLD",            &QS_SEE_THRESHOLD,            4,   -50,     50,   5.00, 0.002);
    spsa_add_int("SEE_MOVE_ORDERING_THRESHOLD", &SEE_MOVE_ORDERING_THRESHOLD,-60, -150,      0,   8.00, 0.002);
    spsa_add_int("SEE_QUIET_THRESHOLD",         &SEE_QUIET_THRESHOLD,       -71,  -120,    -20,   5.00, 0.002);
    spsa_add_int("SEE_NOISY_THRESHOLD",         &SEE_NOISY_THRESHOLD,       -40,   -80,      0,   4.00, 0.002);
    spsa_add_int("MOVE_ORDER_HIST_MULT",        &MOVE_ORDER_HIST_MULT,      350,   128,   1024,  40.00, 0.002);
    spsa_add_int("SEE_PRUNING_HIST_MULT",       &SEE_PRUNING_HIST_MULT,     558,   128,   1024,  40.00, 0.002);
    spsa_add_int("SEE_QUIET_HIST_MULT",         &SEE_QUIET_HIST_MULT,       111,    32,    512,  15.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_PAWN",        &SEE_PIECE_VALUES[0],       99,    50,    200,  10.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_KNIGHT",      &SEE_PIECE_VALUES[1],       251,   150,    500,  30.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_BISHOP",      &SEE_PIECE_VALUES[2],       323,   150,    500,  30.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_ROOK",        &SEE_PIECE_VALUES[3],       532,   300,    800,  50.00, 0.002);
    spsa_add_int("SEE_PIECE_VALUE_QUEEN",       &SEE_PIECE_VALUES[4],      1466,   800,   1800, 120.00, 0.002);

    // ── LMR Scalars ──
    spsa_add_int("DEEPER_LMR_MARGIN",                 &DEEPER_LMR_MARGIN,               40,     10,     80,   4.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MULT",            &QUIET_HISTORY_LMR_MULT,           4,      1,     16,   1.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_DIVISOR",         &QUIET_HISTORY_LMR_DIVISOR,  18088,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MINIMUM_SCALAR",  &QUIET_HISTORY_LMR_MINIMUM_SCALAR, 3335, 1024, 6144, 100.00, 0.002);
    spsa_add_int("QUIET_HISTORY_LMR_MAXIMUM_SCALAR",  &QUIET_HISTORY_LMR_MAXIMUM_SCALAR, 3151, 1024, 6144, 100.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MULT",             &PAWN_HISTORY_LMR_MULT,            4,      1,     16,   1.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_DIVISOR",          &PAWN_HISTORY_LMR_DIVISOR,   18305,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MINIMUM_SCALAR",   &PAWN_HISTORY_LMR_MINIMUM_SCALAR, 2985, 1024, 6144, 100.00, 0.002);
    spsa_add_int("PAWN_HISTORY_LMR_MAXIMUM_SCALAR",   &PAWN_HISTORY_LMR_MAXIMUM_SCALAR, 3211, 1024, 6144, 100.00, 0.002);
    spsa_add_int("NOISY_HISTORY_LMR_MULT",            &NOISY_HISTORY_LMR_MULT,         103,     32,    512,  15.00, 0.002);
    spsa_add_int("NOISY_HISTORY_LMR_DIVISOR",         &NOISY_HISTORY_LMR_DIVISOR,  1314248, 655360, 2621440, 10000.0, 0.002);
    spsa_add_int("QUIET_NON_PV_LMR_SCALAR",           &QUIET_NON_PV_LMR_SCALAR,       907,    256,   2048,  50.00, 0.002);
    spsa_add_int("CUT_NODE_LMR_SCALAR",               &CUT_NODE_LMR_SCALAR,           2030,    512,   4096, 100.00, 0.002);
    spsa_add_int("TT_PV_LMR_SCALAR",                  &TT_PV_LMR_SCALAR,              1021,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_PV_FAIL_LOW_LMR_SCALAR",         &TT_PV_FAIL_LOW_LMR_SCALAR,     1077,    256,   2048,  50.00, 0.002);
    spsa_add_int("TT_CAPTURE_LMR_SCALAR",             &TT_CAPTURE_LMR_SCALAR,         897,    256,   2048,  50.00, 0.002);
    spsa_add_int("GOOD_EVAL_LMR_SCALAR",              &GOOD_EVAL_LMR_SCALAR,          835,    256,   2048,  50.00, 0.002);
    spsa_add_int("GOOD_EVAL_LMR_MARGIN",              &GOOD_EVAL_LMR_MARGIN,           229,    150,    600,  30.00, 0.002);
    spsa_add_int("FUTILITY_LMR_BASE",                 &FUTILITY_LMR_BASE,              149,     50,    300,  15.00, 0.002);
    spsa_add_int("FUTILITY_LMR_MULT",                 &FUTILITY_LMR_MULT,               87,     30,    150,  10.00, 0.002);
    spsa_add_int("FUTILITY_LMR_SCALAR",               &FUTILITY_LMR_SCALAR,           845,    256,   2048,  50.00, 0.002);
    spsa_add_int("IMPROVING_LMR_SCALAR",              &IMPROVING_LMR_SCALAR,          1211,    256,   2048,  50.00, 0.002);
    spsa_add_int("IMPROVING_FAIL_HIGH_MARGIN",        &IMPROVING_FAIL_HIGH_MARGIN,     108,     50,    200,  15.00, 0.002);
    spsa_add_int("GIVES_CHECK_LMR_SCALAR",            &GIVES_CHECK_LMR_SCALAR,        1171,    256,   2048,  50.00, 0.002);
    spsa_add_int("LMR_DEPTH_HIST_MULT",               &LMR_DEPTH_HIST_MULT,           2032,   1024,   8192,  50.00, 0.002);
    spsa_add_int("LMP_HIST_MULT",                     &LMP_HIST_MULT,                  236,     64,   1024,  25.00, 0.002);
    spsa_add_int("LMP_HIST_DIVISOR",                  &LMP_HIST_DIVISOR,             15547,   8192,  32768, 1000.0, 0.002);



    // ── Probcut ──
    spsa_add_int("PROBCUT_BETA_MARGIN",         &PROBCUT_BETA_MARGIN,            147,     50,    300,  15.00, 0.002);
    spsa_add_int("PROBCUT_IMPROVING_MARGIN",    &PROBCUT_IMPROVING_MARGIN,        37,      0,     80,   4.00, 0.002);
    spsa_add_int("PROBCUT_SEE_NOISY_THRESHOLD", &PROBCUT_SEE_NOISY_THRESHOLD,    122,      0,    250,  10.00, 0.002);
    spsa_add_int("PROBCUT_NOISY_HIST_MULT",     &PROBCUT_NOISY_HIST_MULT,        106,     32,    512,  15.00, 0.002);
    spsa_add_int("PROBCUT_FP_BASE",             &PROBCUT_FP_BASE,                185,     50,    300,  16.00, 0.002);
    spsa_add_int("PROBCUT_FP_MULT",             &PROBCUT_FP_MULT,                109,     20,    200,  10.00, 0.002);
    spsa_add_int("SPROBCUT_BETA_MARGIN",        &SPROBCUT_BETA_MARGIN,           354,    150,    600,  30.00, 0.002);

    // ── Futility & Razoring ──
    spsa_add_int("FP_MARGIN",                   &FP_MARGIN,                       75,     30,    150,   8.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_1",   &FUTILITY_PRUNING_OFFSET[1],      71,     30,    150,   8.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_2",   &FUTILITY_PRUNING_OFFSET[2],      41,     10,    100,   4.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_3",   &FUTILITY_PRUNING_OFFSET[3],      26,      5,     50,   2.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_4",   &FUTILITY_PRUNING_OFFSET[4],      11,      0,     30,   1.00, 0.002);
    spsa_add_int("FUTILITY_PRUNING_OFFSET_5",   &FUTILITY_PRUNING_OFFSET[5],       6,      0,     20,   1.00, 0.002);
    spsa_add_int("BNFP_MARGIN",                 &BNFP_MARGIN,                     72,     20,    150,   7.00, 0.002);
    spsa_add_int("QUIET_HISTORY_PRUNING_MARGIN",&QUIET_HISTORY_PRUNING_MARGIN,  2078,   1024,   4096, 200.00, 0.002);
    spsa_add_int("FP_HIST_MULT",                &FP_HIST_MULT,                   402,    128,   2048,  50.00, 0.002);
    spsa_add_int("FP_HIST_DIVISOR",             &FP_HIST_DIVISOR,              15440,   8192,  32768, 1000.0, 0.002);
    spsa_add_int("LDSE_BASE_MARGIN",            &LDSE_BASE_MARGIN,                19,      5,     80,   4.00, 0.002);
    spsa_add_int("LDSE_CORRECTION_MULT",        &LDSE_CORRECTION_MULT,           787,    256,   2048,  40.00, 0.002);
    spsa_add_int("LDSE_CORRECTION_DIVISOR",     &LDSE_CORRECTION_DIVISOR,      99350,  49152, 196608, 1000.0, 0.002);
    
    spsa_add_int("RAZORING_FULL_MARGIN",        &RAZORING_FULL_MARGIN,           242,     80,    400,  15.00, 0.002);
    spsa_add_int("RAZORING_DEPTH_SCALE",        &RAZORING_DEPTH_SCALE,            11,      5,     40,   2.00, 0.002);
    spsa_add_int("RAZORING_VERIFY_MARGIN",      &RAZORING_VERIFY_MARGIN,         143,     40,    250,  12.00, 0.002);

    // ── History Bonuses ──
    spsa_add_int("QUIET_HIST_BONUS_BASE",       &QUIET_HIST_BONUS_BASE,           7,      0,     50,   2.00, 0.002);
    spsa_add_int("QUIET_HIST_BONUS_DEPTH",      &QUIET_HIST_BONUS_DEPTH,         194,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_BONUS_MAX",        &QUIET_HIST_BONUS_MAX,          4297,   1024,   8192, 200.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_BASE",       &QUIET_HIST_MALUS_BASE,           11,      0,     50,   2.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_DEPTH",      &QUIET_HIST_MALUS_DEPTH,         141,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_FAILED_LOW_BONUS", &QUIET_HIST_FAILED_LOW_BONUS,    224,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_FAILED_LOW_MALUS", &QUIET_HIST_FAILED_LOW_MALUS,    229,     50,    500,  20.00, 0.002);
    spsa_add_int("QUIET_HIST_MALUS_MAX",        &QUIET_HIST_MALUS_MAX,          3828,   1024,   8192, 200.00, 0.002);
    spsa_add_int("HISTORY_RED_MULT",            &HISTORY_RED_MULT,              1013,      0,   4096,  50.00, 0.002);
    spsa_add_int("HISTORY_RED_DIVISOR",         &HISTORY_RED_DIVISOR,       16886493,  8388608, 33554432, 100000.0, 0.002);
    spsa_add_int("CONTHIST_MULT",               &CONTHIST_MULT,                 1087,      0,   4096,  50.00, 0.002);

    spsa_add_int("CONTHIST_BONUS_BASE",         &CONTHIST_BONUS_BASE,             6,      0,     50,   2.00, 0.002);
    spsa_add_int("CONTHIST_BONUS_DEPTH",        &CONTHIST_BONUS_DEPTH,           120,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_BONUS_MAX",          &CONTHIST_BONUS_MAX,            3505,   1024,   8192, 200.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_BASE",         &CONTHIST_MALUS_BASE,             11,      0,     50,   2.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_DEPTH",        &CONTHIST_MALUS_DEPTH,           208,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_FAILED_LOW_BONUS",   &CONTHIST_FAILED_LOW_BONUS,      217,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_FAILED_LOW_MALUS",   &CONTHIST_FAILED_LOW_MALUS,      204,     50,    500,  20.00, 0.002);
    spsa_add_int("CONTHIST_MALUS_MAX",          &CONTHIST_MALUS_MAX,            4274,   1024,   8192, 200.00, 0.002);

    spsa_add_int("PAWNHIST_BONUS_BASE",         &PAWNHIST_BONUS_BASE,             11,      0,     50,   2.00, 0.002);
    spsa_add_int("PAWNHIST_BONUS_DEPTH",        &PAWNHIST_BONUS_DEPTH,           159,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_BONUS_MAX",          &PAWNHIST_BONUS_MAX,            3996,   1024,   8192, 200.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_BASE",         &PAWNHIST_MALUS_BASE,             7,      0,     50,   2.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_DEPTH",        &PAWNHIST_MALUS_DEPTH,           235,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_FAILED_LOW_BONUS",   &PAWNHIST_FAILED_LOW_BONUS,      188,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_FAILED_LOW_MALUS",   &PAWNHIST_FAILED_LOW_MALUS,      198,     50,    500,  20.00, 0.002);
    spsa_add_int("PAWNHIST_MALUS_MAX",          &PAWNHIST_MALUS_MAX,            4177,   1024,   8192, 200.00, 0.002);

    spsa_add_int("CAPTHIST_BONUS_BASE",         &CAPTHIST_BONUS_BASE,             12,      0,     50,   2.00, 0.002);
    spsa_add_int("CAPTHIST_BONUS_DEPTH",        &CAPTHIST_BONUS_DEPTH,           224,     50,    500,  20.00, 0.002);
    spsa_add_int("CAPTHIST_BONUS_MAX",          &CAPTHIST_BONUS_MAX,            4101,   1024,   8192, 200.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_BASE",         &CAPTHIST_MALUS_BASE,             13,      0,     50,   2.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_DEPTH",        &CAPTHIST_MALUS_DEPTH,           297,     50,    500,  20.00, 0.002);
    spsa_add_int("CAPTHIST_MALUS_MAX",          &CAPTHIST_MALUS_MAX,            4351,   1024,   8192, 200.00, 0.002);

    spsa_add_int("BAD_QUIET_INDEX_SCALE",       &BAD_QUIET_INDEX_SCALE,           19,      5,     80,   3.00, 0.002);

    // ── Correction History ──
    spsa_add_int("PAWN_CORRHIST_WEIGHT_SCALE",       &PAWN_CORRHIST_WEIGHT_SCALE,          215,     64,    512,  25.00, 0.002);
    spsa_add_int("PAWN_CORRHIST_GRAIN",              &PAWN_CORRHIST_GRAIN,                 303,     64,    512,  25.00, 0.002);
    
    spsa_add_int("MINOR_CORRHIST_WEIGHT_SCALE",      &MINOR_CORRHIST_WEIGHT_SCALE,         302,     64,    512,  25.00, 0.002);
    spsa_add_int("MINOR_CORRHIST_GRAIN",             &MINOR_CORRHIST_GRAIN,                267,     64,    512,  25.00, 0.002);
    
    spsa_add_int("MAJOR_CORRHIST_WEIGHT_SCALE",      &MAJOR_CORRHIST_WEIGHT_SCALE,         279,     64,    512,  25.00, 0.002);
    spsa_add_int("MAJOR_CORRHIST_GRAIN",             &MAJOR_CORRHIST_GRAIN,                267,     64,    512,  25.00, 0.002);
    
    spsa_add_int("NON_PAWN_CORRHIST_WEIGHT_SCALE",   &NON_PAWN_CORRHIST_WEIGHT_SCALE,      267,     64,    512,  25.00, 0.002);
    spsa_add_int("NON_PAWN_CORRHIST_GRAIN",          &NON_PAWN_CORRHIST_GRAIN,             333,     64,    512,  25.00, 0.002);
    
    spsa_add_int("KRP_CORRHIST_WEIGHT_SCALE",        &KRP_CORRHIST_WEIGHT_SCALE,           237,     64,    512,  25.00, 0.002);
    spsa_add_int("KRP_CORRHIST_GRAIN",               &KRP_CORRHIST_GRAIN,                  269,     64,    512,  25.00, 0.002);
    
    spsa_add_int("CONT_CORRHIST_WEIGHT_SCALE",       &CONT_CORRHIST_WEIGHT_SCALE,          259,     64,    512,  25.00, 0.002);
    spsa_add_int("CONT_CORRHIST_GRAIN",              &CONT_CORRHIST_GRAIN,                 223,     64,    512,  25.00, 0.002);

    // ── Time Management ──
    spsa_add_double("DEF_TIME_MULTIPLIER",      &DEF_TIME_MULTIPLIER,          0.055215030184613524,  0.020,  0.120,  0.005, 0.002);
    spsa_add_double("DEF_INC_MULTIPLIER",       &DEF_INC_MULTIPLIER,           0.6650512210404446,  0.400,  1.500,  0.050, 0.002);
    spsa_add_double("MAX_TIME_MULTIPLIER",      &MAX_TIME_MULTIPLIER,          0.7171636044985824,  0.300,  1.500,  0.050, 0.002);
    spsa_add_double("HARD_LIMIT_MULTIPLIER",    &HARD_LIMIT_MULTIPLIER,        3.0808332782345405,  1.500,  5.000,  0.150, 0.002);
    spsa_add_double("SOFT_LIMIT_MULTIPLIER",    &SOFT_LIMIT_MULTIPLIER,        0.812239212923424,  0.300,  1.500,  0.050, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_0",     &TM_BEST_MOVE_SCALE_0,         2.4903184885170844,   1.50,   3.50,  0.20, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_1",     &TM_BEST_MOVE_SCALE_1,         1.0283219425471126,   0.80,   2.00,  0.10, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_2",     &TM_BEST_MOVE_SCALE_2,         0.8454599914620153,   0.50,   1.50,  0.10, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_3",     &TM_BEST_MOVE_SCALE_3,         0.9754290474880251,   0.40,   1.20,  0.08, 0.002);
    spsa_add_double("TM_BEST_MOVE_SCALE_4",     &TM_BEST_MOVE_SCALE_4,         0.9008432419124114,   0.20,   1.00,  0.08, 0.002);
    spsa_add_double("TM_EVAL_SCALE_0",          &TM_EVAL_SCALE_0,              1.400974291622475,   0.80,   1.80,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_1",          &TM_EVAL_SCALE_1,              1.1567921525834612,   0.70,   1.60,  0.09, 0.002);
    spsa_add_double("TM_EVAL_SCALE_2",          &TM_EVAL_SCALE_2,              0.9919680953215663,   0.50,   1.50,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_3",          &TM_EVAL_SCALE_3,              0.9589504668299684,   0.40,   1.40,  0.10, 0.002);
    spsa_add_double("TM_EVAL_SCALE_4",          &TM_EVAL_SCALE_4,              0.6171668022949585,   0.40,   1.40,  0.10, 0.002);
    spsa_add_double("TM_COMPLEXITY_BASE",       &TM_COMPLEXITY_BASE,           0.9334744808131928,   0.30,   1.20,  0.08, 0.002);
    spsa_add_double("TM_COMPLEXITY_DIVISOR",    &TM_COMPLEXITY_DIVISOR,      347.64730841522703, 200.00, 800.00, 40.00, 0.002);
    spsa_add_double("TM_COMPLEXITY_MULT",       &TM_COMPLEXITY_MULT,           0.6562358649130683,   0.20,   1.50,  0.10, 0.002);
    spsa_add_double("TM_NODE_FRACTION_BASE",    &TM_NODE_FRACTION_BASE,        1.4295223442789158,   1.00,   2.50,  0.15, 0.002);
    spsa_add_double("TM_NODE_MULTIPLIER",       &TM_NODE_MULTIPLIER,           1.3227801253003622,   0.80,   2.20,  0.10, 0.002);

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

            // update LMR table
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
