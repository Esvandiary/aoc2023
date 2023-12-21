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

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define dataindex(y, x) ((y)*d.lineLength + (x))

#define HOLESIZE 1536
#define holeindex(y, x) (((y)*HOLESIZE) + (x))

static const uint8_t hexmap[] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 00
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 10
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 20
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 30
    0x0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 40
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 50
    0x0, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 60
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 70
};

typedef struct line
{
    chartype dir;
    uint8_t len;
    uint32_t value;
} line;

#define ED_TOP 0
#define ED_RIGHT 1
#define ED_BOTTOM 2
#define ED_LEFT 3

#define ED_OPPOSITE(x) (((x) + 2) & 3)

#define DC_NONE 0
#define DC_LEFT 1
#define DC_RIGHT 2
#define DC_SWAP 3

#define DIR_R 0
#define DIR_D 1
#define DIR_L 2
#define DIR_U 3

typedef struct move
{
    int8_t dx;
    int8_t dy;
    uint8_t newdir;
    uint8_t dirchange;
    uint8_t middirchange;
} move;

typedef struct gridentry
{
    uint8_t depth;
    uint8_t dirchange;
} gridentry;

static inline FORCEINLINE uint8_t getdir(chartype c)
{
    switch (c)
    {
        case 'L': return DIR_L;
        case 'D': return DIR_D;
        case 'R': return DIR_R;
        case 'U': return DIR_U;
    }
    return -1;
}

static inline FORCEINLINE int get_start_ed(uint8_t c)
{
    switch (c)
    {
        case DIR_L: return ED_RIGHT;
        case DIR_D: return ED_TOP;
        case DIR_R: return ED_LEFT;
        case DIR_U: return ED_BOTTOM;
    }
    return -1;
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(part1);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    line lines[2048];
    size_t lineCount = 0;

    while (data < end)
    {
        uint8_t dir = getdir(*data);
        data += 2;
        int8_t dx, dy;
        uint8_t len = 0;
        while (isdigit(*data))
        {
            len *= 10;
            len += (*data++ & 0xF);
        }
        data += 3;
        uint32_t value = 0;
        for (int i = 0; i < 6; ++i)
        {
            value <<= 4;
            value |= hexmap[*data++];
        }
        data += 2;
        lines[lineCount++] = (line) { .dir = dir, .len = len, .value = value };
    }

    move offsets[4][4] = {0};
    offsets[ED_RIGHT ][DIR_L] = (move) { .dx = -1, .dy =  0, .newdir = ED_RIGHT,  .dirchange = DC_NONE,  .middirchange = DC_NONE };
    offsets[ED_LEFT  ][DIR_R] = (move) { .dx =  1, .dy =  0, .newdir = ED_LEFT,   .dirchange = DC_NONE,  .middirchange = DC_NONE };
    offsets[ED_TOP   ][DIR_D] = (move) { .dx =  0, .dy =  1, .newdir = ED_TOP,    .dirchange = DC_SWAP,  .middirchange = DC_SWAP };
    offsets[ED_BOTTOM][DIR_U] = (move) { .dx =  0, .dy = -1, .newdir = ED_BOTTOM, .dirchange = DC_SWAP,  .middirchange = DC_SWAP };
    offsets[ED_LEFT  ][DIR_D] = (move) { .dx =  0, .dy =  1, .newdir = ED_TOP,    .dirchange = DC_RIGHT, .middirchange = DC_SWAP };
    offsets[ED_BOTTOM][DIR_L] = (move) { .dx = -1, .dy =  0, .newdir = ED_RIGHT,  .dirchange = DC_LEFT,  .middirchange = DC_NONE };
    offsets[ED_TOP   ][DIR_R] = (move) { .dx =  1, .dy =  0, .newdir = ED_LEFT,   .dirchange = DC_LEFT,  .middirchange = DC_NONE };
    offsets[ED_RIGHT ][DIR_U] = (move) { .dx =  0, .dy = -1, .newdir = ED_BOTTOM, .dirchange = DC_RIGHT, .middirchange = DC_SWAP };
    offsets[ED_LEFT  ][DIR_U] = (move) { .dx =  0, .dy = -1, .newdir = ED_BOTTOM, .dirchange = DC_LEFT,  .middirchange = DC_SWAP };
    offsets[ED_TOP   ][DIR_L] = (move) { .dx = -1, .dy =  0, .newdir = ED_RIGHT,  .dirchange = DC_RIGHT, .middirchange = DC_NONE };
    offsets[ED_BOTTOM][DIR_R] = (move) { .dx =  1, .dy =  0, .newdir = ED_LEFT,   .dirchange = DC_RIGHT, .middirchange = DC_NONE };
    offsets[ED_RIGHT ][DIR_D] = (move) { .dx =  0, .dy =  1, .newdir = ED_TOP,    .dirchange = DC_LEFT,  .middirchange = DC_SWAP };

    int64_t sum1 = 0, sum2 = 0;

    gridentry grid[HOLESIZE*HOLESIZE] = {0};
    int x = HOLESIZE/2, y = HOLESIZE/2;
    int minX = HOLESIZE, maxX = 0, minY = HOLESIZE, maxY = 0;
    int entryDir = get_start_ed(lines[0].dir);
    grid[holeindex(y, x)] = (gridentry) { .depth = 1, .dirchange = -1 }; // fix dirchange at the end
    for (size_t i = 0; i < lineCount; ++i)
    {
        move* off = &offsets[entryDir][lines[i].dir];
        grid[holeindex(y,x)].dirchange = off->dirchange;
        entryDir = off->newdir;
        for (int j = 1; j <= lines[i].len; ++j)
        {
            x += off->dx;
            y += off->dy;
            grid[holeindex(y,x)].depth = 1;
            grid[holeindex(y,x)].dirchange = off->middirchange;
        }

        minX = MIN(minX, x);
        maxX = MAX(maxX, x);
        minY = MIN(minY, y);
        maxY = MAX(maxY, y);
    }
    grid[holeindex(HOLESIZE/2,HOLESIZE/2)].dirchange = offsets[entryDir][lines[0].dir].dirchange;

    uint8_t add = 0;
    uint8_t entrydc = DC_NONE;
    for (int hy = minY; hy <= maxY; ++hy)
    {
        for (int hx = minX; hx <= maxX; ++hx)
        {
            gridentry* g = grid + holeindex(hy,hx);
            if (g->depth)
            {
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
                ++sum1;
            }
            else
            {
                sum1 += add;
                g->depth = add;
            }
        }
    }

    print_int64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);



    print_int64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

