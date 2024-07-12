//
// Created by erena on 29.05.2024.
//
#pragma once

#ifdef WIN64

#include <windows.h>
#include <stdbool.h>

#else
#include <sys/time.h>
#endif

// UCI "movestogo" command moves counter
extern int movestogo;

// UCI "movetime" command time counter
extern int movetime;

// UCI "time" command holder (ms)
extern int time;

// UCI "inc" command's time increment holder
extern int inc;

// UCI "starttime" command time holder
extern int starttime;

// UCI "stoptime" command time holder
extern int stoptime;

// variable to flag time control availability
extern int timeset;

// variable to flag when the time is up
extern int stopped;

// exit from engine flag
extern int quit;

void resetTimeControl();
int getTimeMiliSecond();
