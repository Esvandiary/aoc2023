#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "astar.h"
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define dataindex(y, x) ((y)*d.lineLength + (x))

typedef struct fdata
{
    const chartype* const data;
    const size_t size;
    const int32_t lineLength;
    const int32_t lineCount;
} fdata;

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
    };

    int64_t sum1 = 0, sum2 = 0;

    astar_node* nodes = (astar_node*)malloc(sizeof(astar_node) * lineLength * lineCount * 4);
    for (int y = 0; y < lineCount; ++y)
    {
        for (int x = 0; x < lineLength - 1; ++x)
        {
            for (int dir = 0; dir < 4; ++dir)
            {
                nodes[nodeindex(lineCount, lineLength, y, x, dir)] = (astar_node) {
                    .pos = (astar_pos) {.y = y, .x = x },
                    .dir = dir,
                    .heatloss = d.data[dataindex(y, x)] & 0xF,
                    .fScore = INT64_MAX,
                    .traversed = false,
                };
            }
        }
    }

    sum1 = calculate(
        nodes,
        lineCount,
        lineLength,
        (astar_pos) { .y = 0, .x = 0 },
        (astar_pos) { .y = lineCount - 1, .x = lineLength - 2 },
        1,
        3);

    print_int64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    for (int i = 0; i < lineLength * lineCount * 4; ++i)
    {
        nodes[i].traversed = false;
        nodes[i].fScore = INT64_MAX;
    }

    sum2 = calculate(
        nodes,
        lineCount,
        lineLength,
        (astar_pos) { .y = 0, .x = 0 },
        (astar_pos) { .y = lineCount - 1, .x = lineLength - 2 },
        4,
        10);

    print_int64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

