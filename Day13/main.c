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

static inline FORCEINLINE int memsinglediff(uint8_t* p1, uint8_t* p2, size_t len)
{
    int result = -1;
    for (size_t idx = 0; idx < len; ++idx)
    {
        if (p1[idx] != p2[idx])
        {
            if (result == -1)
            {
                result = idx;
            }
            else
                return -1;
        }
    }
    return result;
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    while (data < end)
    {
        uint8_t ngrid[128][128];
        uint8_t fgrid[128][128];
        int8_t ynum[128] = {0};
        int8_t xnum[128] = {0};

        DEBUGLOG("===== part 1 =====\n");

        int yidx = 0, xidx = 0;
        while (data < end && *data != '\n')
        {
            xidx = 0;
            while (*data != '\n')
            {
                const uint8_t value = (~(*data >> 3)) & 1;
                ngrid[yidx][xidx] = value;
                fgrid[xidx][yidx] = value;
                ynum[yidx] += value;
                xnum[xidx] += value;
                ++xidx;
                ++data;
            }
            ++yidx;
            ++data; // '\n'
        }
        ++data; // '\n'

        for (int y = 0; y + 1 < yidx; ++y)
        {
            const int maxdy = MIN(y, yidx - y - 2);
            for (int dy = 0; dy <= maxdy; ++dy)
            {
                if (ynum[y - dy] != ynum[y + 1 + dy] || memcmp(ngrid[y - dy], ngrid[y + 1 + dy], sizeof(uint8_t) * xidx) != 0)
                {
                    DEBUGLOG("y %d failed at dy %d\n", y, dy);
                    goto p1nexty;
                }
            }
            DEBUGLOG("y succeeded at %d\n", y);
            sum1 += 100 * (y + 1);
            goto p1skip;
        p1nexty:
        }
        for (int x = 0; x + 1 < xidx; ++x)
        {
            const int maxdx = MIN(x, xidx - x - 2);
            DEBUGLOG("x %d, maxdx %d\n", x, maxdx);
            for (int dx = 0; dx <= maxdx; ++dx)
            {
                if (xnum[x - dx] != xnum[x + 1 + dx] || memcmp(fgrid[x - dx], fgrid[x + 1 + dx], sizeof(uint8_t) * yidx) != 0)
                {
                    DEBUGLOG("x %d failed at dx %d\n", x, dx);
                    goto p1nextx;
                }
            }
            DEBUGLOG("x succeeded at %d\n", x);
            sum1 += (x + 1);
            goto p1skip;
        p1nextx:
        }
    p1skip:

        DEBUGLOG("===== part 2 =====\n");
        for (int y = 0; y + 1 < yidx; ++y)
        {
            const int maxdy = MIN(y, yidx - y - 2);
            int offdy = -1, offdx = -1;
            for (int dy = 0; dy <= maxdy; ++dy)
            {
                int yoff = abs(ynum[y - dy] - ynum[y + 1 + dy]);
                int xoffidx;
                if (yoff == 1 && (xoffidx = memsinglediff(ngrid[y - dy], ngrid[y + 1 + dy], xidx)) >= 0)
                {
                    if (offdy >= 0)
                        goto p2nexty;
                    offdy = dy;
                    offdx = xoffidx;
                    DEBUGLOG("yoff 1 for y %d at dy %d dx %d\n", y, offdy, offdx);
                }
                else if (yoff != 0 || memcmp(ngrid[y - dy], ngrid[y + 1 + dy], sizeof(uint8_t) * xidx) != 0)
                {
                    goto p2nexty;
                }
            }
            if (offdy >= 0)
            {
                DEBUGLOG("p2 y candidate at %d,%d\n", y, offdx);
                uint8_t* yd1 = ngrid[y - offdy];
                uint8_t* yd2 = ngrid[y + 1 + offdy];

                // switch one bit, and run p1
                yd1[offdx] = (~yd1[offdx]) & 1;
                int result = memcmp(yd1, yd2, xidx);
                yd1[offdx] = (~yd1[offdx]) & 1;
                DEBUGLOG("result = %d\n", result);
                if (result == 0)
                {
                    DEBUGLOG("p2 y match at %d\n", y);
                    sum2 += (100 * (y + 1));
                    goto p2skip;
                }
            }
        p2nexty:
        }
        for (int x = 0; x + 1 < xidx; ++x)
        {
            const int maxdx = MIN(x, xidx - x - 2);
            int offdx = -1, offdy = -1;
            for (int dx = 0; dx <= maxdx; ++dx)
            {
                int xoff = abs(xnum[x - dx] - xnum[x + 1 + dx]);
                int yoffidx;
                if (xoff == 1 && (yoffidx = memsinglediff(fgrid[x - dx], fgrid[x + 1 + dx], yidx)) >= 0)
                {
                    if (offdx >= 0)
                        goto p2nextx;
                    offdx = dx;
                    offdy = yoffidx;
                    DEBUGLOG("xoff 1 for x %d at dy %d dx %d\n", x, offdy, offdx);
                }
                else if (xoff != 0 || memcmp(fgrid[x - dx], fgrid[x + 1 + dx], sizeof(uint8_t) * yidx) != 0)
                {
                    goto p2nextx;
                }
            }
            if (offdx >= 0)
            {
                DEBUGLOG("p2 x candidate at %d,%d\n", offdy, x);
                uint8_t* xd1 = fgrid[x - offdx];
                uint8_t* xd2 = fgrid[x + 1 + offdx];

                // switch one bit, and run p1
                xd1[offdy] = (~xd1[offdy]) & 1;
                int result = memcmp(xd1, xd2, yidx);
                xd1[offdy] = (~xd1[offdy]) & 1;
                if (result == 0)
                {
                    DEBUGLOG("p2 x match at %d\n", x);
                    sum2 += (x + 1);
                    goto p2skip;
                }
            }
        p2nextx:
        }
    p2skip:
    }

    DSTOPWATCH_END(logic);

    print_uint64(sum1);
    print_uint64(sum2);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

