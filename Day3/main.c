#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"
#include "../common/vuctor.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y, x) (((y) * (lineLength + 1)) + (x))

static const uint8_t issymbol[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, // 00
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 10
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, // 20
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, // 30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 50
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 70
};

typedef struct number
{
    int16_t number;
    int16_t length;
} number;

static number numbers[65536] = {0};

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int textLength = (int)(file.size);

    DSTOPWATCH_START(init);
    // get line length
    int lineLength = 0;
    for (int i = 0; i < textLength; ++i)
    {
        if (file.data[i] == '\n')
        {
            lineLength = i;
            break;
        }
    }

    int gears[8192];
    size_t gearCount = 0;

    chartype* data = file.data + 1;
    chartype* end = file.data + textLength;

    DSTOPWATCH_END(init);
    DSTOPWATCH_PRINT(init);

    //
    // Part 1
    //

    DSTOPWATCH_START(part1);
    int sum1 = 0;

    static void* charjumps[] = {
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 00
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 10
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&gear,  &&null,  &&null,  &&null,  &&null,  &&null,  // 20
        &&digit, &&digit, &&digit, &&digit, &&digit, &&digit, &&digit, &&digit, &&digit, &&digit, &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 30
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 40
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 50
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 60
        &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  &&null,  // 70
    };

    while (data < end)
    {
        goto *charjumps[*data];

    gear:
        gears[gearCount++] = data - file.data;
    null:
        ++data;
        continue;

    digit:
        chartype* numend = data + 1;
        int num = *data & 0xF;
        while (isdigit(*numend))
        {
            num *= 10;
            num += *numend & 0xF;
            ++numend;
        }

        const size_t idx = data - file.data;
        const size_t len = numend - data;
        for (int i = 0; i < len; ++i)
            numbers[idx+i] = (number) { .number = num, .length = len - i };

        if (issymbol[data[-1]] || issymbol[numend[0]])
        {
            sum1 += num;
            goto next;
        }
        if (idx >= lineLength + 1)
        {
            chartype* ccur = data - MIN(idx, lineLength + 2);
            chartype* cend = data + len - lineLength;
            while (ccur < cend)
            {
                if (issymbol[*ccur])
                {
                    sum1 += num;
                    goto next;
                }
                ++ccur;
            }
        }
        if (numend + lineLength + 1 < end)
        {
            chartype* ccur = data + lineLength;
            chartype* cend = data + len + lineLength + 2;
            while (ccur < cend)
            {
                if (issymbol[*ccur])
                {
                    sum1 += num;
                    goto next;
                }
                ++ccur;
            }
        }
    next:
        data = numend;
    }
    DSTOPWATCH_END(part1);
    DSTOPWATCH_PRINT(part1);

    print_uint64(sum1);

    //
    // Part 2
    //

    DSTOPWATCH_START(part2);
    int sum2 = 0;

    for (int gi = 0; gi < gearCount; ++gi)
    {
        const int gidx = gears[gi];
        const int y = gidx / (lineLength + 1);
        const int x = gidx % (lineLength + 1);

        int numCount = 0;
        int mul = 1;

        for (int cy = y - 1; cy <= y + 1; ++cy)
        {
            for (int cx = x - 1; cx <= x + 1; ++cx)
            {
                size_t idx = dataindex(cy, cx);
                if (isdigit(file.data[idx]))
                {
                    mul *= numbers[idx].number;
                    ++numCount;
                    cx += numbers[idx].length;
                }
            }
        }

        if (numCount == 2)
            sum2 += mul;
    }
    DSTOPWATCH_END(part2);
    DSTOPWATCH_PRINT(part2);

    print_uint64(sum2);

    return 0;
}
