#ifndef POTENTIAL_TIME_H
#define POTENTIAL_TIME_H

#pragma once

#include "structs.h"
#include "threads.h"
#include "values.h"

#ifdef WIN64

#include <windows.h>
#include <stdbool.h>

#else
#include <sys/time.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>


#endif

void initTimeControl(my_time* time);

int getTimeMiliSecond();
int input_waiting();
void check_time_limit(my_time *time, int startTime, ThreadData *t, int score);



#endif //POTENTIAL_TIME_H
