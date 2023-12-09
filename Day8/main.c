#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"
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

    vuctor aaaaaa = VUCTOR_INIT;
    VUCTOR_RESERVE(aaaaaa, uint16_t, 64);

    size_t idx = 0;
    while (idx < file.size && file.data[idx] != '\n')
    {
        file.data[idx] = (file.data[idx] >> 4) & 1;
        ++idx;
    }
    const uint8_t* aInstructions = (const uint8_t*)(file.data);
    size_t instructionCount = idx;
    idx += 2; // '\n\n'

    uint16_t destinations[1U << 16];

    DSTOPWATCH_START(parsing);
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
    DSTOPWATCH_END(parsing);
    DSTOPWATCH_PRINT(parsing);

    //
    // Part 1
    //

    DSTOPWATCH_START(part1);
    uint64_t sum1 = 0;

    uint16_t src = GETIDX('A', 'A', 'A');
    const uint16_t soln = GETIDX('Z', 'Z', 'Z');
    while (src != soln)
    {
        const uint8_t n = aInstructions[sum1++ % instructionCount];
        src = destinations[DSTIDX(src, n)];
    }

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_PRINT(part1);

    //
    // Part 2
    //

    DSTOPWATCH_START(part2);
    uint64_t sum2 = 0;

    uint16_t* const aaaaaaa = VUCTOR_GET_PTR(aaaaaa, uint16_t, 0);
    uint64_t* const periods = (uint64_t*)calloc(aaaaaa.size, sizeof(uint64_t));

    for (size_t i = 0; i < aaaaaa.size; ++i)
    {
        if (aaaaaaa[i] == GETIDX('A','A','A'))
        {
            DEBUGLOG("skipping AAA -> ZZZ\n");
            periods[i] = sum1;
            continue;
        }
        size_t iter = 0;
        while (true)
        {
            for (size_t j = 0; j < instructionCount; ++j)
            {
                const uint8_t n = aInstructions[j];
                // DEBUGLOG("[%5lu] [%c] [%c%c%c --> %c%c%c]\n", iter, n ? 'R' : 'L', CHARLIST(aaaaaaa[i]), CHARLIST(destinations[DSTIDX(aaaaaaa[i], n)]));
                aaaaaaa[i] = destinations[DSTIDX(aaaaaaa[i], n)];
            }
            iter += instructionCount;
            if ((aaaaaaa[i] & 0x1F) == 26)
            {
                periods[i] = iter;
                break;
            }
        }
    }
    DSTOPWATCH_END(part2);
    DSTOPWATCH_PRINT(part2);

    // DEBUGLOG("GOTTEM\n");
    // for (size_t i = 0; i < aaaaaa.size; ++i)
    //     DEBUGLOG("[%zu] %lu\n", i, periods[i]);

    DSTOPWATCH_START(lcm);
    sum2 = lcm(periods, aaaaaa.size);
    DSTOPWATCH_END(lcm);
    DSTOPWATCH_PRINT(lcm);

    print_uint64(sum2);

    return 0;
}

