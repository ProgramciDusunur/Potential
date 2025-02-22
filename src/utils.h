//
// Created by erena on 2.02.2025.
//

#ifndef POTENTIAL_UTILS_H
#define POTENTIAL_UTILS_H
#include "structs.h"
#include "board_constants.h"
#include <stdio.h>

void printMailbox(board *position);
int myMAX(int x, int y);
int myMIN(int x, int y);
int clamp(int d, int min, int max);

#endif //POTENTIAL_UTILS_H
