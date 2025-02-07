//
// Created by erena on 2.02.2025.
//

#include "utils.h"


int myMAX(int x, int y) {
    return (x > y) ? x : y;
}

int myMIN(int x, int y) {
    return (x < y) ? x : y;
}

int clamp(int d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}
