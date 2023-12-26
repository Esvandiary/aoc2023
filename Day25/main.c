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
#include "stoerwagner.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct component
{
    uint16_t listidx;
    uint16_t connectionsCount;
    uint16_t connections[14];
} component;

static component components[32*32*32] = {0};

static uint16_t componentList[2048] = {0};
static uint32_t componentCount = 0;

int main(int argc, char** argv)
{
    DSTOPWATCH_START(init);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    while (data < end)
    {
        uint16_t c1 = ((*data++ & 0x1F) << 10) | ((*data++ & 0x1F) << 5) | (*data++ & 0x1F);
        if (components[c1].connectionsCount == 0)
        {
            componentList[componentCount] = c1;
            components[c1].listidx = componentCount++;
        }
        ++data; // ':'
        while (*data++ != '\n')
        {
            uint16_t cn = ((*data++ & 0x1F) << 10) | ((*data++ & 0x1F) << 5) | (*data++ & 0x1F);
            if (components[cn].connectionsCount == 0)
            {
                componentList[componentCount] = cn;
                components[cn].listidx = componentCount++;
            }
            components[c1].connections[components[c1].connectionsCount++] = cn;
            components[cn].connections[components[cn].connectionsCount++] = c1;
        }
    }

    DEBUGLOG("parsed %d components\n", componentCount);

    DSTOPWATCH_END(init);

    int64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(part1);

    sw_state* state = (sw_state*)malloc(sizeof(sw_state));
    sw_init(state, componentCount);

    for (int i = 0; i < componentCount; ++i)
    {
        for (int j = 0; j < components[componentList[i]].connectionsCount; ++j)
        {
            uint32_t clidx = components[components[componentList[i]].connections[j]].listidx;
            state->edge[i][clidx] = 1;
            state->edge[clidx][i] = 1;
        }
    }

    int mc = sw_perform(state);
    int mincut = (mc & 0xFF);
    int t = (mc >> 8);
    DEBUGLOG("mincut = %d, t = %d (nodes x%d)\n", mincut, t, state->co_count[t]);

    sum1 = state->co_count[t] * (componentCount - state->co_count[t]);

    print_int64(sum1);
    DSTOPWATCH_END(part1);
    // DSTOPWATCH_START(part2);

    // print_int64(sum2);
    // DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(init);
    DSTOPWATCH_PRINT(part1);
    // DSTOPWATCH_PRINT(part2);

    return 0;
}

