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

void update_datagen_ui(uint64_t games, uint64_t positions, double nps, double elapsed_sec) {
    // \r   : Satırın en başına döner
    // \033[K : Satırın geri kalanını temizler (Windows Terminal ve Linux destekler)
    printf("\r\033[K[Datagen] Oyun: %" PRIu64 " | Pozisyon: %" PRIu64 " | Hız: %.0f pos/s | Süre: %.1fs",
           games, positions, nps, elapsed_sec);
    
    // ÇOK ÖNEMLİ: \n olmadığı için ekranı anında güncellemeye zorluyoruz
    fflush(stdout);
}

// Datagen bittiğinde alt satıra geçmek için:
void finish_datagen_ui(void) {
    printf("\n[Datagen] Tamamlandı!\n");
}

int start_datagen() {    
    for (int i = 0; i < 10; i++) {        
        update_datagen_ui(i + 1, (i + 1) * 1000, 1000.0, (i + 1) * 0.5);
        sleep_ms(1000); // Wait for 1 second
    }

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