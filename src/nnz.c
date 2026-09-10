#include "nnz.h"

__attribute__((aligned(64))) uint16_t NONZERO_INDICES[256][8];

void init_nnz(void) {
    for (int i = 0; i < 256; i++) {
        int count = 0;
        for (int j = 0; j < 8; j++) {
            if (i & (1 << j)) {
                NONZERO_INDICES[i][count++] = (uint16_t)j;
            }
        }
        while (count < 8) {
            NONZERO_INDICES[i][count++] = 0;
        }
    }
}
