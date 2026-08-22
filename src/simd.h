#ifndef POTENTIAL_SIMD_H
#define POTENTIAL_SIMD_H

#include <stdbool.h>
#include <stdint.h>

#define idcs(x) (0 + x), (2 + x), (4 + x), (6 + x)
#if defined(__AVX512F__)
#define VEC_BYTES 64
#define EVENS idcs(0), idcs(8), idcs(16), idcs(24)
#define ODDS idcs(1), idcs(9), idcs(17), idcs(25)
#elif defined(__AVX2__)
#define VEC_BYTES 32
#define EVENS idcs(0), idcs(8)
#define ODDS idcs(1), idcs(9)
#else
#define VEC_BYTES 16
#define EVENS idcs(0)
#define ODDS idcs(1)
#endif

#define VEC_ELEMENTS(T) (VEC_BYTES / (int)sizeof(T))

// vector of i16
typedef int16_t v16 __attribute__((__vector_size__(VEC_BYTES), __may_alias__));
// unaligned v16
typedef int16_t v16u __attribute__((__vector_size__(VEC_BYTES), __may_alias__, __aligned__(1)));
typedef int32_t v32 __attribute__((__vector_size__(VEC_BYTES), __may_alias__));
// wide v32
typedef int32_t v32w __attribute__((__vector_size__(2 * VEC_BYTES), __may_alias__));

static inline v16 crelu(v16 a) {
    #pragma GCC unroll
    for (int i = 0; i < VEC_ELEMENTS(int16_t); ++i)
        a[i] = a[i] < 0 ? 0 : a[i];
    #pragma GCC unroll
    for (int i = 0; i < VEC_ELEMENTS(int16_t); ++i)
        a[i] = a[i] > 255 ? 255 : a[i];
    return a;
}

static inline v32 madd(v16 a, v16 b) {
    v32w p = __builtin_convertvector(a, v32w) * __builtin_convertvector(b, v32w);
    return __builtin_shufflevector(p, p, EVENS) + __builtin_shufflevector(p, p, ODDS);
}

#endif // POTENTIAL_SIMD_H
