//
// Created by erena on 13.09.2024.
//

#include "time.h"


void initTimeControl(timeControl* time) {
    // init timing
    time->quit = 0;
    time->movestogo = 20;
    time->movetime = -1;
    time->time = -1;
    time->inc = 0;
    time->starttime = 0;
    time->stoptime = 0;
    time->timeset = 0;
    time->stopped = 0;
    time->softLimit = 0;
    time->hardLimit = 0;
}


// reset timeControl control variables
void resetTimeControl(timeControl* time) {
    // reset timing
    time->quit = 0;
    time->movestogo = 20;
    time->movetime = -1;
    time->time = -1;
    time->inc = 0;
    time->starttime = 0;
    time->stoptime = 0;
    time->timeset = 0;
    time->stopped = 0;
    time->softLimit = 0;
    time->hardLimit = 0;
}

int getTimeMilliSecond(void) {
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
            gettimeofday(&time_value, NULL);
            return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
}

