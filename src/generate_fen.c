#include "generate_fen.h"

// Default FEN generation settings
const int how_many_ply = 8;

void extract_and_print_fen(board *pos) {
    const char* pieces = "PNBRQKpnbrqk";
    char fen[128]; 
    int char_idx = 0;
    
    // 1. extract pieces
    for (int rank = 0; rank <= 7; rank++) { 
        int empty_count = 0;
        for (int file = 0; file < 8; file++) {            
            int square = rank * 8 + file; 
            int piece = pos->mailbox[square];

            if (piece >= 12) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen[char_idx++] = (char)(empty_count + '0');
                    empty_count = 0;
                }
                fen[char_idx++] = pieces[piece];
            }
        }
        if (empty_count > 0) {
            fen[char_idx++] = (char)(empty_count + '0');
        }
        if (rank < 7) {
            fen[char_idx++] = '/';
        }
    }

    // a space for side to move
    fen[char_idx++] = ' ';

    // 2. side to move
    fen[char_idx++] = (pos->side == white) ? 'w' : 'b';

    // a space for castling rights
    fen[char_idx++] = ' ';

    // 3. castling rights
    if (pos->castle == 0) {
        fen[char_idx++] = '-';
    } else {
        if (pos->castle & wk) fen[char_idx++] = 'K';
        if (pos->castle & wq) fen[char_idx++] = 'Q';
        if (pos->castle & bk) fen[char_idx++] = 'k';
        if (pos->castle & bq) fen[char_idx++] = 'q';
    }

    // 4. en passant
    fen[char_idx++] = ' ';
    if (pos->enpassant == no_sq) {
        fen[char_idx++] = '-';
    } else {
        int file = pos->enpassant % 8;
        int rank = 8 - (pos->enpassant / 8);
        fen[char_idx++] = (char)(file + 'a');
        fen[char_idx++] = (char)(rank + '0');
    }

    // 5. Fifty Move and Full Move Counter
    fen[char_idx] = '\0';
    sprintf(fen + char_idx, " %d %d", pos->fifty, pos->full_moves);
        
    printf("FEN: %s\n", fen);
}

void default_fen_generation(board *pos) {
    moves moveList[1];
    moveGenerator(moveList, pos);    

    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(pos, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, pos) == 0) {
            // skip to the next move
            continue;
        }

        pBoard(pos);

        takeBack(pos, &copyPosition);
    }
}