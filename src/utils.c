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

int clamp(const int d, const int min, const int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}

double clampDecimalValue(double d, const double min, const double max) {
    const double t = d < min ? min : d;
    return t > max ? max : t;
}
