//
// Created by erena on 29.05.2024.
//

#include "time.h"

/**********************************\
 ==================================

       Time controls variables

 ==================================
\**********************************/

// exit from engine flag
int quit = 0;

// UCI "movestogo" command moves counter
int movestogo = 30;

// UCI "movetime" command time counter
int movetime = -1;

// UCI "time" command holder (ms)
int time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;

// reset time control variables
void resetTimeControl() {
    // reset timing
    quit = 0;
    movestogo = 30;
    movetime = -1;
    time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}

int getTimeMiliSecond() {
#ifdef WIN64
    return GetTickCount();
#else
    struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}
