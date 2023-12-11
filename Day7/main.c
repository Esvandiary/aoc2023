#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/radixsort.h"
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define HT_HIGH  1
#define HT_1PAIR 2
#define HT_2PAIR 3
#define HT_3KIND 4
#define HT_HOUSE 5
#define HT_4KIND 6
#define HT_5KIND 7
#define HT_MAX   8

#define C_JOKER 0
#define C_2 2
#define C_3 3
#define C_4 4
#define C_5 5
#define C_6 6
#define C_7 7
#define C_8 8
#define C_9 9
#define C_T 10
#define C_J 16
#define C_Q 32
#define C_K 33
#define C_A 34
#define C_MAX 35

static const uint8_t values[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 00
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 10
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
    0,   0, C_2, C_3, C_4, C_5, C_6, C_7, C_8, C_9,   0,   0,   0,   0,   0,   0, // 30
    0, C_A,   0,   0,   0,   0,   0,   0,   0,   0, C_J, C_K,   0,   0,   0,   0, // 40
    0, C_Q,   0,   0, C_T,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 50
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 70
};

static const uint8_t htidx[] = {
    HT_5KIND, 0, 0, 0, 0, 0, 0, 0, // only possible with jokers
    HT_HIGH, HT_HIGH, HT_HIGH, HT_HIGH, HT_HIGH, 0, 0, 0,
    HT_1PAIR, HT_1PAIR, HT_2PAIR, 0, 0, 0, 0, 0,
    HT_3KIND, HT_3KIND, HT_HOUSE, 0, 0, 0, 0, 0,
    HT_4KIND, HT_4KIND, HT_4KIND, HT_4KIND, 0, 0, 0, 0,
    HT_5KIND, HT_5KIND, HT_5KIND, HT_5KIND, HT_5KIND, 0, 0, 0
};

static inline FORCEINLINE uint64_t get_ht(uint8_t maxCount, uint8_t secondCount)
{
    return htidx[(maxCount << 3) | secondCount];
}

static inline FORCEINLINE size_t parse_hand(const chartype* s, uint64_t* out1, uint64_t* out2)
{
    // format
    //  63     ...     30 29    0
    // [HT C1 C2 C3 C4 C5 BIDSIZE]
    const chartype* s_start = s;
    uint64_t result1 = 0, result2 = 0;
    uint8_t counts[C_MAX] = {0};

    const uint64_t v1 = values[*s++];
    result1 |= (v1 << 54);
    ++counts[v1];
    const uint64_t v2 = values[*s++];
    result1 |= (v2 << 48);
    ++counts[v2];
    const uint64_t v3 = values[*s++];
    result1 |= (v3 << 42);
    ++counts[v3];
    const uint64_t v4 = values[*s++];
    result1 |= (v4 << 36);
    ++counts[v4];
    const uint64_t v5 = values[*s++];
    result1 |= (v5 << 30);
    ++counts[v5];

    ++s; // space

    uint32_t bid = 0;
    while (isdigit(*s))
    {
        bid *= 10;
        bid += (*s++ & 0xF);
    }
    result1 |= bid;

    // figure out result
    uint8_t maxCount1, secondCount1 = 0;
    uint8_t maxCount2, secondCount2;

    #define CHECKMAX(n, mcount, scount) if (counts[n] > (mcount)) { scount = mcount; mcount = counts[n]; } else if (counts[n] > (scount)) { scount = counts[n]; }

    maxCount1 = counts[C_2];
    CHECKMAX(C_3, maxCount1, secondCount1);
    CHECKMAX(C_4, maxCount1, secondCount1);
    CHECKMAX(C_5, maxCount1, secondCount1);
    CHECKMAX(C_6, maxCount1, secondCount1);
    CHECKMAX(C_7, maxCount1, secondCount1);
    CHECKMAX(C_8, maxCount1, secondCount1);
    CHECKMAX(C_9, maxCount1, secondCount1);
    CHECKMAX(C_T, maxCount1, secondCount1);
    CHECKMAX(C_Q, maxCount1, secondCount1);
    CHECKMAX(C_K, maxCount1, secondCount1);
    CHECKMAX(C_A, maxCount1, secondCount1);
    maxCount2 = maxCount1;
    secondCount2 = secondCount1;
    CHECKMAX(C_J, maxCount1, secondCount1);

    result2 = result1 & (~(1ULL << 58) & ~(1ULL << 52) & ~(1ULL << 46) & ~(1ULL << 40) & ~(1ULL << 34));
    
    result1 |= (get_ht(maxCount1, secondCount1) << 60);
    result2 |= (get_ht(maxCount2 + counts[C_J], secondCount2) << 60);

    *out1 = result1;
    *out2 = result2;
    return (s - s_start);
}

static uint64_t games[4096];
static uint64_t games2[4096];
static uint64_t buf[4096];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    //
    // Part 1 + 2
    //

    size_t gamesCount = 0, games2Count = 0;

    uint64_t sum1 = 0;

    size_t idx = 0;
    while (idx + 2 < file.size)
    {
        uint64_t n, n2;
        idx += parse_hand(file.data + idx, &n, &n2);
        games[gamesCount++] = n;
        games2[games2Count++] = n2;
        ++idx; // '\n'
    }

    radixSort(games, gamesCount, buf);

    for (int c = 0; c < gamesCount; ++c)
    {
        uint32_t bid = games[c] & 0x3FFFFFFF;
        sum1 += (bid * (c + 1));
    }

    print_uint64(sum1);

    uint64_t sum2 = 0;

    radixSort(games2, games2Count, buf);

    for (int c = 0; c < games2Count; ++c)
    {
        uint32_t bid = games2[c] & 0x3FFFFFFF;
        sum2 += (bid * (c + 1));
    }

    print_uint64(sum2);

    return 0;
}

