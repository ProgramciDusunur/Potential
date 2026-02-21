//
// Created by erena on 2.02.2025.
//

#ifndef POTENTIAL_UTILS_H
#define POTENTIAL_UTILS_H

#include "structs.h"
#include "board_constants.h"
#include "bit_manipulation.h"
#include "values.h"

// Relaxed atomic helpers
#define load_rlx(x) atomic_load_explicit(&(x), memory_order_relaxed)
#define inc_rlx(x)  atomic_fetch_add_explicit(&(x), 1, memory_order_relaxed)
#define dec_rlx(x)  atomic_fetch_sub_explicit(&(x), 1, memory_order_relaxed)
#define store_rlx(x, v) atomic_store_explicit(&(x), (v), memory_order_relaxed)


bool is_mate_score(int score);
int myMAX(int x, int y);
int myMIN(int x, int y);
double my_max_double(double x, double y);
double my_min_double(double x, double y);
double clamp_double(const double d, const double min, const double max);
void pBoard(board* position);
void printMailbox(const board *position);
int clamp(const int d, const int min, const int max);

#endif //POTENTIAL_UTILS_H
