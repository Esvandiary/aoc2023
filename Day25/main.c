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
#include "karger.h"

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
            components[c1].connections[components[c1].connectionsCount++] = components[cn].listidx;
            components[cn].connections[components[cn].connectionsCount++] = components[c1].listidx;
        }
    }

    DEBUGLOG("parsed %d components\n", componentCount);

    DSTOPWATCH_END(init);

    int64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(part1);

    ks_edgegraph* const graph = (ks_edgegraph*)calloc(1, sizeof(ks_edgegraph));
    graph->nvertices = componentCount;
    graph->edgesCount = 0;

    for (int i = 0; i < componentCount; ++i)
    {
        for (int j = 0; j < components[componentList[i]].connectionsCount; ++j)
        {
            const uint32_t clidx = components[componentList[i]].connections[j];
            if (clidx > i)
            {
                graph->edges[graph->edgesCount++] = (ks_edge) { .head = i, .tail = clidx };
                // graph->edges[graph->edgesCount++] = (ks_edge) { .head = clidx, .tail = i };
            }
        }
    }

    DEBUGLOG("going to perform\n");
    ks_graphcut* const gc = (ks_graphcut*)calloc(1, sizeof(ks_graphcut));
    while (gc->cut_size != 3)
    {
        memset(gc, 0, sizeof(*gc));
        ks_perform(graph, gc);
    }

    const ks_psresult result = ks_graphcut_get_partition_sizes(gc);
    DEBUGLOG("cut size 3 with sizes %u, %u\n", result.partition1Size, result.partition2Size);

    sum1 = result.partition1Size * result.partition2Size;

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

