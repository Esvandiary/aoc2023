#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y,x) ((y) * lineLength + (x))
#define dataY(idx) ((idx) / lineLength)
#define dataX(idx) ((idx) % lineLength)

static int32_t galaxies[256];
static uint8_t widths1[512] = { [0 ... 511] = 2 };
static uint8_t heights1[512] = { [0 ... 511] = 2 };
static uint32_t widths2[512] = { [0 ... 511] = 1000000 };
static uint32_t heights2[512] = { [0 ... 511] = 1000000 };

int main(int argc, char** argv)
{
    DSTOPWATCH_START(init);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const chartype* line = file.data;
    while (*line++ != '\n');
    const int lineLength = line - file.data;
    DEBUGLOG("line length %d\n", lineLength);

    size_t galaxyCount = 0;

    const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    while (data < end)
    {
        const char c = *data;
        if (c == '#')
        {
            const uint32_t idx = data - file.data;
            DEBUGLOG("galaxy at %d,%d (%u)\n", dataY(idx), dataX(idx), idx);
            heights1[dataY(idx)] = heights2[dataY(idx)] = 1;
            widths1[dataX(idx)] = widths2[dataX(idx)] = 1;
            galaxies[galaxyCount++] = idx;
        }
        ++data;
    }

    DSTOPWATCH_END(init);

    //
    // Part 1 + 2
    //

    DSTOPWATCH_START(logic);
    int64_t sum1 = 0, sum2 = 0;

    for (int i = 0; i < galaxyCount - 1; ++i)
    {
        for (int j = i + 1; j < galaxyCount; ++j)
        {
            const int y1 = dataY(galaxies[i]), y2 = dataY(galaxies[j]);
            if (y1 != y2)
            {
                const int startY = MIN(y1, y2);
                const int endY = MAX(y1, y2);
                for (int cy = startY; cy < endY; ++cy)
                {
                    sum1 += heights1[cy];
                    sum2 += heights2[cy];
                }
            }
            const int x1 = dataX(galaxies[i]), x2 = dataX(galaxies[j]);
            if (x1 != x2)
            {
                const int startX = MIN(x1, x2);
                const int endX = MAX(x1, x2);
                for (int cx = startX; cx < endX; ++cx)
                {
                    sum1 += widths1[cx];
                    sum2 += widths2[cx];
                }
            }
        }
    }

    print_int64(sum1);
    print_int64(sum2);
    DSTOPWATCH_END(logic);

    DSTOPWATCH_PRINT(init);
    DSTOPWATCH_PRINT(logic);

    return 0;
}

