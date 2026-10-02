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
    for (int i = 0; i < 3; i++) {        
        update_datagen_ui(i + 1, (i + 1) * 1000, 1000.0, (i + 1) * 0.5);
        sleep_ms(1000); // Wait for 1 second
    }

    char link[64];
    int8_t worker_id = 1;
    snprintf(link, sizeof(link), "Potential_Worker_%02d.exe", worker_id);
    CreateHardLinkA(link, "Potential.exe", NULL);

    // Boru değişkenlerini tanımla
    HANDLE hStdin_Rd, hStdin_Wr;
    HANDLE hStdout_Rd, hStdout_Wr;

    // Miras alma ayarını yap
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Boruları oluştur
    CreatePipe(&hStdin_Rd,  &hStdin_Wr,  &sa, 0);
    CreatePipe(&hStdout_Rd, &hStdout_Wr, &sa, 0);
    
    STARTUPINFOA si;          // 1. Kutu: Motora vereceğimiz ayarlar (kablolar buraya)
    PROCESS_INFORMATION pi;   // 2. Kutu: Motor açılınca Windows'un bize vereceği kumanda
    ZeroMemory(&si, sizeof(si)); // Kutuları temizle
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si); // Windows'un zorunlu kuralı (kendi boyutunu bilmek ister)


    // 4. Şimdi o sorduğun si ayarlarına tam olarak bunları veriyoruz:
    si.hStdInput  = hStdin_Rd;   // Motorun kulağına 1. borunun OKUMA ucunu ver
    si.hStdOutput = hStdout_Wr;  // Motorun ağzına 2. borunun YAZMA ucunu ver
    si.hStdError  = hStdout_Wr;  // Hata yaparsa da aynı boruya bağırsın
    si.dwFlags   |= STARTF_USESTDHANDLES;   // "Ben kabloları bağladım, bunları kullan!" bayrağ



    // 1. MOTORU ATEŞLE! (Penceresiz, arka planda başlar)
    CreateProcessA(
        NULL,
        link,              // "Potential_Worker_01.exe"
        NULL, NULL,
        TRUE,              // Boruları çocuğa devret
        CREATE_NO_WINDOW,  // Siyah konsol açma
        NULL, NULL,
        &si, &pi
    );
    // 2. Çocuğa verdiğimiz uçları ana programda kapatıyoruz (çift açık kalmasın)
    CloseHandle(hStdin_Rd);
    CloseHandle(hStdout_Wr);
    CloseHandle(pi.hThread); // Thread kumandasına gerek yok
    // 3. Boruların ucuna C prizi takıyoruz (FILE* yapıyoruz)
    int fd_in  = _open_osfhandle((intptr_t)hStdin_Wr,  _O_WRONLY);
    int fd_out = _open_osfhandle((intptr_t)hStdout_Rd, _O_RDONLY);
    FILE *in  = _fdopen(fd_in,  "w");
    FILE *out = _fdopen(fd_out, "r");
    setvbuf(in, NULL, _IONBF, 0); // Tamponlamayı kapat (yazdığın anında gitsin)
    
    fprintf(in, "uci\n");
    // Motora "hazır mısın?" diye soruyoruz
    fprintf(in, "isready\n");
    
    //fprintf(engine, "ucinewgame\n");
    fprintf(in, "position startpos\n");
    fprintf(in, "setoption name Hash value 1\n");
    fprintf(in, "go nodes 5000\n"); // \n eklendi!
    fflush(in);

    char line[1024];
    char bestmove[16] = "";

    // Motor cevabı basana kadar dinle:
    while (fgets(line, sizeof(line), out)) {
        // İstersen motorun düşündüğü satırları da görebilirsin:
        printf("%s", line);

        if (strncmp(line, "bestmove", 8) == 0) {
            sscanf(line, "bestmove %s", bestmove);
            break; 
        }
    }

    printf("\n>>> Worker'dan gelen en iyi hamle: %s <<<\n", bestmove);

    fprintf(in, "quit\n");
    fclose(in);
    fclose(out);
    DeleteFileA(link);
    

    return 0;
}