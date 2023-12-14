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
#include "../common/vuctor.h"
#include "../common/uthash/uthash.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y,x) ((y) * lineLength + (x))

static inline FORCEINLINE int calculateLoad(int ymax, int lastStop, int rockCount)
{
    const int minRockNum = ymax - (lastStop + 1);
    const int maxRockNum = ymax - (lastStop + rockCount);
    const int result = rockCount * (minRockNum + maxRockNum) / 2;
    // DEBUGLOG("calculateLoad(%d, %d, %d) = %d\n", ymax, lastStop, rockCount, result);
    return result;
}

static inline FORCEINLINE int calculateNorthLoad(chartype* data, int lineLength, int lineCount)
{
    int sum = 0;
    for (int y = 0; y < lineCount; ++y)
    {
        for (int x = 0; x < lineLength; ++x)
        {
            if (*data++ == 'O')
                sum += (lineCount - y);
        }
    }
    return sum;
}

static inline FORCEINLINE int calculatePart1Load(chartype* const data, int lineLength, int lineCount)
{
    int sum = 0;
    for (int x = 0; x < lineLength - 1; ++x)
    {
        int lastStop = -1;
        int curRocks = 0;
        for (int y = 0; y < lineCount; ++y)
        {
            switch (data[dataindex(y, x)])
            {
                case '.':
                    break;
                case '#':
                    if (curRocks > 0)
                        sum += calculateLoad(lineCount, lastStop, curRocks);
                    lastStop = y;
                    curRocks = 0;
                    break;
                case 'O':
                    ++curRocks;
                    break;
            }
        }
        if (curRocks > 0)
            sum += calculateLoad(lineCount, lastStop, curRocks);
    }
    return sum;
}

static inline void letsRotateTheBoard(chartype* const data, int lineLength, int lineCount)
{
    // north
    for (int x = 0; x < lineLength - 1; ++x)
    {
        int lastStop = -1;
        int curRocks = 0;
        for (int y = 0; y < lineCount; ++y)
        {
            switch (data[dataindex(y, x)])
            {
                case '.':
                    break;
                case '#':
                    if (curRocks > 0)
                    {
                        int cy;
                        for (cy = lastStop + 1; cy < lastStop + 1 + curRocks; ++cy)
                            data[dataindex(cy,x)] = 'O';
                        for (; cy < y; ++cy)
                            data[dataindex(cy,x)] = '.';
                    }
                    lastStop = y;
                    curRocks = 0;
                    break;
                case 'O':
                    ++curRocks;
                    break;
            }
        }
        if (curRocks > 0)
        {
            int cy;
            for (cy = lastStop + 1; cy < lastStop + 1 + curRocks; ++cy)
                data[dataindex(cy,x)] = 'O';
            for (; cy < lineCount; ++cy)
                data[dataindex(cy,x)] = '.';
        }
    }
    // west
    for (int y = 0; y < lineCount; ++y)
    {
        int lastStop = -1;
        int curRocks = 0;
        for (int x = 0; x < lineLength - 1; ++x)
        {
            switch (data[dataindex(y, x)])
            {
                case '.':
                    break;
                case '#':
                    if (curRocks > 0)
                    {
                        int cx;
                        for (cx = lastStop + 1; cx < lastStop + 1 + curRocks; ++cx)
                            data[dataindex(y,cx)] = 'O';
                        for (; cx < x; ++cx)
                            data[dataindex(y,cx)] = '.';
                    }
                    lastStop = x;
                    curRocks = 0;
                    break;
                case 'O':
                    ++curRocks;
                    break;
            }
        }
        if (curRocks > 0)
        {
            int cx;
            for (cx = lastStop + 1; cx < lastStop + 1 + curRocks; ++cx)
                data[dataindex(y,cx)] = 'O';
            for (; cx < lineCount; ++cx)
                data[dataindex(y,cx)] = '.';
        }
    }
    // south
    for (int x = 0; x < lineLength - 1; ++x)
    {
        int lastStop = lineCount;
        int curRocks = 0;
        for (int y = lineCount - 1; y >= 0; --y)
        {
            switch (data[dataindex(y, x)])
            {
                case '.':
                    break;
                case '#':
                    if (curRocks > 0)
                    {
                        int cy;
                        for (cy = lastStop - 1; cy > lastStop - 1 - curRocks; --cy)
                            data[dataindex(cy,x)] = 'O';
                        for (; cy > y; --cy)
                            data[dataindex(cy,x)] = '.';
                    }
                    lastStop = y;
                    curRocks = 0;
                    break;
                case 'O':
                    ++curRocks;
                    break;
            }
        }
        if (curRocks > 0)
        {
            int cy;
            for (cy = lastStop - 1; cy > lastStop - 1 - curRocks; --cy)
                data[dataindex(cy,x)] = 'O';
            for (; cy >= 0; --cy)
                data[dataindex(cy,x)] = '.';
        }
    }
    // east
    for (int y = 0; y < lineCount; ++y)
    {
        int lastStop = lineLength - 1;
        int curRocks = 0;
        for (int x = lineLength - 2; x >= 0; --x)
        {
            switch (data[dataindex(y, x)])
            {
                case '.':
                    break;
                case '#':
                    if (curRocks > 0)
                    {
                        int cx;
                        for (cx = lastStop - 1; cx > lastStop - 1 - curRocks; --cx)
                            data[dataindex(y,cx)] = 'O';
                        for (; cx > x; --cx)
                            data[dataindex(y,cx)] = '.';
                    }
                    lastStop = x;
                    curRocks = 0;
                    break;
                case 'O':
                    ++curRocks;
                    break;
            }
        }
        if (curRocks > 0)
        {
            int cx;
            for (cx = lastStop - 1; cx > lastStop - 1 - curRocks; --cx)
                data[dataindex(y,cx)] = 'O';
            for (; cx >= 0; --cx)
                data[dataindex(y,cx)] = '.';
        }
    }
}

static chartype boardCache[256*256];

#define INIT_COUNT 1000
#define TEST_COUNT 100
#define END_ITER   1000000000

int main(int argc, char** argv)
{
    DSTOPWATCH_START(init);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    while (*data++ != '\n');
    const int lineLength = data - file.data;
    const int lineCount = (file.size + lineLength / 2) / lineLength;
    DEBUGLOG("grid size %dx%d\n", lineCount, lineLength - 1);
    data = file.data;

    DSTOPWATCH_END(init);

    DSTOPWATCH_START(part1);

    sum1 = calculatePart1Load(file.data, lineLength, lineCount);

    print_uint64(sum1);

    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    // always run some iters to let things settle
    int iter;
    for (iter = 0; iter < INIT_COUNT; ++iter)
    {
        letsRotateTheBoard(file.data, lineLength, lineCount);
    }

    // run some iters to find a pattern
    int loads[TEST_COUNT];
    for (; iter < INIT_COUNT + TEST_COUNT; ++iter)
    {
        letsRotateTheBoard(file.data, lineLength, lineCount);
        loads[iter - INIT_COUNT] = calculateNorthLoad(file.data, lineLength, lineCount);
    }

    int patternLen = -1;
    for (int i = 1; i < TEST_COUNT/2; ++i)
    {
        for (int j = 0; j < TEST_COUNT/i; ++j)
        {
            if (memcmp(loads + i*j, loads, sizeof(int) * i) != 0)
                goto nextiter;
        }
        patternLen = i;
        break;
    nextiter:
    }

    DEBUGLOG("pattern length = %d\n", patternLen);
    for (; ((END_ITER - iter) % patternLen) != 0; ++iter)
    {
        letsRotateTheBoard(file.data, lineLength, lineCount);
    }

    sum2 = calculateNorthLoad(file.data, lineLength, lineCount);

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(init);
    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

