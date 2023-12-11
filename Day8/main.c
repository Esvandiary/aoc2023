#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"

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

uint16_t aaaaaa[64];
uint64_t periods[64] = {0};

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    size_t aaaaaaCount = 0;

    chartype* data = file.data;
    chartype* const end = file.data + fileSize;
    while (data < end && *data != '\n')
    {
        *data = (*data >> 4) & 1;
        ++data;
    }
    const uint8_t* aInstructions = (const uint8_t*)(file.data);
    size_t instructionCount = data - file.data;
    data += 2; // '\n\n'

    uint16_t destinations[1U << 16];

    DSTOPWATCH_START(parsing);
    while (data < end)
    {
        uint16_t src = GETIDX(*data++, *data++, *data++);
        data += 4; // ' = (';
        uint16_t dleft = GETIDX(*data++, *data++, *data++);
        data += 2; // ', '
        uint16_t dright = GETIDX(*data++, *data++, *data++);
        data += 2; // ')\n'

        destinations[DSTIDX(src, 0)] = dleft;
        destinations[DSTIDX(src, 1)] = dright;

        if ((src & 0x1F) == 1)
            aaaaaa[aaaaaaCount++] = src;
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
        for (size_t i = 0; i < instructionCount; ++i)
        {
            const uint8_t n = aInstructions[i];
            src = destinations[DSTIDX(src, n)];
        }
        sum1 += instructionCount;
    }

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_PRINT(part1);

    //
    // Part 2
    //

    DSTOPWATCH_START(part2);
    uint64_t sum2 = 0;

    for (size_t i = 0; i < aaaaaaCount; ++i)
    {
        if (aaaaaa[i] == GETIDX('A','A','A'))
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
                aaaaaa[i] = destinations[DSTIDX(aaaaaa[i], n)];
            }
            iter += instructionCount;
            if ((aaaaaa[i] & 0x1F) == 26)
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
    sum2 = lcm(periods, aaaaaaCount);
    DSTOPWATCH_END(lcm);
    DSTOPWATCH_PRINT(lcm);

    print_uint64(sum2);

    return 0;
}

