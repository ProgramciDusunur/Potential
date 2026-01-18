#include "generate_fen.h"

// Default FEN generation settings
const int how_many_ply = 2;

FenString get_fen(board *pos) {
    const char* pieces = "PNBRQKpnbrqk";
    FenString result;
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
                    result.str[char_idx++] = (char)(empty_count + '0');
                    empty_count = 0;
                }
                result.str[char_idx++] = pieces[piece];
            }
        }
        if (empty_count > 0) {
            result.str[char_idx++] = (char)(empty_count + '0');
        }
        if (rank < 7) {
            result.str[char_idx++] = '/';
        }
    }

    // a space for side to move
    result.str[char_idx++] = ' ';

    // 2. side to move
    result.str[char_idx++] = (pos->side == white) ? 'w' : 'b';

    // a space for castling rights
    result.str[char_idx++] = ' ';

    // 3. castling rights
    if (pos->castle == 0) {
        result.str[char_idx++] = '-';
    } else {
        if (pos->castle & wk) result.str[char_idx++] = 'K';
        if (pos->castle & wq) result.str[char_idx++] = 'Q';
        if (pos->castle & bk) result.str[char_idx++] = 'k';
        if (pos->castle & bq) result.str[char_idx++] = 'q';
    }

    // 4. en passant
    result.str[char_idx++] = ' ';
    if (pos->enpassant == no_sq) {
        result.str[char_idx++] = '-';
    } else {
        int file = pos->enpassant % 8;
        int rank = 8 - (pos->enpassant / 8);
        result.str[char_idx++] = (char)(file + 'a');
        result.str[char_idx++] = (char)(rank + '0');
    }

    // 5. Fifty Move and Full Move Counter
    result.str[char_idx] = '\0';
    sprintf(result.str + char_idx, " %d %d", pos->fifty, pos->full_moves);

    return result;
}

void default_fen_generation(board *pos, int current_ply) {    
    if (current_ply == how_many_ply) {
        printf("info string genfens %s\n", get_fen(pos).str);
        return;
    }

    moves moveList[1];
    moveGenerator(moveList, pos);
    
    int legal_moves[256];
    int legal_count = 0;

    for (int i = 0; i < moveList->count; i++) {
        struct copyposition cp;
        copyBoard(pos, &cp);

        if (makeMove(moveList->moves[i], allMoves, pos)) {
            legal_moves[legal_count++] = moveList->moves[i];
        }        
        takeBack(pos, &cp);
    }
    
    if (legal_count == 0) return;
    
    int random_idx = get_random_uint64_number() % legal_count;
    int selected_move = legal_moves[random_idx];

    struct copyposition cp_step;
    copyBoard(pos, &cp_step);
    
    if (makeMove(selected_move, allMoves, pos)) {
        default_fen_generation(pos, current_ply + 1);
    }
        
    takeBack(pos, &cp_step);
}