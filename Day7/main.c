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

#define HT_HIGH  0
#define HT_1PAIR 1
#define HT_2PAIR 2
#define HT_3KIND 3
#define HT_HOUSE 4
#define HT_4KIND 5
#define HT_5KIND 6

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

const uint8_t values[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 00
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 10
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
    0,   0, C_2, C_3, C_4, C_5, C_6, C_7, C_8, C_9,   0,   0,   0,   0,   0,   0, // 30
    0, C_A,   0,   0,   0,   0,   0,   0,   0,   0, C_J, C_K,   0,   0,   0,   0, // 40
    0, C_Q,   0,   0, C_T,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 50
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 70
};

static inline FORCEINLINE uint64_t get_ht(uint8_t maxCount, uint8_t secondCount)
{
    switch (maxCount)
    {
        case 5:
            return HT_5KIND;
        case 4:
            return HT_4KIND;
        case 3:
            return (secondCount == 2) ? HT_HOUSE : HT_3KIND;
        case 2:
            return (secondCount == 2) ? HT_2PAIR : HT_1PAIR;
        default:
            return HT_HIGH;
    }
}

static inline FORCEINLINE size_t parse_hand(const chartype* s, uint64_t* out)
{
    // format
    //  63     ...     30 29    0
    // [HT C1 C2 C3 C4 C5 BIDSIZE]
    const chartype* s_start = s;
    uint64_t result = 0;
    uint8_t counts[64] = {0};

    const uint64_t v1 = values[*s++];
    result |= (v1 << 54);
    ++counts[v1];
    const uint64_t v2 = values[*s++];
    result |= (v2 << 48);
    ++counts[v2];
    const uint64_t v3 = values[*s++];
    result |= (v3 << 42);
    ++counts[v3];
    const uint64_t v4 = values[*s++];
    result |= (v4 << 36);
    ++counts[v4];
    const uint64_t v5 = values[*s++];
    result |= (v5 << 30);
    ++counts[v5];

    ++s; // space

    uint32_t bid = 0;
    while (isdigit(*s))
    {
        bid *= 10;
        bid += (*s++ & 0xF);
    }
    result |= bid;

    // figure out result
    uint8_t maxCount = 0;
    uint8_t secondCount = 0;
    for (int i = C_2; i <= C_A; ++i)
    {
        if (counts[i] > maxCount)
        {
            secondCount = maxCount;
            maxCount = counts[i];
        }
        else if (counts[i] > secondCount)
        {
            secondCount = counts[i];
        }
    }
    
    result |= (get_ht(maxCount, secondCount) << 60);

    *out = result;
    return (s - s_start);
}

static int comparer(const void* p1, const void* p2)
{
    return *(const uint64_t*)p1 > *(const uint64_t*)p2;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    //
    // Part 1
    //

    uint64_t sum1 = 0;

    vuctor games = VUCTOR_INIT;
    VUCTOR_RESERVE(games, uint64_t, 2048);

    size_t idx = 0;
    while (idx + 2 < file.size)
    {
        uint64_t* n = VUCTOR_ADD_NOINIT(games, uint64_t);
        idx += parse_hand(file.data + idx, n);
        ++idx; // '\n'
    }

    vuctor gcopy = VUCTOR_INIT;
    VUCTOR_RESERVE(gcopy, uint64_t, games.size);

    radixSort((uint64_t*)games.data, games.size, (uint64_t*)gcopy.data);

    for (int i = 0; i < games.size; ++i)
    {
        uint32_t bid = VUCTOR_GET(games, uint64_t, i) & 0x3FFFFFFF;
        sum1 += (bid * (i + 1));
    }

    print_uint64(sum1);
 
    //
    // Part 2
    //

    uint64_t sum2 = 0;

    for (int i = 0; i < games.size; ++i)
    {
        uint64_t* n = VUCTOR_GET_PTR(games, uint64_t, i);
        // mask out jokers and previous result
        *n &= (~(7ULL << 60) & ~(1ULL << 58) & ~(1ULL << 52) & ~(1ULL << 46) & ~(1ULL << 40) & ~(1ULL << 34));

        uint8_t counts[64] = {0};
        ++counts[(*n >> 54) & 0x3F];
        ++counts[(*n >> 48) & 0x3F];
        ++counts[(*n >> 42) & 0x3F];
        ++counts[(*n >> 36) & 0x3F];
        ++counts[(*n >> 30) & 0x3F];

        // figure out result
        uint8_t maxCount = 0;
        uint8_t secondCount = 0;
        for (int i = C_2; i <= C_A; ++i)
        {
            if (counts[i] > maxCount)
            {
                secondCount = maxCount;
                maxCount = counts[i];
            }
            else if (counts[i] > secondCount)
            {
                secondCount = counts[i];
            }
        }
        // add jokers to maximise result
        maxCount += counts[C_JOKER];
        
        *n |= (get_ht(maxCount, secondCount) << 60);
    }

    radixSort((uint64_t*)games.data, games.size, (uint64_t*)gcopy.data);

    for (int i = 0; i < games.size; ++i)
    {
        uint32_t bid = VUCTOR_GET(games, uint64_t, i) & 0x3FFFFFFF;
        sum2 += (bid * (i + 1));
    }

    print_uint64(sum2);

    return 0;
}

