#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/vuctor.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct Race
{
    uint64_t time;
    uint64_t record;
} Race;

static inline FORCEINLINE uint64_t distance(uint64_t time, uint64_t hold)
{
    return (time - hold) * hold;
}

// https://stackoverflow.com/a/63452286
static inline FORCEINLINE uint8_t bit_width(uint64_t x)
{
    return x == 0 ? 1 : 64 - __builtin_clzll(x);
}

static inline FORCEINLINE uint64_t usqrt(uint64_t n)
{
    uint8_t shift = bit_width(n);
    shift += shift & 1;

    uint64_t result = 0;

    do {
        shift -= 2;
        result <<= 1;
        result |= 1;
        result ^= result * result > (n >> shift);
    } while (shift != 0);

    return result;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    Race aRaces1[64];
    int aRaces1Count = 0;

    Race race2;

    int idx = 9; // 'Distance:'
    uint64_t num2 = 0;
    while (idx < file.size && file.data[idx] != '\n')
    {
        ++idx; // space
        while (!isdigit(file.data[idx]))
            ++idx;
        uint64_t num = 0;
        while (isdigit(file.data[idx]))
        {
            num *= 10;
            num += file.data[idx] & 0xF;
            num2 *= 10;
            num2 += file.data[idx] & 0xF;
            ++idx;
        }
        aRaces1[aRaces1Count++].time = num;
    }
    race2.time = num2;
    num2 = 0;
    idx += 10; // '\nDistance:'
    int raceIdx = 0;
    while (idx < file.size && file.data[idx] != '\n')
    {
        ++idx; // space
        while (!isdigit(file.data[idx]))
            ++idx;
        uint64_t num = 0;
        while (isdigit(file.data[idx]))
        {
            num *= 10;
            num += file.data[idx] & 0xF;
            num2 *= 10;
            num2 += file.data[idx] & 0xF;
            ++idx;
        }
        aRaces1[raceIdx++].record = num;
    }
    race2.record = num2;

    //
    // Part 1
    //

    uint64_t sum1 = 1;

    for (int i = 0; i < aRaces1Count; ++i)
    {
        uint64_t hold = (aRaces1[i].time - (uint64_t)sqrtf(aRaces1[i].time * aRaces1[i].time - 4 * aRaces1[i].record)) / 2;
        while (distance(aRaces1[i].time, hold) <= aRaces1[i].record)
            ++hold;

        sum1 *= 2 * ((aRaces1[i].time / 2) + 1 - hold) - (1 - (aRaces1[i].time % 2));
    }

    print_uint64(sum1);
 
    //
    // Part 2
    //

    uint64_t hold = (race2.time - (uint64_t)sqrtf(race2.time * race2.time - 4 * race2.record)) / 2;
    while (distance(race2.time, hold) <= race2.record)
        ++hold;

    uint64_t sum2 = 2 * ((race2.time / 2) + 1 - hold) - (1 - (race2.time % 2));

    print_uint64(sum2);

    return 0;
}

