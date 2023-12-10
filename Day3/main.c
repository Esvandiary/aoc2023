#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/vuctor.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y, x) (((y) * (lineLength + 1)) + (x))

typedef struct number
{
    int number;
    int length;
} number;

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int textLength = (int)(file.size);

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
    const int lineCount = ((textLength + lineLength / 2) / (lineLength + 1));

    number* numbers = (number*)calloc((uint32_t)textLength, sizeof(number));

    int gears[8192];
    size_t gearCount = 0;

    chartype* data = file.data;
    chartype* end = file.data + textLength;

    //
    // Part 1
    //

    int sum1 = 0;

    while (data < end)
    {
        const char c = *data;
        if (!isdigit(c))
        {
            if (c == '*')
                gears[gearCount++] = data - file.data;
            ++data;
            continue;
        }

        chartype* numend = data + 1;
        int num = *data & 0xF;
        while (numend < end && isdigit(*numend))
        {
            num *= 10;
            num += *numend & 0xF;
            ++numend;
        }

        const size_t idx = data - file.data;
        const size_t len = numend - data;
        numbers[idx] = (number) { .number = num, .length = len };

        if ((idx != 0 && data[-1] != '.' && data[-1] != '\n') || (numend[0] != '.' && numend[0] != '\n'))
        {
            sum1 += num;
            goto next;
        }
        if (idx >= lineLength + 1)
        {
            chartype* ccur = file.data + MAX(0, idx - lineLength - 2);
            chartype* cend = file.data + idx + len - lineLength;
            while (ccur < cend)
            {
                if (*ccur != '.' && *ccur != '\n')
                {
                    sum1 += num;
                    goto next;
                }
                ++ccur;
            }
        }
        if (numend + lineLength + 1 < end)
        {
            chartype* ccur = file.data + idx + lineLength;
            chartype* cend = file.data + idx + len + lineLength + 2;
            while (ccur < cend)
            {
                if (*ccur != '.' && *ccur != '\n')
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

    print_uint64(sum1);

    //
    // Part 2
    //

    int sum2 = 0;

    for (int gi = 0; gi < gearCount; ++gi)
    {
        int gidx = gears[gi];
        int y = gidx / (lineLength + 1);
        int x = gidx % (lineLength + 1);

        int numCount = 0;
        int mul = 1;

        for (int cy = MAX(0, y - 1); cy <= MIN(lineCount - 1, y + 1); ++cy)
        {
            for (int cx = MAX(0, x - 1); cx <= x + 1; ++cx)
            {
                size_t idx = dataindex(cy, cx);
                if (isdigit(file.data[idx]))
                {
                    while (!numbers[idx].length)
                    {
                        --idx;
                        --cx;
                    }
                    mul *= numbers[idx].number;
                    ++numCount;
                    cx += numbers[idx].length;
                }
            }
        }

        if (numCount == 2)
            sum2 += mul;
    }

    print_uint64(sum2);

    return 0;
}
