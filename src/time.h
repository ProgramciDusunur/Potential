//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_TIME_H
#define POTENTIAL_TIME_H

#pragma once

#include "structs.h"

#ifdef WIN64

#include <windows.h>
#include <stdbool.h>

#else
#include <sys/time.h>
#endif

void initTimeControl(time* time);
void resetTimeControl(time* time);
int getTimeMiliSecond();
int input_waiting();



#endif //POTENTIAL_TIME_H
