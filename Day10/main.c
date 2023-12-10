#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define ENABLE_DEBUGLOG
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
#define DC_LEFT -1
#define DC_RIGHT 1
#define DC_SWAP 2

typedef struct move
{
    int16_t offset;
    int8_t newdir;
    int8_t dirchange;
} move;

static const char* edNames[] = { "top", "right", "bottom", "left" };
static const char* dcNamesRaw[] = { "left", "none", "right", "swap" };
static const char** dcNames = (dcNamesRaw + 1);

static const uint8_t dcS[4*4] = {
    DC_NONE,  DC_LEFT,  DC_SWAP,  DC_RIGHT, // first ED_TOP -> down
    DC_RIGHT, DC_NONE,  DC_LEFT,  DC_SWAP,  // first ED_RIGHT -> left
    DC_SWAP,  DC_RIGHT, DC_NONE,  DC_LEFT,  // first ED_BOTTOM -> up
    DC_LEFT,  DC_SWAP,  DC_RIGHT, DC_NONE,  // first ED_LEFT -> right
};

static move offsets[4][128] = {0};
static move startOffsets[4] = {0};

static uint8_t grid[65536] = {0};
static int8_t dirchanges[65536] = {0};

typedef struct fresult
{
    int distance;
    int lastEntryDir;
} fresult;

static inline FORCEINLINE fresult follow(const chartype* const start, const chartype* const s, const chartype* data, int entryDir, uint8_t gridchar)
{
    int count = 0, dirchange = 0;
    do
    {
        // DEBUGLOG("data %p, entry dir %s, char %c\n", data, edNames[entryDir], *data);
        const char c = *data;
        if (offsets[entryDir][c].offset == 0)
        {
            DEBUGLOG("hit zero offset with entryDir %s char %c after %d steps\n", edNames[entryDir], *data, count);
            return (fresult) { .distance = -1, .lastEntryDir = -1 };
        }
        grid[data - start] = gridchar;
        dirchanges[data - start] = offsets[entryDir][c].dirchange;
        data += offsets[entryDir][c].offset;
        entryDir = offsets[entryDir][c].newdir;
        ++count;
    } while (data != s);
    grid[s - start] = gridchar;
    return (fresult) { .distance = count, .lastEntryDir = entryDir };
}

int main(int argc, char** argv)
{
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

    //
    // Part 1
    //

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

    //
    // Part 2
    //

    DEBUGLOG("start newdir = %d, last entry dir = %d\n", startOffsets[gidx].newdir, result.lastEntryDir);
    dirchanges[s - file.data] = dcS[(startOffsets[gidx].newdir << 2) | result.lastEntryDir];
    DEBUGLOG("s dirchange = %s\n", dcNames[dirchanges[s - file.data]]);

    uint64_t sum2 = 0;

    uint8_t entrydc = DC_NONE;
    uint8_t add = 0;
    for (size_t gi = 0; gi < fileSize; ++gi)
    {
        if (grid[gi] == gidx + 1)
        {
            DEBUGLOG("%c", file.data[gi]);
            if (entrydc == DC_NONE && dirchanges[gi] != DC_SWAP)
                entrydc = dirchanges[gi];
            if (dirchanges[gi] != DC_NONE)
            {
                if (dirchanges[gi] == DC_SWAP)
                {
                    add = (add + 1) & 1;
                    entrydc = DC_NONE;
                }
                else if (dirchanges[gi] != entrydc)
                {
                    entrydc = DC_NONE;
                    add = (add + 1) & 1;
                }
            }
            // DEBUGLOG("[%ld] got pipe part %c, dirchange %s, add now %u\n", gi, file.data[gi], dcNames[dirchanges[gi]], add);
        }
        else
        {
            if (file.data[gi] == '\n')
                DEBUGLOG("\n");
            else
                DEBUGLOG("%c", add ? 'I' : 'O');
            sum2 += add;
            // DEBUGLOG("[%ld] got non-pipe %c; adding %u, sum now %lu\n", gi, file.data[gi] != '\n' ? file.data[gi] : '\\', add, sum2);
        }
    }

    print_uint64(sum2);

    return 0;
}

