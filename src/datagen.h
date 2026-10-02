#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include <io.h>     // _open_osfhandle için
#include <fcntl.h>  // _O_WRONLY ve _O_RDONLY için
#include <stdint.h> // intptr_t için

#if defined(_WIN32)
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

int start_datagen();