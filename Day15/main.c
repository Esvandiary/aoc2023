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
    uint16_t lbloff;
    uint8_t lbllen;
    uint8_t focalLength;
} entry;

static entry hashmap[256][256] = {0};
static uint8_t maplen[256] = {0};

#define HASH(current, value) (((current) + (value)) * 17)
#define MAPEXISTS(hash, idx) (hashmap[hash][idx].focalLength != 0)

int main(int argc, char** argv)
{
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    for (int idx = 0; idx < fileSize; ++idx)
    {
        DEBUGLOG("idx = %u\n", idx);
        uint8_t current1 = 0;
        entry e;
        e.lbloff = idx;
        while (data[idx] != '=' && data[idx] != '-')
            current1 = HASH(current1, data[idx++]);
        e.lbllen = idx - e.lbloff;
        uint8_t current2 = current1; // letters only
        current1 = HASH(current1, data[idx++]);
        if (data[idx] != ',' && data[idx] != '\n')
        {
            e.focalLength = data[idx] & 0xF;
            for (size_t i = 0; i < maplen[current2]; ++i)
            {
                if (MAPEXISTS(current2, i) && memcmp(&data[hashmap[current2][i].lbloff], &data[e.lbloff], e.lbllen) == 0)
                {
                    hashmap[current2][i].focalLength = e.focalLength;
                    goto foundexisting;
                }
            }
            hashmap[current2][maplen[current2]++] = e;
        foundexisting:
            current1 = HASH(current1, data[idx]);
            ++idx;
        }
        else
        {
            // YEET IT
            for (uint8_t i = 0; i < maplen[current2]; ++i)
            {
                if (MAPEXISTS(current2, i) && memcmp(&data[hashmap[current2][i].lbloff], &data[e.lbloff], e.lbllen) == 0)
                {
                    memset(&hashmap[current2][i], 0, sizeof(entry));
                }
            }
        }
        sum1 += current1;
    }

    print_uint64(sum1);

    for (size_t i = 0; i < 256; ++i)
    {
        int slot = 1;
        for (size_t j = 0; j < maplen[i]; ++j)
        {
            if (MAPEXISTS(i, j))
                sum2 += (i+1) * (slot++) * hashmap[i][j].focalLength;
        }
    }

    print_uint64(sum2);
    DSTOPWATCH_END(logic);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

