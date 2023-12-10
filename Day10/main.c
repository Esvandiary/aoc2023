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

#define ED_TOP 0
#define ED_RIGHT 1
#define ED_BOTTOM 2
#define ED_LEFT 3

#define ED_OPPOSITE(x) (((x) + 2) & 3)

#define DC_NONE 0
#define DC_LEFT 1
#define DC_RIGHT 2
#define DC_SWAP 3

typedef struct move
{
    int16_t offset;
    uint8_t newdir;
    uint8_t dirchange;
} move;

typedef struct gridentry
{
    uint8_t idx;
    uint8_t dirchange;
} gridentry;

static const char* edNames[] = { "top", "right", "bottom", "left" };
static const char* dcNames[] = { "none", "left", "right", "swap" };

static const uint8_t dcS[4*4] = {
    DC_SWAP,  DC_LEFT,  DC_NONE,  DC_RIGHT, // first ED_TOP -> down
    DC_RIGHT, DC_SWAP,  DC_LEFT,  DC_NONE,  // first ED_RIGHT -> left
    DC_NONE,  DC_RIGHT, DC_SWAP,  DC_LEFT,  // first ED_BOTTOM -> up
    DC_LEFT,  DC_NONE,  DC_RIGHT, DC_SWAP,  // first ED_LEFT -> right
};

static move offsets[4][128] = {0};
static move startOffsets[4] = {0};

static gridentry grid[65536] = {0};

typedef struct fresult
{
    int distance;
    int lastEntryDir;
} fresult;

static inline FORCEINLINE fresult follow(const chartype* const start, const chartype* const s, const chartype* data, int entryDir, uint8_t gridchar)
{
    int count = 0;
    while (true)
    {
        // DEBUGLOG("data %p, entry dir %s, char %c\n", data, edNames[entryDir], *data);
        const char c = *data;
        if (offsets[entryDir][c].offset == 0)
        {
            DEBUGLOG("hit zero offset with entryDir %s char %c after %d steps\n", edNames[entryDir], *data, count);
            return (fresult) { .distance = -1, .lastEntryDir = -1 };
        }
        grid[data - start] = (gridentry) { .idx = gridchar, .dirchange = offsets[entryDir][c].dirchange };
        data += offsets[entryDir][c].offset;
        ++count;
        if (data == s)
            break;
        entryDir = offsets[entryDir][c].newdir;
    }
    DEBUGLOG("got to S after %d steps with entry dir %s\n", count, edNames[entryDir]);
    grid[s - start] = (gridentry) { .idx = gridchar, .dirchange = DC_NONE };
    return (fresult) { .distance = count, .lastEntryDir = entryDir };
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(init);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    const chartype* line = file.data;
    while (*line++ != '\n');
    const int lineLength = line - file.data;
    DEBUGLOG("line length %d\n", lineLength);
    while (*line != 'S')
        ++line;
    const chartype* const s = line;

    offsets[ED_RIGHT ]['-'] = (move) { .offset = -1,          .newdir = ED_RIGHT,  .dirchange = DC_NONE  };
    offsets[ED_LEFT  ]['-'] = (move) { .offset = 1,           .newdir = ED_LEFT,   .dirchange = DC_NONE  };
    offsets[ED_TOP   ]['|'] = (move) { .offset = lineLength,  .newdir = ED_TOP,    .dirchange = DC_SWAP  };
    offsets[ED_BOTTOM]['|'] = (move) { .offset = -lineLength, .newdir = ED_BOTTOM, .dirchange = DC_SWAP  };
    offsets[ED_LEFT  ]['7'] = (move) { .offset = lineLength,  .newdir = ED_TOP,    .dirchange = DC_RIGHT };
    offsets[ED_BOTTOM]['7'] = (move) { .offset = -1,          .newdir = ED_RIGHT,  .dirchange = DC_LEFT };
    offsets[ED_TOP   ]['L'] = (move) { .offset = 1,           .newdir = ED_LEFT,   .dirchange = DC_LEFT };
    offsets[ED_RIGHT ]['L'] = (move) { .offset = -lineLength, .newdir = ED_BOTTOM, .dirchange = DC_RIGHT };
    offsets[ED_LEFT  ]['J'] = (move) { .offset = -lineLength, .newdir = ED_BOTTOM, .dirchange = DC_LEFT };
    offsets[ED_TOP   ]['J'] = (move) { .offset = -1,          .newdir = ED_RIGHT,  .dirchange = DC_RIGHT };
    offsets[ED_BOTTOM]['F'] = (move) { .offset = 1,           .newdir = ED_LEFT,   .dirchange = DC_RIGHT };
    offsets[ED_RIGHT ]['F'] = (move) { .offset = lineLength,  .newdir = ED_TOP,    .dirchange = DC_LEFT };

    const int idx = (s - file.data);
    if (idx % lineLength < lineLength - 1)
    {
        DEBUGLOG("right is valid\n");
        startOffsets[0] = (move) { .offset = 1, .newdir = ED_LEFT };
    }
    if (idx % lineLength)
    {
        DEBUGLOG("left is valid\n");
        startOffsets[1] = (move) { .offset = -1, .newdir = ED_RIGHT };
    }
    if (idx + lineLength <= fileSize)
    {
        DEBUGLOG("down is valid\n");
        startOffsets[2] = (move) { .offset = lineLength, .newdir = ED_TOP };
    }
    if (idx >= lineLength)
    {
        DEBUGLOG("up is valid\n");
        startOffsets[3] = (move) { .offset = -lineLength, .newdir = ED_BOTTOM };
    }

    DSTOPWATCH_END(init);

    //
    // Part 1
    //

    DSTOPWATCH_START(part1);
    int64_t sum1 = 0;

    int gidx;
    fresult result;
    for (gidx = 0; gidx < 4; ++gidx)
    {
        if (startOffsets[gidx].offset)
        {
            DEBUGLOG("trying going %s\n", edNames[ED_OPPOSITE(startOffsets[gidx].newdir)]);
            result = follow(file.data, s, s + startOffsets[gidx].offset, startOffsets[gidx].newdir, (uint8_t)(gidx + 1));
            if (result.distance >= 0)
            {
                DEBUGLOG("found path: distance %d, last entry dir = %s\n", result.distance, edNames[result.lastEntryDir]);
                break;
            }
        }
    }
    sum1 = (result.distance + 1) / 2 + (result.distance + 1) % 2;

    print_int64(sum1);
    DSTOPWATCH_END(part1);

    //
    // Part 2
    //

    DSTOPWATCH_START(part2);
    grid[s - file.data].dirchange = dcS[(startOffsets[gidx].newdir << 2) | result.lastEntryDir];
    DEBUGLOG("s dirchange = %s\n", dcNames[grid[s - file.data].dirchange]);

    uint64_t sum2 = 0;

    const int gidx1 = gidx + 1;
    int8_t entrydc = DC_NONE;
    uint8_t add = 0;

    const gridentry* g = grid;
    const gridentry* gend = grid + fileSize;
    while (g < gend)
    {
        if (g->idx == gidx1)
        {
            DEBUGLOG("%c", file.data[g - grid]);
            switch (g->dirchange)
            {
                case DC_SWAP:
                    add = ~add & 1;
                    entrydc = DC_NONE;
                    break;
                case DC_NONE:
                    break;
                default:
                    if (entrydc == DC_NONE)
                    {
                        entrydc = g->dirchange;
                    }
                    else if (entrydc != g->dirchange)
                    {
                        add = ~add & 1;
                        entrydc = DC_NONE;
                    }
                    break;
            }
        }
        else
        {
            DEBUGLOG("%c", file.data[g - grid] != '\n' ? (add ? 'I' : 'O') : '\n');
            sum2 += add;
        }
        ++g;
    }

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(init);
    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

