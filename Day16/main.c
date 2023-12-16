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

#define ED_BAIL 1
#define ED_TOP 2
#define ED_RIGHT 3
#define ED_BOTTOM 4
#define ED_LEFT 5

typedef struct fdata
{
    const chartype* const data;
    const size_t size;
    const int32_t lineLength;
    const int32_t lineCount;
    const int32_t entryOffsets[6];
} fdata;

#define GRID_SIZE 128*128

#define ENERGISED(idx) (grid[idx] & 1U)
#define NEEDS_CALCULATE(idx, size, dir) ((idx) >= 0 && (idx) < (size) && (grid[idx] & (1U << (dir))) == 0)

uint64_t calculate(const fdata d, uint8_t* grid, int64_t idx, uint8_t entryDir)
{
    // DEBUGLOG("calculate(d, %lu, %u)\n", idx, entryDir);
    uint64_t result = 0;
    while (idx >= 0 && idx < d.size)
    {
        grid[idx] |= (1U << entryDir);
        switch (d.data[idx])
        {
            case '\\':
                switch (entryDir)
                {
                    case ED_TOP: entryDir = ED_LEFT; break;
                    case ED_RIGHT: entryDir = ED_BOTTOM; break;
                    case ED_BOTTOM: entryDir = ED_RIGHT; break;
                    case ED_LEFT: entryDir = ED_TOP; break;
                }
                break;
            case '/':
                switch (entryDir)
                {
                    case ED_TOP: entryDir = ED_RIGHT; break;
                    case ED_RIGHT: entryDir = ED_TOP; break;
                    case ED_BOTTOM: entryDir = ED_LEFT; break;
                    case ED_LEFT: entryDir = ED_BOTTOM; break;
                }
                break;
            case '|':
                if (entryDir == ED_TOP || entryDir == ED_BOTTOM)
                    break;
                if (NEEDS_CALCULATE(idx + d.entryOffsets[ED_TOP], d.size, ED_TOP))
                    result += calculate(d, grid, idx + d.entryOffsets[ED_TOP], ED_TOP);
                if (NEEDS_CALCULATE(idx + d.entryOffsets[ED_BOTTOM], d.size, ED_BOTTOM))
                    result += calculate(d, grid, idx + d.entryOffsets[ED_BOTTOM], ED_BOTTOM);
                entryDir = ED_BAIL;
                break;
            case '-':
                if (entryDir == ED_LEFT || entryDir == ED_RIGHT)
                    break;
                if (NEEDS_CALCULATE(idx + d.entryOffsets[ED_LEFT], d.size, ED_LEFT))
                    result += calculate(d, grid, idx + d.entryOffsets[ED_LEFT], ED_LEFT);
                if (NEEDS_CALCULATE(idx + d.entryOffsets[ED_RIGHT], d.size, ED_RIGHT))
                    result += calculate(d, grid, idx + d.entryOffsets[ED_RIGHT], ED_RIGHT);
                entryDir = ED_BAIL;
                break;
            case '.':
                break;
            case '\n':
                goto end;
        }
        if (!ENERGISED(idx))
        {
            grid[idx] |= 1;
            ++result;
        }
        // DEBUGLOG("did %c @ %ld,%ld; idx = %ld --> %ld, entrydir %d, result %ld\n", d.data[idx], idx / d.lineLength, idx % d.lineLength, idx, idx + d.entryOffsets[entryDir], entryDir, result);
        idx += d.entryOffsets[entryDir];
    }
end:
    // DEBUGLOG("calculate --> %lu\n", result);
    return result;
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(part1);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    while (*data++ != '\n');
    const int32_t lineLength = data - file.data;
    const int32_t lineCount = (file.size + lineLength / 2) / lineLength;
    DEBUGLOG("lineLength = %u, lineCount = %u\n", lineLength, lineCount);

    fdata d = {
        .data = file.data,
        .size = fileSize,
        .lineLength = lineLength,
        .lineCount = lineCount,
        .entryOffsets = {0, INT32_MAX, lineLength, -1, -lineLength, 1}
    };

    uint64_t sum1 = 0, sum2 = 0;

    uint8_t grid[GRID_SIZE] = {0};
    sum1 = calculate(d, grid, 0, ED_LEFT);

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    // from the top, from the bottom
    for (int x = 0; x < lineLength - 1; ++x)
    {
        uint8_t grid2[GRID_SIZE] = {0};
        int64_t idx = x;
        uint64_t result = calculate(d, grid2, x, ED_TOP);
        sum2 = MAX(sum2, result);
    }
    for (int x = 0; x < lineLength - 1; ++x)
    {
        uint8_t grid2[GRID_SIZE] = {0};
        int64_t idx = lineLength * lineCount - 1 - x;
        uint64_t result = calculate(d, grid2, idx, ED_BOTTOM);
        sum2 = MAX(sum2, result);
    }
    // from the window, from the WAAALLLLLLLL
    for (int y = 0; y < lineCount; ++y)
    {
        uint8_t grid2[GRID_SIZE] = {0};
        int64_t idx = y * lineLength;
        uint64_t result = calculate(d, grid2, idx, ED_LEFT);
        sum2 = MAX(sum2, result);
    }
    for (int y = 0; y < lineCount; ++y)
    {
        uint8_t grid2[GRID_SIZE] = {0};
        int64_t idx = (y+1)*lineLength - 2;
        uint64_t result = calculate(d, grid2, idx, ED_RIGHT);
        sum2 = MAX(sum2, result);
    }

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

