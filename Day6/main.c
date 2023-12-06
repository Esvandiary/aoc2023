#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mmap.h"
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

static inline FORCEINLINE uint64_t countwins(Race* race)
{
    uint64_t minHold = 1, maxHold = race->time / 2;
    while (maxHold - minHold > 1)
    {
        uint64_t hold = minHold + (maxHold - minHold) / 2;
        if (distance(race->time, hold) > race->record)
            maxHold = hold;
        else
            minHold = hold;
    }
    return 2 * ((race->time / 2) + 1 - maxHold) - (1 - (race->time % 2));
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    vuctor races = VUCTOR_INIT;
    VUCTOR_RESERVE(races, Race, 64);

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
        Race* r = VUCTOR_ADD_NOINIT(races, Race);
        r->time = num;
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
        Race* r = VUCTOR_GET_PTR(races, Race, raceIdx);
        r->record = num;
        ++raceIdx;
    }
    race2.record = num2;

    //
    // Part 1
    //

    uint64_t sum1 = 1;

    Race* aRaces = VUCTOR_GET_PTR(races, Race, 0);
    for (int i = 0; i < races.size; ++i)
    {
        sum1 *= countwins(aRaces + i);
    }

    printf("%" PRIu64 "\n", sum1);
 
    //
    // Part 2
    //

    uint64_t sum2 = 0;

    sum2 = countwins(&race2);

    printf("%" PRIu64 "\n", sum2);

    return 0;
}

