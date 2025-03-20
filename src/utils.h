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

int clamp(int d, int min, int max);
void pBoard(board* position);
int clamp(const int d, const int min, const int max);
double clampDecimalValue(double d, const double min, const double max);

#endif //POTENTIAL_UTILS_H
