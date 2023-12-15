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

typedef struct entry
{
    uint8_t focalLength;
    chartype label[7];
} entry;
typedef struct hentry
{
    union
    {
        entry entry;
        uint64_t u64;
    };
} hentry;
_Static_assert(sizeof(entry) == sizeof(uint64_t));
_Static_assert(sizeof(hentry) == sizeof(uint64_t));

static hentry hashmap[256][256] = {0};
static uint8_t maplen[256] = {0};

#define HASH(current, value) (((current) + (value)) * 17)
#define LBLEQ(x, y) (((x).u64 ^ (y).u64) <= 0xFF)

int main(int argc, char** argv)
{
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    while (data < end)
    {
        uint8_t current1 = 0;
        hentry e = { .u64 = 0 };
        chartype* lbl = e.entry.label;
        while (*data & 0x40)
        {
            current1 = HASH(current1, *data);
            *lbl++ = *data++;
        }
        uint8_t current2 = current1; // letters only
        current1 = HASH(current1, *data++);
        if (*data & 0x10)
        {
            e.entry.focalLength = *data & 0xF;
            hentry* entry = hashmap[current2];
            hentry* entryEnd = hashmap[current2] + maplen[current2];
            while (entry < entryEnd)
            {
                if (LBLEQ(*entry, e))
                {
                    entry->entry.focalLength = e.entry.focalLength;
                    goto foundexisting;
                }
                ++entry;
            }
            *entryEnd = e;
            ++maplen[current2];
        foundexisting:
            current1 = HASH(current1, *data++);
        }
        else
        {
            // YEET IT
            hentry* entry = hashmap[current2];
            hentry* entryEnd = hashmap[current2] + maplen[current2];
            while (entry < entryEnd)
            {
                if (LBLEQ(*entry, e))
                {
                    entry->u64 = 0;
                }
                ++entry;
            }
        }
        sum1 += current1;
        ++data;
    }

    print_uint64(sum1);

    for (size_t i = 0; i < 256; ++i)
    {
        hentry* entry = hashmap[i];
        hentry* entryEnd = hashmap[i] + maplen[i];
        int slot = 1;
        uint64_t isum2 = 0;
        while (entry < entryEnd)
        {
            if (entry->u64)
                isum2 += (slot++) * entry->entry.focalLength;
            ++entry;
        }
        sum2 += (isum2 * (i + 1));
    }

    print_uint64(sum2);
    DSTOPWATCH_END(logic);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

