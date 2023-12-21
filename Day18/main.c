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
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    int64_t sum1 = 0, sum2 = 0;

    int64_t p1x = 0, p1y = 0, p1px = 0, p1py = 0;
    int64_t p1i = 0, p1b = 0;
    int64_t p2x = 0, p2y = 0, p2px = 0, p2py = 0;
    int64_t p2i = 0, p2b = 0;

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

        switch (dir)
        {
            case DIR_R: p1x += len; break;
            case DIR_D: p1y += len; break;
            case DIR_L: p1x -= len; break;
            case DIR_U: p1y -= len; break;
        }
        p1i += (p1px * p1y - p1x * p1py);
        p1b += len;
        p1px = p1x; p1py = p1y;

        switch (value & 0xF)
        {
            case DIR_R: p2x += (value >> 4); break;
            case DIR_D: p2y += (value >> 4); break;
            case DIR_L: p2x -= (value >> 4); break;
            case DIR_U: p2y -= (value >> 4); break;
        }
        p2i += (p2px * p2y - p2x * p2py);
        p2b += (value >> 4);
        p2px = p2x; p2py = p2y;
    }

    sum1 = (p1i + p1b) / 2 + 1;
    sum2 = (p2i + p2b) / 2 + 1;

    print_int64(sum1);
    print_int64(sum2);
    DSTOPWATCH_END(logic);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

