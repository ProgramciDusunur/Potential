//
// Created by erena on 13.09.2024.
//

#include "time.h"


void initTimeControl(time* time) {
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


// reset time control variables
void resetTimeControl(time* time) {
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

int getTimeMiliSecond(void) {
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
            gettimeofday(&time_value, NULL);
            return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
    }

int input_waiting(void) {
    #ifndef WIN32
        fd_set readfds;
            struct timeval tv;
            FD_ZERO (&readfds);
            FD_SET (fileno(stdin), &readfds);
            tv.tv_sec=0; tv.tv_usec=0;
            select(16, &readfds, 0, 0, &tv);

            return (FD_ISSET(fileno(stdin), &readfds));
    #else
        int init = 0, pipe;
        HANDLE inh;
        DWORD dw;

        if (!init) {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe) {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }

        if (pipe) {
            if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
            return dw;
        } else {
            GetNumberOfConsoleInputEvents(inh, &dw);
            return dw <= 1 ? 0 : dw;
        }

    #endif
}
