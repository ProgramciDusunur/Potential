//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_BENCH_H
#define POTENTIAL_BENCH_H

#pragma once

#include "bit_manipluation.h"
#include <stdio.h>
#include "search.h"
#include "time.h"
#include "fen.h"

extern char* benchmarkfens[52];
void benchmark(int depth, board* position, time* time);

#endif //POTENTIAL_BENCH_H
