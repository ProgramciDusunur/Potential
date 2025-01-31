//
// Created by erena on 31.01.2025.
//

#include "utils.h"


int clamp(int d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}

int myMAX(int x, int y) {
    return (x > y) ? x : y;
}

int myMIN(int x, int y) {
    return (x < y) ? x : y;
}
