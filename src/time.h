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
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>


#endif

void initTimeControl(timeControl* time);
void resetTimeControl(timeControl* time);
int getTimeMiliSecond();
int input_waiting();



#endif //POTENTIAL_TIME_H
