//
// Created by erena on 2.02.2025.
//

#include "utils.h"



void printMailbox(board *position) {
    printf("\n");
    for (int y = 0, coordinate = 8; y < 8; y++, coordinate--) {
        printf("%d  ", coordinate);
        for (int x = 0; x < 8; x++) {
            int index = y * 8 + x;
            int piece = position->mailbox[index];
            switch (piece) {
                case P:
                    printf("P ");
                    break;
                case N:
                    printf("N ");
                    break;
                case B:
                    printf("B ");
                    break;
                case R:
                    printf("R ");
                    break;
                case Q:
                    printf("Q ");
                    break;
                case K:
                    printf("K ");
                    break;

                case p:
                    printf("p ");
                    break;
                case n:
                    printf("n ");
                    break;
                case b:
                    printf("b ");
                    break;
                case r:
                    printf("r ");
                    break;
                case q:
                    printf("q ");
                    break;
                case k:
                    printf("k ");
                    break;
                default:
                    printf("- ");
                    break;
            }

        }
        printf(" \n");
    }
    printf("\n   a b c d e f g h \n");


    printf("\n");

}

int myMAX(int x, int y) {
    return (x > y) ? x : y;
}

int myMIN(int x, int y) {
    return (x < y) ? x : y;
}

int clamp(int d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}
