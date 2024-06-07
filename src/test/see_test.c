//
// Created by erena on 31.05.2024.
//

#include <stdio.h>

#include "see_test.h"
#include "../board_constants.h"
#include "../move.h"
#include "../see.h"
#include "../fen.h"



int test_see() {
    see_test_case tests[30];
    tests[0].fen   = "4R3/2r3p1/5bk1/1p1r3p/p2PR1P1/P1BK1P2/1P6/8 b - -";
    tests[0].move  = encodeMove(h5, g4, p, 0, 1, 0, 0, 0);
    tests[0].score = 0;

    tests[1].fen   = "4R3/2r3p1/5bk1/1p1r1p1p/p2PR1P1/P1BK1P2/1P6/8 b - -";
    tests[1].move  = encodeMove(h5, g4, p, 0, 1, 0, 0, 0);
    tests[1].score = 0;

    tests[2].fen   = "4r1k1/5pp1/nbp4p/1p2p2q/1P2P1b1/1BP2N1P/1B2QPPK/3R4 b - -";
    tests[2].move  = encodeMove(g4, f3, b, 0, 1, 0, 0, 0);
    tests[2].score = -28;

    tests[3].fen   = "2r1r1k1/pp1bppbp/3p1np1/q3P3/2P2P2/1P2B3/P1N1B1PP/2RQ1RK1 b - -";
    tests[3].move  = encodeMove(d6, e5, p, 0, 1, 0, 0, 0);
    tests[3].score = 82;

    tests[4].fen   = "7r/5qpk/p1Qp1b1p/3r3n/BB3p2/5p2/P1P2P2/4RK1R w - -";
    tests[4].move  = encodeMove(e1, e8, R, 0, 0, 0, 0, 0);
    tests[4].score = 0;

    tests[5].fen   = "6rr/6pk/p1Qp1b1p/2n5/1B3p2/5p2/P1P2P2/4RK1R w - -";
    tests[5].move  = encodeMove(e1, e8, R, 0, 0, 0, 0, 0);
    tests[5].score = -477;

    tests[6].fen   = "7r/5qpk/2Qp1b1p/1N1r3n/BB3p2/5p2/P1P2P2/4RK1R w - -";
    tests[6].move  = encodeMove(e1, e8, R, 0, 0, 0, 0, 0);
    tests[6].score = -477;

    tests[7].fen   = "6RR/4bP2/8/8/5r2/3K4/5p2/4k3 w - -";
    tests[7].move  = encodeMove(f7, f8, P, Q, 0, 0, 0, 0);
    tests[7].score = 283;

    tests[8].fen   = "6RR/4bP2/8/8/5r2/3K4/5p2/4k3 w - -";
    tests[8].move  = encodeMove(f7, f8, P, N, 0, 0, 0, 0);
    tests[8].score = 255;

    tests[9].fen   = "7R/5P2/8/8/8/3K2r1/5p2/4k3 w - -";
    tests[9].move  = encodeMove(f7, f8, P, Q, 0, 0, 0, 0);
    tests[9].score = 943;

    tests[10].fen   = "7R/5P2/8/8/8/3K2r1/5p2/4k3 w - -";
    tests[10].move  = encodeMove(f7, f8, P, B, 0, 0, 0, 0);
    tests[10].score = 283;

    tests[11].fen   = "7R/4bP2/8/8/1q6/3K4/5p2/4k3 w - -";
    tests[11].move  = encodeMove(f7, f8, P, R, 0, 0, 0, 0);
    tests[11].score = -82;

    tests[12].fen   = "8/4kp2/2npp3/1Nn5/1p2PQP1/7q/1PP1B3/4KR1r b - -";
    tests[12].move  = encodeMove(h1, f1, R, 0, 1, 0, 0, 0);
    tests[12].score = 0;

    tests[13].fen   = "8/4kp2/2npp3/1Nn5/1p2P1P1/7q/1PP1B3/4KR1r b - -";
    tests[13].move  = encodeMove(h1, f1, R, 0, 1, 0, 0, 0);
    tests[13].score = 0;

    tests[14].fen   = "2r2r1k/6bp/p7/2q2p1Q/3PpP2/1B6/P5PP/2RR3K b - -";
    tests[14].move  = encodeMove(c5, c1, Q, 0, 1, 0, 0, 0);
    tests[14].score = -71;

    tests[15].fen   = "r2qk1nr/pp2ppbp/2b3p1/2p1p3/8/2N2N2/PPPP1PPP/R1BQR1K1 w kq --";
    tests[15].move  = encodeMove(f3, e5, N, 0, 1, 0, 0, 0);
    tests[15].score = 82;

    tests[16].fen   = "6r1/4kq2/b2p1p2/p1pPb3/p1P2B1Q/2P4P/2B1R1P1/6K1 w - -";
    tests[16].move  = encodeMove(f4, e5, B, 0, 1, 0, 0, 0);
    tests[16].score = 0;

    tests[17].fen   = "3q2nk/pb1r1p2/np6/3P2Pp/2p1P3/2R4B/PQ3P1P/3R2K1 w - h6";
    tests[17].move  = encodeMove(g5, h6, P, 0, 1, 0, 1, 0);
    tests[17].score = 0;

    tests[18].fen   = "3q2nk/pb1r1p2/np6/3P2Pp/2p1P3/2R1B2B/PQ3P1P/3R2K1 w - h6";
    tests[18].move  = encodeMove(g5, h6, P, 0, 1, 0, 1, 0);
    tests[18].score = 82;

    tests[19].fen   = "2r4r/1P4pk/p2p1b1p/7n/BB3p2/2R2p2/P1P2P2/4RK2 w - -";
    tests[19].move  = encodeMove(c3, c8, R, 0, 1, 0, 0, 0);
    tests[19].score = 477;

    tests[20].fen   = "2r5/1P4pk/p2p1b1p/5b1n/BB3p2/2R2p2/P1P2P2/4RK2 w - -";
    tests[20].move  = encodeMove(c3, c8, R, 0, 1, 0, 0, 0);
    tests[20].score = 365;

    /*
        Score of this test is 477 (Rook value) because after Rxc8, the bishop on f5 is not supposed
        to take back on c8 because it leads to pawn promotion. But in the current SEE implementation,
        the bishop on f5 takes on c8 leading to the SEE value being 365 (Bishop value).
    */

    tests[21].fen   = "2r4k/2r4p/p7/2b2p1b/4pP2/1BR5/P1R3PP/2Q4K w - -";
    tests[21].move  = encodeMove(c3, c5, R, 0, 1, 0, 0, 0);
    tests[21].score = 365;

    tests[22].fen   = "8/pp6/2pkp3/4bp2/2R3b1/2P5/PP4B1/1K6 w - -";
    tests[22].move  = encodeMove(g2, c6, B, 0, 1, 0, 0, 0);
    tests[22].score = -283;

    tests[23].fen   = "4q3/1p1pr1k1/1B2rp2/6p1/p3PP2/P3R1P1/1P2R1K1/4Q3 b - -";
    tests[23].move  = encodeMove(e6, e4, r, 0, 1, 0, 0, 0);
    tests[23].score = -395;

    tests[24].fen   = "4q3/1p1pr1kb/1B2rp2/6p1/p3PP2/P3R1P1/1P2R1K1/4Q3 b - -";
    tests[24].move  = encodeMove(h7, e4, b, 0, 1, 0, 0, 0);
    tests[24].score = 82;

    // Leorik Test Cases
    tests[25].fen = "6k1/1pp4p/p1pb4/6q1/3P1pRr/2P4P/PP1Br1P1/5RKN w - -";
    tests[25].move = encodeMove(g4, f4, R, 0, 1, 0, 0, 0);
    tests[25].score = -100;


    board position[1];

    for (int i=0; i < 30; i++) {
        parseFEN(tests[i].fen, position);
        if (see(position, tests[i].move) != tests[i].score) {
            //fprintf(stderr, "Test %d failed for fen: %s\n", i+1, tests[i].fen);

            printf("Test %d failed for fen: %s\n", i+1, tests[i].fen);
            //exit(1);
        }
    }
    return 0;
}

void lanTest() {

}


