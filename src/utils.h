//
// Created by erena on 2.02.2025.
//

#ifndef POTENTIAL_UTILS_H
#define POTENTIAL_UTILS_H

#include "structs.h"
#include "board_constants.h"
#include "bit_manipulation.h"

int myMAX(int x, int y);
int myMIN(int x, int y);
double my_max_double(double x, double y);
double my_min_double(double x, double y);
double clamp_double(const double d, const double min, const double max);
void pBoard(board* position);
void printMailbox(const board *position);
int clamp(const int d, const int min, const int max);

#endif //POTENTIAL_UTILS_H
