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

static int32_t galaxies[1024];
static uint8_t widths1[512] = { [0 ... 511] = 2 };
static uint8_t heights1[512] = { [0 ... 511] = 2 };
static uint32_t widths2[512] = { [0 ... 511] = 1000000 };
static uint32_t heights2[512] = { [0 ... 511] = 1000000 };
static int16_t ydistances1[512] = { 0 };
static int32_t ydistances2[512] = { 0 };
static int16_t xdistances1[512] = { 0 };
static int32_t xdistances2[512] = { 0 };

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

    const int ymax = (fileSize / lineLength) + 1;
    int yd1 = 0, yd2 = 0;
    for (int y = 0; y < ymax; ++y)
    {
        yd1 += heights1[y];
        yd2 += heights2[y];
        ydistances1[y] = yd1;
        ydistances2[y] = yd2;
    }
    int xd1 = 0, xd2 = 0;
    for (int x = 0; x < lineLength - 1; ++x)
    {
        xd1 += widths1[x];
        xd2 += widths2[x];
        xdistances1[x] = xd1;
        xdistances2[x] = xd2;
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
                sum1 += abs(ydistances1[y2] - ydistances1[y1]);
                sum2 += abs(ydistances2[y2] - ydistances2[y1]);
            }
            const int x1 = dataX(galaxies[i]), x2 = dataX(galaxies[j]);
            if (x1 != x2)
            {
                sum1 += abs(xdistances1[x2] - xdistances1[x1]);
                sum2 += abs(xdistances2[x2] - xdistances2[x1]);
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

