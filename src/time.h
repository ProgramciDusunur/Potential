//
// Created by erena on 29.05.2024.
//
#pragma once
#ifndef CHESSENGINE_TIME_H
#define CHESSENGINE_TIME_H

#endif //CHESSENGINE_TIME_H

#ifdef WIN64

#include <windows.h>
#include <stdbool.h>

#else
#include <sys/time.h>
#endif

void resetTimeControl();
int getTimeMiliSecond();
