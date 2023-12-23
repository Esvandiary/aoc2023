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
    chartype* const data;
    const size_t size;
    const int lineLength;
    const int lineCount;
    const uint32_t startIndex;
    const uint32_t finishIndex;
} fdata;

typedef struct cresult
{
    int nextcount[5];
} cresult;

#define PD_NONE 0
#define PD_UP 1
#define PD_LEFT 2
#define PD_RIGHT 3
#define PD_DOWN 4

static const char* const pdnames[] = {"none", "up", "left", "right", "down"};

static int findmaxlen1(fdata* d, uint32_t idx, uint32_t prevdir, int count)
{
    if (idx == d->finishIndex)
        return count;

    int maxlen = -1;
    const chartype orig = d->data[idx];
    d->data[idx] = '#';
    if (idx >= d->lineLength && prevdir != PD_DOWN && (d->data[idx - d->lineLength] == '.' || d->data[idx - d->lineLength] == '^'))
    {
        int ulen = findmaxlen1(d, idx - d->lineLength, PD_UP, count + 1);
        maxlen = MAX(maxlen, ulen);
    }
    if (idx > 0 && prevdir != PD_RIGHT && (d->data[idx - 1] == '.' || d->data[idx - 1] == '<'))
    {
        int llen = findmaxlen1(d, idx - 1, PD_LEFT, count + 1);
        maxlen = MAX(maxlen, llen);
    }
    if (idx + 1 < d->size && prevdir != PD_LEFT && (d->data[idx + 1] == '.' || d->data[idx + 1] == '>'))
    {
        int rlen = findmaxlen1(d, idx + 1, PD_RIGHT, count + 1);
        maxlen = MAX(maxlen, rlen);
    }
    if (idx + d->lineLength < d->size && prevdir != PD_UP && (d->data[idx + d->lineLength] == '.' || d->data[idx + d->lineLength] == 'v'))
    {
        int dlen = findmaxlen1(d, idx + d->lineLength, PD_DOWN, count + 1);
        maxlen = MAX(maxlen, dlen);
    }
    d->data[idx] = orig;
    return maxlen;
}

static const bool isvalid2[256] = { ['.'] = true, ['^'] = true, ['v'] = true, ['<'] = true, ['>'] = true };

static int findmaxlen2(fdata* d, uint32_t idx, uint32_t prevdir, int count)
{
    if (idx == d->finishIndex)
    {
        return count;
    }

    int maxlen = -1;
    const chartype orig = d->data[idx];
    d->data[idx] = 'O';
    if (idx >= d->lineLength && prevdir != PD_DOWN && isvalid2[d->data[idx - d->lineLength]])
    {
        int ulen = findmaxlen2(d, idx - d->lineLength, PD_UP, count + 1);
        maxlen = MAX(maxlen, ulen);
    }
    if (idx > 0 && prevdir != PD_RIGHT && isvalid2[d->data[idx - 1]])
    {
        int llen = findmaxlen2(d, idx - 1, PD_LEFT, count + 1);
        maxlen = MAX(maxlen, llen);
    }
    if (idx + 1 < d->size && prevdir != PD_LEFT && isvalid2[d->data[idx + 1]])
    {
        int rlen = findmaxlen2(d, idx + 1, PD_RIGHT, count + 1);
        maxlen = MAX(maxlen, rlen);
    }
    if (idx + d->lineLength < d->size && prevdir != PD_UP && isvalid2[d->data[idx + d->lineLength]])
    {
        int dlen = findmaxlen2(d, idx + d->lineLength, PD_DOWN, count + 1);
        maxlen = MAX(maxlen, dlen);
    }

    d->data[idx] = orig;
    return maxlen;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    while (*data != '.')
        ++data;
    uint32_t sidx = (data - file.data);
    while (*data++ != '\n');
    const int lineLength = data - file.data;
    const int lineCount = (fileSize + lineLength / 2) / lineLength;

    DEBUGLOG("lineLength = %d, lineCount = %d\n", lineLength, lineCount);

    data = file.data + (lineCount - 1) * lineLength;
    uint32_t fidx;
    while (data < end)
    {
        if (*data == '.')
        {
            fidx = (data - file.data);
            break;
        }
        ++data;
    }

    fdata d = {
        .data = file.data,
        .size = fileSize,
        .lineLength = lineLength,
        .lineCount = lineCount,
        .startIndex = sidx,
        .finishIndex = fidx
    };

    DEBUGLOG("sidx = (%d,%d), fidx = (%d,%d)\n", dataY(d, sidx), dataX(d, sidx), dataY(d, fidx), dataX(d, fidx));

    int64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(part1);

    sum1 = findmaxlen1(&d, d.startIndex, PD_NONE, 0);

    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    sum2 = findmaxlen2(&d, d.startIndex, PD_NONE, 0);

    DSTOPWATCH_END(part2);

    print_int64(sum1);
    print_int64(sum2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

