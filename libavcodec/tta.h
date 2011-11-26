#ifndef AVCODEC_TTA_H
#define AVCODEC_TTA_H

#include "avcodec.h"

#define FORMAT_SIMPLE    1
#define FORMAT_ENCRYPTED 2

#define MAX_ORDER 16

static const uint32_t shift_1[] = {
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x80000000, 0x80000000, 0x80000000, 0x80000000,
    0x80000000, 0x80000000, 0x80000000, 0x80000000
};

static const uint32_t * const shift_16 = shift_1 + 4;

typedef struct TTAFilter {
    int32_t shift, round, error, mode;
    int32_t qm[MAX_ORDER];
    int32_t dx[MAX_ORDER];
    int32_t dl[MAX_ORDER];
} TTAFilter;

typedef struct TTARice {
    uint32_t k0, k1, sum0, sum1;
} TTARice;

#endif /* AVCODEC_TTA_H */
