#ifndef POTENTIAL_NNZ_H
#define POTENTIAL_NNZ_H

#include <stdint.h>
#include <stdbool.h>
#include "simd.h"

#define NNZ_MAX_TILES 512
#define NNZ_BUFFER_SIZE 528

extern uint16_t NONZERO_INDICES[256][8];

void init_nnz(void);

#if defined(USE_AVX512)

static inline int find_nonzero_indices(const uint8_t *ft, uint16_t *indices) {
    int count = 0;
    __m128i base = _mm_setzero_si128();
    const __m128i v_eight = _mm_set1_epi16(8);
    const __m512i zero512 = _mm512_setzero_si512();

    for (int i = 0; i < NNZ_MAX_TILES * 4; i += 64) {
        __m512i ft_vec = _mm512_loadu_si512((const __m512i *)&ft[i]);
        __mmask16 mask = _mm512_cmpneq_epi32_mask(ft_vec, zero512);

        uint8_t byte0 = (uint8_t)(mask & 0xFF);
        __m128i mask_indices0 = _mm_load_si128((const __m128i *)NONZERO_INDICES[byte0]);
        __m128i actual_indices0 = _mm_add_epi16(mask_indices0, base);
        _mm_storeu_si128((__m128i *)&indices[count], actual_indices0);
        count += __builtin_popcount(byte0);
        base = _mm_add_epi16(base, v_eight);

        uint8_t byte1 = (uint8_t)(mask >> 8);
        __m128i mask_indices1 = _mm_load_si128((const __m128i *)NONZERO_INDICES[byte1]);
        __m128i actual_indices1 = _mm_add_epi16(mask_indices1, base);
        _mm_storeu_si128((__m128i *)&indices[count], actual_indices1);
        count += __builtin_popcount(byte1);
        base = _mm_add_epi16(base, v_eight);
    }
    return count;
}

#elif defined(USE_AVX2)

static inline int find_nonzero_indices(const uint8_t *ft, uint16_t *indices) {
    int count = 0;
    __m128i base = _mm_setzero_si128();
    const __m128i v_eight = _mm_set1_epi16(8);
    const __m256i zero256 = _mm256_setzero_si256();

    for (int i = 0; i < NNZ_MAX_TILES * 4; i += 32) {
        __m256i ft_vec = _mm256_loadu_si256((const __m256i *)&ft[i]);
        __m256i cmp = _mm256_cmpeq_epi32(ft_vec, zero256);
        uint8_t byte = (uint8_t)(~_mm256_movemask_ps(_mm256_castsi256_ps(cmp)));

        __m128i mask_indices = _mm_load_si128((const __m128i *)NONZERO_INDICES[byte]);
        __m128i actual_indices = _mm_add_epi16(mask_indices, base);
        _mm_storeu_si128((__m128i *)&indices[count], actual_indices);

        count += __builtin_popcount(byte);
        base = _mm_add_epi16(base, v_eight);
    }
    return count;
}

#else

static inline int find_nonzero_indices(const uint8_t *ft, uint16_t *indices) {
    int count = 0;
    const uint32_t *in32 = (const uint32_t *)ft;
    for (int t = 0; t < NNZ_MAX_TILES; t++) {
        if (in32[t]) {
            indices[count++] = (uint16_t)t;
        }
    }
    return count;
}

#endif


#endif // POTENTIAL_NNZ_H
