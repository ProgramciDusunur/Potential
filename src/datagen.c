#include "datagen.h"

/* Adjudication Settings */

// Win adjudication settings
int16_t win_adj_score = 2000;
int16_t win_move_count = 5;

// Draw adjudication settings
int16_t draw_adj_score = 15;
int16_t draw_move_count = 6;
int16_t draw_move_number = 32;

/* Game Conditions */
bool MINIMAL = true;
int16_t HASH = 1;
int16_t NODE_COUNT = 5000;

int start_datagen() {    
    FILE *engine = _popen("Potential.exe", "w");
    if (!engine) {
        printf("Engine could not be started!\n");
        return 1;
    }
    
    fprintf(engine, "uci\n");
    fprintf(engine, "isready\n");
    //fprintf(engine, "ucinewgame\n");
    fprintf(engine, "position startpos\n");
    fprintf(engine, "go nodes 5000");
    fflush(engine);    
    fprintf(engine, "quit\n");
    _pclose(engine);
    printf("Commands sent and engine closed.\n");
    return 0;
}