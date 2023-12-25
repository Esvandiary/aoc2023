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

typedef struct vec2i64
{
    int64_t x;
    int64_t y;
} vec2i64;

typedef struct vec2f
{
    double x;
    double y;
} vec2f;

static inline FORCEINLINE vec2f intersection_xy(vec2i64 p1, vec2i64 d1, vec2i64 p2, vec2i64 d2)
{
    __int128_t x1 = p1.x; __int128_t x2 = p1.x + d1.x;
    __int128_t x3 = p2.x; __int128_t x4 = p2.x + d2.x;
    __int128_t y1 = p1.y; __int128_t y2 = p1.y + d1.y;
    __int128_t y3 = p2.y; __int128_t y4 = p2.y + d2.y;
    
    const double pxnum = (double)((x1*y2 - y1*x2)*(x3-x4) - (x1-x2)*(x3*y4 - y3*x4));
    const double pynum = (double)((x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4 - y3*x4));
    const double pxdenom = (double)((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
    const double pydenom = (double)((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

    double px = pxnum / pxdenom;
    double py = pynum / pydenom;

    return (vec2f) { .x = px, .y = py };
}

static inline FORCEINLINE vec2i64 intersection_xy_i(vec2i64 p1, vec2i64 d1, vec2i64 p2, vec2i64 d2)
{
    __int128_t x1 = p1.x; __int128_t x2 = p1.x + d1.x;
    __int128_t x3 = p2.x; __int128_t x4 = p2.x + d2.x;
    __int128_t y1 = p1.y; __int128_t y2 = p1.y + d1.y;
    __int128_t y3 = p2.y; __int128_t y4 = p2.y + d2.y;
    
    const __int128_t pxnum = ((x1*y2 - y1*x2)*(x3-x4) - (x1-x2)*(x3*y4 - y3*x4));
    const __int128_t pynum = ((x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4 - y3*x4));
    const __int128_t pxdenom = ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
    const __int128_t pydenom = ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

    __int128_t px = (pxdenom != 0) ? pxnum / pxdenom : INT64_MAX;
    __int128_t py = (pxdenom != 0) ? pynum / pydenom : INT64_MAX;

    return (vec2i64) { .x = (int64_t)px, .y = (int64_t)py };
}

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

    for (int i = 0; i < stonesCount; ++i)
        DEBUGLOG("stone %d: [%ld, %ld, %ld] @ [%ld, %ld, %ld]\n", i, stones[i].x, stones[i].y, stones[i].z, stones[i].dx, stones[i].dy, stones[i].dz);

    DSTOPWATCH_END(init);

    int64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(part1);

    for (int i = 0; i < stonesCount - 1; ++i)
    {
        for (int j = i + 1; j < stonesCount; ++j)
        {
            if (stones[i].dx == stones[j].dx && stones[i].dy == stones[j].dy)
                continue;

            vec2f p = intersection_xy(
                (vec2i64) { .x = stones[i].x, .y = stones[i].y },
                (vec2i64) { .x = stones[i].dx, .y = stones[i].dy },
                (vec2i64) { .x = stones[j].x, .y = stones[j].y },
                (vec2i64) { .x = stones[j].dx, .y = stones[j].dy });

            double t1x = (p.x - stones[i].x) / stones[i].dx;
            double t1y = (p.y - stones[i].y) / stones[i].dy;
            double t2x = (p.x - stones[j].x) / stones[j].dx;
            double t2y = (p.y - stones[j].y) / stones[j].dy;
            bool t1future = (t1x >= 0 && t1y >= 0);
            bool t2future = (t2x >= 0 && t2y >= 0);

            // DEBUGLOG("intersection between %d and %d at [%.3f, %.3f], t1x %.3f t1y %.3f t2x %.3f t2y %.3f\n", i, j, px, py, t1x, t1y, t2x, t2y);

            if (!isinf(p.x) && !isinf(p.y) && !isinf(t1x) && !isinf(t2x) && !isinf(t1y) && !isinf(t2y)
                && p.x >= teststartx && p.x <= testendx && p.y >= teststarty && p.y <= testendy
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

    bool xvalid[1000] = { [0 ... 999] = true };
    bool yvalid[1000] = { [0 ... 999] = true };
    bool zvalid[1000] = { [0 ... 999] = true };

    for (int i = 0; i < stonesCount; ++i)
    {
        for (int j = 0; j < stonesCount; ++j)
        {
            if (i == j) continue;

            if (stones[i].x > stones[j].x && stones[i].dx > stones[j].dx)
            {
                for (int cdx = MAX(-500, stones[j].dx + 1); cdx < MIN(500, stones[i].dx); ++cdx)
                    xvalid[cdx + 500] = false;
            }
            if (stones[i].y > stones[j].y && stones[i].dy > stones[j].dy)
            {
                for (int cdy = MAX(-500, stones[j].dy + 1); cdy < MIN(500, stones[i].dy); ++cdy)
                    yvalid[cdy + 500] = false;
            }
            if (stones[i].z > stones[j].z && stones[i].dz > stones[j].dz)
            {
                for (int cdz = MAX(-500, stones[j].dz + 1); cdz < MIN(500, stones[i].dz); ++cdz)
                    zvalid[cdz + 500] = false;
            }
        }
    }

    int xvc = 0, yvc = 0, zvc = 0;
    for (int i = 0; i < 1000; ++i)
    {
        xvc += xvalid[i];
        yvc += yvalid[i];
        zvc += zvalid[i];
    }
    DEBUGLOG("valid x = %d, y = %d, z = %d\n", xvc, yvc, zvc);

    for (int cvx = -500; cvx < 500; ++cvx)
    {
        if (!xvalid[cvx + 500])
           continue;

        for (int cvy = -500; cvy < 500; ++cvy)
        {
            if (!yvalid[cvy + 500])
               continue;

            vec2i64 curp = {0};

            int64_t adjdx1 = stones[0].dx - cvx;
            int64_t adjdy1 = stones[0].dy - cvy;

            for (int j = 1; j < stonesCount; ++j)
            {
                int64_t adjdx2 = stones[j].dx - cvx;
                int64_t adjdy2 = stones[j].dy - cvy;
                // DEBUGLOG("[%ld,%ld] testing %d vs %d, adjusted d1 (%ld,%ld) d2 (%ld,%ld)\n", cvx, cvy, 0, j, adjdx1, adjdy1, adjdx2, adjdy2);

                vec2i64 p = intersection_xy_i(
                    (vec2i64) { .x = stones[0].x, .y = stones[0].y },
                    (vec2i64) { .x = adjdx1, .y = adjdy1 },
                    (vec2i64) { .x = stones[j].x, .y = stones[j].y },
                    (vec2i64) { .x = adjdx2, .y = adjdy2 });

                double t1 = (adjdx1 != 0) ? (double)(p.x - stones[0].x) / adjdx1 : (double)(p.y - stones[0].y) / adjdy1;
                double t2 = (adjdx2 != 0) ? (double)(p.x - stones[j].x) / adjdx2 : (double)(p.y - stones[j].y) / adjdy2;
                bool t1future = !isinf(t1) && t1 >= 0;
                bool t2future = !isinf(t2) && t2 >= 0;

                // DEBUGLOG("t1 = %.3f, t2 = %.3f, t1f = %d, t2f = %d\n", t1, t2, t1future, t2future);

                if (p.x != INT64_MAX && p.y != INT64_MAX && t1future && t2future)
                {
                    // DEBUGLOG("xy intersection\n");
                    if (curp.x == 0 && curp.y == 0)
                        curp = p;
                    
                    if (curp.x == p.x && curp.y == p.y)
                        continue;
                    // if (fabs(curp.x - p.x) < FLT_EPSILON && fabs(curp.y - p.y) < FLT_EPSILON)
                    //     continue;
                }

                // DEBUGLOG("p (%ld,%ld) didn't match existing (%ld,%ld)\n", p.x, p.y, curp.x, curp.y);
                goto nextpos;
            }

            DEBUGLOG("match?? [%d,%d]\n", cvx, cvy);

            for (int cvz = -500; cvz < 500; ++cvz)
            {
                if (!zvalid[cvz + 500])
                    continue;

                vec2i64 zcurp = {0};

                int64_t adjdz1 = stones[0].dz - cvz;
                for (int j = 1; j < stonesCount; ++j)
                {
                    int64_t adjdx2 = stones[j].dx - cvx;
                    int64_t adjdz2 = stones[j].dz - cvz;

                    vec2i64 zp = intersection_xy_i(
                        (vec2i64) { .x = stones[0].x, .y = stones[0].z },
                        (vec2i64) { .x = adjdx1, .y = adjdz1 },
                        (vec2i64) { .x = stones[j].x, .y = stones[j].z },
                        (vec2i64) { .x = adjdx2, .y = adjdz2 });

                    double zt1 = (adjdx1 != 0) ? (double)(zp.x - stones[0].x) / adjdx1 : (double)(zp.y - stones[0].z) / adjdz1;
                    double zt2 = (adjdx2 != 0) ? (double)(zp.x - stones[j].x) / adjdx2 : (double)(zp.y - stones[j].z) / adjdz2;
                    bool zt1future = !isinf(zt1) && zt1 >= 0;
                    bool zt2future = !isinf(zt2) && zt2 >= 0;

                    // DEBUGLOG("t1 = %.3f, t2 = %.3f, t1f = %d, t2f = %d\n", t1, t2, t1future, t2future);

                    if (zp.x != INT64_MAX && zp.y != INT64_MAX && zt1future && zt2future)
                    {
                        // DEBUGLOG("xy intersection\n");
                        if (zcurp.x == 0 && zcurp.y == 0)
                            zcurp = zp;
                        
                        if (zcurp.x == zp.x && zcurp.y == zp.y)
                            continue;
                        // if (fabs(curp.x - p.x) < FLT_EPSILON && fabs(curp.y - p.y) < FLT_EPSILON)
                        //     continue;
                    }

                    // DEBUGLOG("p (%ld,%ld) didn't match existing (%ld,%ld)\n", p.x, p.y, curp.x, curp.y);
                    goto znextpos;
                }

                DEBUGLOG("match!! [%d,%d,%d]\n", cvx, cvy, cvz);
            
                DEBUGLOG("rock origin [%lld,%lld,%lld]\n", curp.x, curp.y, zcurp.y);

                sum2 = curp.x + curp.y + zcurp.y;

            znextpos:
            }

        nextpos:
        }
    }

    print_int64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

