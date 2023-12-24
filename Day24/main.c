#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct hailstone
{
    int64_t x;
    int64_t y;
    int64_t z;
    int64_t dx;
    int64_t dy;
    int64_t dz;
} hailstone;

int main(int argc, char** argv)
{
    DSTOPWATCH_START(init);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);


    const int64_t teststartx = 200000000000000LL;
    const int64_t teststarty = 200000000000000LL;
    const int64_t testendx = 400000000000000LL;
    const int64_t testendy = 400000000000000LL;

/*
    const int64_t teststartx = 7;
    const int64_t teststarty = 7;
    const int64_t testendx = 27;
    const int64_t testendy = 27;
*/
    register chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    hailstone stones[512];
    int stonesCount = 0;

    while (data < end)
    {
        hailstone s = {0};
        while (isdigit(*data))
        {
            s.x *= 10;
            s.x += (*data++ & 0xF);
        }
        data += 2; // ', '
        while (isdigit(*data))
        {
            s.y *= 10;
            s.y += (*data++ & 0xF);
        }
        data += 2;
        while (isdigit(*data))
        {
            s.z *= 10;
            s.z += (*data++ & 0xF);
        }
        data += 3; // ' @ '
        int dxmul = 1;
        if (*data == '-')
        {
            dxmul = -1;
            ++data;
        }
        while (isdigit(*data))
        {
            s.dx *= 10;
            s.dx += (*data++ & 0xF);
        }
        s.dx *= dxmul;
        data += 2; // ', '
        int dymul = 1;
        if (*data == '-')
        {
            dymul = -1;
            ++data;
        }
        while (isdigit(*data))
        {
            s.dy *= 10;
            s.dy += (*data++ & 0xF);
        }
        s.dy *= dymul;
        data += 2; // ', '
        int dzmul = 1;
        if (*data == '-')
        {
            dzmul = -1;
            ++data;
        }
        while (isdigit(*data))
        {
            s.dz *= 10;
            s.dz += (*data++ & 0xF);
        }
        s.dz *= dzmul;
        ++data; // '\n'

        stones[stonesCount++] = s;
    }

    DEBUGLOG("parsed %d stones\n", stonesCount);

    DSTOPWATCH_END(init);

    int64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(part1);

    int64_t intersections[512] = {[0 ... 511] = INT64_MAX};

    for (int i = 0; i < stonesCount - 1; ++i)
    {
        for (int j = i + 1; j < stonesCount; ++j)
        {
            if (stones[i].dx == stones[j].dx && stones[i].dy == stones[j].dy)
                continue;

            __int128_t x1 = stones[i].x; __int128_t x2 = stones[i].x + stones[i].dx;
            __int128_t x3 = stones[j].x; __int128_t x4 = stones[j].x + stones[j].dx;
            __int128_t y1 = stones[i].y; __int128_t y2 = stones[i].y + stones[i].dy;
            __int128_t y3 = stones[j].y; __int128_t y4 = stones[j].y + stones[j].dy;
            
            double px = (double)((x1*y2 - y1*x2)*(x3-x4) - (x1-x2)*(x3*y4 - y3*x4)) / (double)((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
            double py = (double)((x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4 - y3*x4)) / (double)((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

            double t1x = (px - x1) / stones[i].dx;
            double t1y = (py - y1) / stones[i].dy;
            double t2x = (px - x3) / stones[j].dx;
            double t2y = (py - y3) / stones[j].dy;
            bool t1future = (t1x >= 0 && t1y >= 0);
            bool t2future = (t2x >= 0 && t2y >= 0);

            // DEBUGLOG("intersection between %d and %d at [%.3f, %.3f], t1x %.3f t1y %.3f t2x %.3f t2y %.3f\n", i, j, px, py, t1x, t1y, t2x, t2y);

            if (!isinf(px) && !isinf(py) && !isinf(t1x) && !isinf(t2x) && !isinf(t1y) && !isinf(t2y)
                && px >= teststartx && px <= testendx && py >= teststarty && py <= testendy
                && t1future && t2future)
            {
                // DEBUGLOG("in range\n");
                ++sum1;
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

