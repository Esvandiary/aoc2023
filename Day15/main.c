#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct hentry
{
    union
    {
        struct
        {
            uint32_t label;
            uint32_t focalLength;
        };
        uint64_t u64;
    };
} hentry;
_Static_assert(sizeof(hentry) == sizeof(uint64_t));

#define HASH(current, value) (((current) + (value)) * 17)
#define LBLEQ(x, y) ((x).label == (y).label)

int main(int argc, char** argv)
{
    DSTOPWATCH_START(part1);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    hentry hashmap[256][64];
    uint8_t maplen[256] = {0};

    register uint64_t sum1 = 0, sum2 = 0;

    while (data < end)
    {
        uint8_t current1 = 0;
        register hentry e = { .label = 0 };
        while (*data & 0x40)
        {
            current1 = HASH(current1, *data);
            e.label = (e.label << 5) | (*data & 0x1F);
            ++data;
        }
        const uint8_t current2 = current1; // letters only
        current1 = HASH(current1, *data++);
        register hentry* entry = hashmap[current2];
        hentry* const entryEnd = entry + maplen[current2];
        if (*data & 0x10)
        {
            while (entry < entryEnd)
            {
                if (LBLEQ(*entry, e))
                {
                    entry->focalLength = *data & 0xF;
                    goto foundexisting;
                }
                ++entry;
            }
            e.focalLength = *data & 0xF;
            *entryEnd = e;
            ++maplen[current2];
        foundexisting:
            current1 = HASH(current1, *data++);
        }
        else
        {
            // YEET IT
            while (entry < entryEnd)
            {
                if (LBLEQ(*entry, e))
                {
                    entry->u64 = 0;
                    break;
                }
                ++entry;
            }
        }
        sum1 += current1;
        ++data;
    }

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    for (size_t i = 0; i < 256; ++i)
    {
        register hentry* entry = hashmap[i];
        const hentry* const entryEnd = entry + maplen[i];
        int slot = 0;
        uint64_t isum2 = 0;
        while (entry < entryEnd)
        {
            if (entry->u64)
                isum2 += (++slot) * entry->focalLength;
            ++entry;
        }
        sum2 += (isum2 * (i + 1));
    }

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

