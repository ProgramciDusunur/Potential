#ifndef POTENTIAL_SPSA_H
#define POTENTIAL_SPSA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// 0 = Normal Test, 1 = SPSA Tuning
#define SPSA_ACTIVE 1

#if SPSA_ACTIVE
    #define TUNE_INT int
    #define TUNE_DOUBLE double
#else
    #define TUNE_INT const int
    #define TUNE_DOUBLE const double
#endif

#define MAX_SPSA_PARAMS 256

typedef struct {
    const char *name;
    int         is_double;   // 0 = int, 1 = double
    void       *ptr;
    double      def;
    double      min;
    double      max;
    double      c_end;       // SPSA perturbation step
    double      r_end;       // SPSA learning rate
} SPSAParam;

extern SPSAParam spsa_params[];
extern int spsa_count;

void spsa_init(void);
void spsa_print_uci_options(void);
int  spsa_set_option(const char *input);
void spsa_print_params(void);

#endif // POTENTIAL_SPSA_H
