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

typedef struct bpos
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
} bpos;

typedef uint32_t brickid;

typedef struct brick
{
    brickid id;
    bpos startpos;
    bpos endpos;
    brickid supportedby[8];
    uint16_t supportedbyCount;
    brickid supporting[8];
    uint16_t supportingCount;
} brick;

#define MINHEAP_NAME brick
#define MINHEAP_TYPE brick
#define MINHEAP_SCORE(b) ((b)->startpos.z)
#include "../common/minheap.h"

typedef struct fdata
{
    const chartype* const data;
    const size_t size;
} fdata;

typedef struct brickblock
{
    brickid bricks[32];
    uint32_t count;
} brickblock;

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    fdata d = {
        .data = file.data,
        .size = fileSize,
    };

    uint64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(init);

    brick bricks[2048] = {0};
    uint32_t brickCount = 1;
    brickblock brickz[512] = {0};

    while (data < end)
    {
        brick b = { .id = brickCount };
        while (isdigit(*data))
        {
            b.startpos.x *= 10;
            b.startpos.x += (*data++ & 0xF);
        }
        ++data; // ','
        while (isdigit(*data))
        {
            b.startpos.y *= 10;
            b.startpos.y += (*data++ & 0xF);
        }
        ++data; // ','
        while (isdigit(*data))
        {
            b.startpos.z *= 10;
            b.startpos.z += (*data++ & 0xF);
        }
        ++data; // '~'
        while (isdigit(*data))
        {
            b.endpos.x *= 10;
            b.endpos.x += (*data++ & 0xF);
        }
        ++data; // ','
        while (isdigit(*data))
        {
            b.endpos.y *= 10;
            b.endpos.y += (*data++ & 0xF);
        }
        ++data; // ','
        while (isdigit(*data))
        {
            b.endpos.z *= 10;
            b.endpos.z += (*data++ & 0xF);
        }
        ++data; // '\n'

        bricks[brickCount] = b;
        brickz[b.startpos.z].bricks[brickz[b.startpos.z].count++] = brickCount;
        ++brickCount;
    }

    DEBUGLOG("parsed %u bricks\n", brickCount - 1);

    DSTOPWATCH_END(init);

    DSTOPWATCH_START(part1);

    uint8_t maxz[10][10] = { 0 };
    uint32_t maxbricks[10][10] = { 0 };

    for (int z = 0; z < 512; ++z)
    {
        for (int bi = 0; bi < brickz[z].count; ++bi)
        {
            brickid bid = brickz[z].bricks[bi];
            brick b = bricks[bid];
            int cmaxz = 0;
            for (int bx = b.startpos.x; bx <= b.endpos.x; ++bx)
                for (int by = b.startpos.y; by <= b.endpos.y; ++by)
                    cmaxz = MAX(cmaxz, maxz[bx][by]);

            DEBUGLOG("brick %u @ %u,%u,%u~%u,%u,%u falling to %u\n",
                bid, b.startpos.x, b.startpos.y, b.startpos.z, b.endpos.x, b.endpos.y, b.endpos.z, cmaxz + 1);

            uint32_t supports[10] = {0};
            uint32_t supportCount = 0;
            for (int bx = b.startpos.x; bx <= b.endpos.x; ++bx)
            {
                for (int by = b.startpos.y; by <= b.endpos.y; ++by)
                {
                    if (cmaxz > 0)
                    {
                        if (maxz[bx][by] == cmaxz)
                        {
                            for (int si = 0; si < supportCount; ++si)
                            {
                                if (maxbricks[bx][by] == supports[si])
                                    goto sifound;
                            }
                            DEBUGLOG("brick %u supported by %u, support count %u\n", bid, maxbricks[bx][by], supportCount);
                            supports[supportCount++] = maxbricks[bx][by];
                        }
                    }
                sifound:
                    maxz[bx][by] = cmaxz + 1 + (b.endpos.z - b.startpos.z);
                    maxbricks[bx][by] = bid;
                }
            }

            memcpy(bricks[bid].supportedby, supports, sizeof(brickid) * supportCount);
            bricks[bid].supportedbyCount = supportCount;

            for (int i = 0; i < supportCount; ++i)
                bricks[supports[i]].supporting[bricks[supports[i]].supportingCount++] = bid;
        }
    }

    DSTOPWATCH_END(part1);

    DSTOPWATCH_START(part2);

    brick_minheap* queue = brick_minheap_init(1024);

    for (int i = 1; i < brickCount; ++i)
    {
        int64_t deleteCount = 0;

        bool baleeted[2048] = {0};
        DEBUGLOG("[%u] baleeted %u\n", i, i);
        baleeted[i] = true;
        for (int j = 0; j < bricks[i].supportingCount; ++j)
            brick_minheap_insert(queue, bricks + bricks[i].supporting[j]);

        while (queue->heap_size != 0)
        {
            brick* b = brick_minheap_extract_min(queue);
            if (baleeted[b->id])
                continue;
            if (b->supportedbyCount != 0)
            {
                bool stillalive = false;
                for (int j = 0; j < b->supportedbyCount; ++j)
                {
                    if (!baleeted[b->supportedby[j]])
                    {
                        stillalive = true;
                        break;
                    }
                }

                if (!stillalive)
                {
                    DEBUGLOG("[%u] baleeted %u\n", i, b->id);
                    ++deleteCount;
                    baleeted[b->id] = true;
                    for (int j = 0; j < b->supportingCount; ++j)
                        brick_minheap_insert(queue, bricks + b->supporting[j]);
                }
            }
        }

        if (deleteCount == 0)
            ++sum1;
        sum2 += deleteCount;
    }

    print_uint64(sum1);
    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(init);
    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

