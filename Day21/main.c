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

int64_t run(const fdata* d, int sidx, int depthoffset, int maxdepth)
{
    bool on[192*192] = {0};
    bool been[2][192*192] = {0};
    bool waiting[2][192*192*2] = {0};

    waiting[0][sidx] = true;

    for (int depth = 0; depth <= maxdepth; ++depth)
    {
        DEBUGLOG("starting depth %d\n", depth);
        bool anywaiting = false;
        for (int idx = 0; idx < d->size; ++idx)
        {
            if (waiting[(depth + depthoffset) & 1][idx])
            {
                DEBUGLOG("  [%d]\n", idx);
                been[depth & 1][idx] = true;

                if ((depth + depthoffset) % 2 == maxdepth % 2)
                {
                    on[idx] = true;
                }
                if (depth + 1 <= maxdepth)
                {
                    int uidx = idx - d->lineLength;
                    int didx = idx + d->lineLength;
                    int lidx = idx - 1;
                    int ridx = idx + 1;
                    DEBUGLOG("    uidx %d depth %d been %d char %c\n", uidx, depth, been[depth & 1][uidx], d->data[uidx]);
                    DEBUGLOG("    didx %d depth %d been %d char %c\n", didx, depth, been[depth & 1][didx], d->data[didx]);
                    DEBUGLOG("    lidx %d depth %d been %d char %c\n", lidx, depth, been[depth & 1][lidx], d->data[lidx]);
                    DEBUGLOG("    ridx %d depth %d been %d char %c\n", ridx, depth, been[depth & 1][ridx], d->data[ridx]);
                    if (uidx >= 0 && !been[(depth + depthoffset + 1) & 1][uidx] && (d->data[uidx] == '.' || d->data[uidx] == 'S'))
                    {
                        waiting[(depth + 1) & 1][uidx] = true;
                        anywaiting = true;
                    }
                    if (didx < d->size && !been[(depth + depthoffset + 1) & 1][didx] && (d->data[didx] == '.' || d->data[didx] == 'S'))
                    {
                        waiting[(depth + 1) & 1][didx] = true;
                        anywaiting = true;
                    }
                    if (lidx >= 0 && !been[(depth + depthoffset + 1) & 1][lidx] && (d->data[lidx] == '.' || d->data[lidx] == 'S'))
                    {
                        waiting[(depth + 1) & 1][lidx] = true;
                        anywaiting = true;
                    }
                    if (ridx < d->size && !been[(depth + depthoffset + 1) & 1][ridx] && (d->data[ridx] == '.' || d->data[ridx] == 'S'))
                    {
                        waiting[(depth + 1) & 1][ridx] = true;
                        anywaiting = true;
                    }
                }
            }
        }
        if (!anywaiting)
            break;
        memset(waiting[depth & 1], 0, sizeof(waiting[depth & 1]));
    }

    int64_t result = 0;
    for (int i = 0; i < d->size; ++i)
        result += on[i];
    return result;
}

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

    uint64_t sum1 = 0, sum2 = 0;

    sum1 = run(&d, sidx, 0, 64);

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);



    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

