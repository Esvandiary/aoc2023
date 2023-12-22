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
#define dataindex(d, y, x) ((y)*(d).lineLength + (x))
#define dataY(d, idx) ((idx) / (d).lineLength)
#define dataX(d, idx) ((idx) % (d).lineLength)

typedef struct fdata
{
    const chartype* const data;
    const size_t size;
    const int lineLength;
    const int lineCount;
} fdata;

#define ONSIZE 192

static int64_t iter = 0;

typedef struct qentry
{
    uint16_t idx;
    uint16_t depth;
} qentry;

#define GETWAITING(depth, idx) waiting[(depth) & 1][idx]
#define ADDWAITING(depth, idx) waiting[(depth) & 1][idx] = true
int64_t rungrid(const fdata* d, int sidx, int maxdepth, int depthoffset)
{
    _Alignas(64) bool on[32768] = {0};
    _Alignas(64) bool been[2][32768] = {0};
    _Alignas(64) bool waiting[2][32768] = {0};

    ADDWAITING(0, sidx);

    for (int depth = 0; depth <= maxdepth; ++depth)
    {
        // DEBUGLOG("starting depth %d\n", depth);
        bool anywaiting = false;
        for (int idx = 0; idx < d->size; ++idx)
        {
            if (GETWAITING(depth, idx))
            {
                been[depth & 1][idx] = true;
                on[idx] |= ~((depth ^ (maxdepth + depthoffset))) & 1;

                if (depth + 1 <= maxdepth)
                {
                    const int uidx = idx - d->lineLength;
                    const int didx = idx + d->lineLength;
                    const int lidx = idx - 1;
                    const int ridx = idx + 1;
                    if (uidx >= 0 && !been[(depth + 1) & 1][uidx] && (d->data[uidx] == '.' || d->data[uidx] == 'S'))
                    {
                        ADDWAITING(depth + 1, uidx);
                        anywaiting = true;
                    }
                    if (didx < d->size && !been[(depth + 1) & 1][didx] && (d->data[didx] == '.' || d->data[didx] == 'S'))
                    {
                        ADDWAITING(depth + 1, didx);
                        anywaiting = true;
                    }
                    if (lidx >= 0 && !been[(depth + 1) & 1][lidx] && (d->data[lidx] == '.' || d->data[lidx] == 'S'))
                    {
                        ADDWAITING(depth + 1, lidx);
                        anywaiting = true;
                    }
                    if (ridx < d->size && !been[(depth + 1) & 1][ridx] && (d->data[ridx] == '.' || d->data[ridx] == 'S'))
                    {
                        ADDWAITING(depth + 1, ridx);
                        anywaiting = true;
                    }
                }
            }
        }
        if (!anywaiting)
        {
            // DEBUGLOG("no more waiting at depth %d\n", depth);
            break;
        }
        memset(waiting[depth & 1], 0, sizeof(waiting[depth & 1]));
    }

    int64_t result = 0;
    for (int i = 0; i < d->size; ++i)
        result += on[i];
    return result;
}

#define S_TOPLEFT  0
#define S_TOP      1
#define S_TOPRIGHT 2
#define S_LEFT     3
#define S_MIDDLE   4
#define S_RIGHT    5
#define S_BTMLEFT  6
#define S_BOTTOM   7
#define S_BTMRIGHT 8

int main(int argc, char** argv)
{
    DSTOPWATCH_START(part1);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    while (*data++ != '\n');
    const int lineLength = data - file.data;
    const int lineCount = (fileSize + lineLength / 2) / lineLength;
    while (*data != 'S')
        ++data;
    int sidx = data - file.data;

    fdata d = {
        .data = file.data,
        .size = fileSize,
        .lineLength = lineLength,
        .lineCount = lineCount
    };

    const uint64_t gridwidth = d.lineLength - 1;
    uint64_t sum1 = 0, sum2 = 0;

    sum1 = rungrid(&d, sidx, 64, 0);

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    int runamt = d.lineLength * 4;
    int64_t fullsums[2][9];
    for (int i = 0; i < 2; ++i)
    {
        fullsums[i][S_TOPLEFT ] = rungrid(&d, dataindex(d, 0, 0), runamt, i);
        fullsums[i][S_TOP     ] = rungrid(&d, dataindex(d, 0, dataX(d, sidx)), runamt, i);
        fullsums[i][S_TOPRIGHT] = rungrid(&d, dataindex(d, 0, gridwidth - 1), runamt, i);
        fullsums[i][S_LEFT    ] = rungrid(&d, dataindex(d, dataY(d, sidx), 0), runamt, i);
        fullsums[i][S_MIDDLE  ] = rungrid(&d, sidx, runamt, i);
        fullsums[i][S_RIGHT   ] = rungrid(&d, dataindex(d, dataY(d, sidx), gridwidth - 1), runamt, i);
        fullsums[i][S_BTMLEFT ] = rungrid(&d, dataindex(d, d.lineCount - 1, 0), runamt, i);
        fullsums[i][S_BOTTOM  ] = rungrid(&d, dataindex(d, d.lineCount - 1, dataX(d, sidx)), runamt, i);
        fullsums[i][S_BTMRIGHT] = rungrid(&d, dataindex(d, d.lineCount - 1, gridwidth - 1), runamt, i);
    }

    const int64_t p2steps = 26501365;
    const int64_t p2blocks = p2steps / gridwidth;
    const int64_t p2cblocks = p2blocks - 2;
    DEBUGLOG("p2steps: %ld, p2blocks: %ld\n", p2steps, p2blocks);

    // block: (n^2 + n) / 2
    // zeros: (n/2)^2
    const int64_t fullcardinals[2] = {p2blocks / 2 - 1, p2blocks / 2};
    int64_t fullcorners[2];
    fullcorners[0] = (p2cblocks / 2) * (p2cblocks / 2);
    fullcorners[1] = (p2cblocks * p2cblocks + p2cblocks) / 2 - fullcorners[0];
    DEBUGLOG("fullcorners = %ld, %ld\n", fullcorners[0], fullcorners[1]);
    int64_t fullcounts[2][9] = {
        { fullcorners[0], fullcardinals[0], fullcorners[0], fullcardinals[0], 1, fullcardinals[0], fullcorners[0], fullcardinals[0], fullcorners[0] },
        { fullcorners[1], fullcardinals[1], fullcorners[1], fullcardinals[1], 0, fullcardinals[1], fullcorners[1], fullcardinals[1], fullcorners[1] },
    };

    const int p2runamtcardinal = gridwidth;
    const int p2runamtcorners0 = gridwidth + (p2steps % gridwidth);
    const int p2runamtcorners1 = (p2steps % gridwidth);
    DEBUGLOG("p2 run amounts: %d / %d / %d\n", p2runamtcardinal, p2runamtcorners0, p2runamtcorners1);
    int64_t partsums[2][9];
    partsums[0][S_TOPLEFT ] = rungrid(&d, dataindex(d, 0, 0), p2runamtcorners0, 0);
    partsums[0][S_TOP     ] = rungrid(&d, dataindex(d, 0, dataX(d, sidx)), p2runamtcardinal, 0);
    partsums[0][S_TOPRIGHT] = rungrid(&d, dataindex(d, 0, gridwidth - 1), p2runamtcorners0, 0);
    partsums[0][S_LEFT    ] = rungrid(&d, dataindex(d, dataY(d, sidx), 0), p2runamtcardinal, 0);
    partsums[0][S_MIDDLE  ] = 0;
    partsums[0][S_RIGHT   ] = rungrid(&d, dataindex(d, dataY(d, sidx), gridwidth - 1), p2runamtcardinal, 0);
    partsums[0][S_BTMLEFT ] = rungrid(&d, dataindex(d, d.lineCount - 1, 0), p2runamtcorners0, 0);
    partsums[0][S_BOTTOM  ] = rungrid(&d, dataindex(d, d.lineCount - 1, dataX(d, sidx)), p2runamtcardinal, 0);
    partsums[0][S_BTMRIGHT] = rungrid(&d, dataindex(d, d.lineCount - 1, gridwidth - 1), p2runamtcorners0, 0);
    partsums[1][S_TOPLEFT ] = rungrid(&d, dataindex(d, 0, 0), p2runamtcorners1, 1);
    partsums[1][S_TOP     ] = 0;
    partsums[1][S_TOPRIGHT] = rungrid(&d, dataindex(d, 0, gridwidth - 1), p2runamtcorners1, 1);
    partsums[1][S_LEFT    ] = 0;
    partsums[1][S_MIDDLE  ] = 0;
    partsums[1][S_RIGHT   ] = 0;
    partsums[1][S_BTMLEFT ] = rungrid(&d, dataindex(d, d.lineCount - 1, 0), p2runamtcorners1, 1);
    partsums[1][S_BOTTOM  ] = 0;
    partsums[1][S_BTMRIGHT] = rungrid(&d, dataindex(d, d.lineCount - 1, gridwidth - 1), p2runamtcorners1, 1);

    const int64_t partcorners[2] = {p2blocks - 1, p2blocks};
    int64_t partcounts[2][9] = {
        partcorners[0], 1, partcorners[0], 1, partcorners[0], 1, partcorners[0], 1, partcorners[0],
        partcorners[1], 0, partcorners[1], 0, partcorners[1], 0, partcorners[1], 0, partcorners[1],
    };

    for (int bt = 0; bt < 2; ++bt)
    {
        for (int st = 0; st < 9; ++st)
        {
            DEBUGLOG("full[%d][%d] = %ld x %ld = %lu\n", bt, st, fullsums[bt][st], fullcounts[bt][st], fullsums[bt][st] * fullcounts[bt][st]);
            sum2 += fullsums[bt][st] * fullcounts[bt][st];
        }
    }
    for (int bt = 0; bt < 2; ++bt)
    {
        for (int st = 0; st < 9; ++st)
        {
            DEBUGLOG("part[%d][%d] = %ld x %ld = %lu\n", bt, st, partsums[bt][st], partcounts[bt][st], partsums[bt][st] * partcounts[bt][st]);
            sum2 += partsums[bt][st] * partcounts[bt][st];
        }
    }

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

