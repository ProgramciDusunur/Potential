#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#if defined(_WIN32)
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

int start_datagen();