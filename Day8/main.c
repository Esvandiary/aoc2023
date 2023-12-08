#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define GETIDX(a, b, c) (uint16_t)((((a) & 0x1F) << 10) | (((b) & 0x1F) << 5) | (((c) & 0x1F) >> 0))
#define DSTIDX(a, n) (((a) << 1) | n)
#define CHARLIST(a) (char)((((a) >> 10) & 0x1F) | 64), (char)((((a) >> 5) & 0x1F) | 64), (char)((((a) >> 0) & 0x1F) | 64)

static inline FORCEINLINE uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b != 0)
    {
        a %= b;
        a ^= b;
        b ^= a;
        a ^= b;
    }
    return a;
}

static inline uint64_t lcm(const uint64_t* a, size_t size)
{
    if (size > 2)
    {
        uint64_t b = lcm(a + 1, size - 1);
        return a[0] * b / gcd(a[0], b);
    }
    else
    {
        return a[0] * a[1] / gcd(a[0], a[1]);
    }
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    vuctor instructions = VUCTOR_INIT;
    VUCTOR_RESERVE(instructions, uint8_t, 1024);

    vuctor aaaaaa = VUCTOR_INIT;
    VUCTOR_RESERVE(aaaaaa, uint16_t, 1024);

    size_t idx = 0;
    while (idx < file.size && file.data[idx] != '\n')
    {
        uint8_t n = (file.data[idx++] >> 4) & 1;
        VUCTOR_ADD(instructions, uint8_t, n);
    }
    idx += 2; // '\n\n'

    uint16_t destinations[1U << 16];

    while (idx < file.size)
    {
        uint16_t src = GETIDX(file.data[idx++], file.data[idx++], file.data[idx++]);
        idx += 4; // ' = (';
        uint16_t dleft = GETIDX(file.data[idx++], file.data[idx++], file.data[idx++]);
        idx += 2; // ', '
        uint16_t dright = GETIDX(file.data[idx++], file.data[idx++], file.data[idx++]);
        idx += 2; // ')\n'

        destinations[DSTIDX(src, 0)] = dleft;
        destinations[DSTIDX(src, 1)] = dright;

        if ((src & 0x1F) == 1)
            VUCTOR_ADD(aaaaaa, uint16_t, src);
    }

    //
    // Part 1
    //

    uint64_t sum1 = 0;

    uint16_t src = GETIDX('A', 'A', 'A');
    const uint16_t soln = GETIDX('Z', 'Z', 'Z');
    while (src != soln)
    {
        const uint8_t n = VUCTOR_GET(instructions, uint8_t, sum1++ % instructions.size);
        src = destinations[DSTIDX(src, n)];
    }

    print_uint64(sum1);

    //
    // Part 2
    //

    uint64_t sum2 = 0;

    uint16_t* const aaaaaaa = VUCTOR_GET_PTR(aaaaaa, uint16_t, 0);
    uint64_t* const periodalloc = (uint64_t*)calloc(aaaaaa.size * 2, sizeof(uint64_t));
    uint64_t* const first = periodalloc + 0;
    uint64_t* const periods = periodalloc + aaaaaa.size;

    for (size_t i = 0; i < aaaaaa.size; ++i)
    {
        if (aaaaaaa[i] == GETIDX('A','A','A'))
        {
            DEBUGLOG("skipping AAA -> ZZZ\n");
            periods[i] = sum1;
            continue;
        }
        size_t iter;
        for (iter = 0; iter < 10000000; ++iter)
        {
            const uint8_t n = VUCTOR_GET(instructions, uint8_t, iter % instructions.size);
            // DEBUGLOG("[%5lu] [%c] [%c%c%c --> %c%c%c]\n", iter, n ? 'R' : 'L', CHARLIST(aaaaaaa[i]), CHARLIST(destinations[DSTIDX(aaaaaaa[i], n)]));
            aaaaaaa[i] = destinations[DSTIDX(aaaaaaa[i], n)];
            if ((aaaaaaa[i] & 0x1F) == 26)
            {
                periods[i] = iter + 1;
                break;
            }
        }
    }

    DEBUGLOG("GOTTEM\n");
    for (size_t i = 0; i < aaaaaa.size; ++i)
        DEBUGLOG("[%zu] %lu\n", i, periods[i]);

    sum2 = lcm(periods, aaaaaa.size);

    print_uint64(sum2);

    return 0;
}

