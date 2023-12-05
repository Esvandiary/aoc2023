#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "../common/mmap.h"
#include "../common/vuctor.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y, x) ((y * (lineLength + 1)) + x)

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
    
    bool* numbers = (bool*)calloc(textLength, sizeof(bool));
    bool* symbols = (bool*)calloc(textLength, sizeof(bool));

    vuctor gears = VUCTOR_INIT;
    VUCTOR_RESERVE(gears, int, lineCount * 5);

    for (int y = 0; y < lineCount; ++y)
    {
        for (int x = 0; x < lineLength; ++x)
        {
            const int idx = (y * lineLength) + x;
            char c = file.data[dataindex(y, x)];
            if (isdigit(c))
            {
                numbers[idx] = true;
            }
            else if (c != '.')
            {
                symbols[idx] = true;
                if (c == '*')
                    VUCTOR_ADD(gears, int, idx);
            }
        }
    }

    //
    // Part 1
    //

    int sum1 = 0;

    for (int y = 0; y < lineCount; ++y)
    {
        for (int x = 0; x < lineLength; ++x)
        {
            int startX = x;
            int startIdx = (y * lineLength + x);
            int idx = startIdx;
            int num = 0;
            while (x < lineLength && numbers[idx])
            {
                num *= 10;
                num += file.data[dataindex(y, x)] & 0xF;
                ++x;
                ++idx;
            }
            if (idx != startIdx)
            {
                if ((startX != 0 && symbols[startIdx - 1]) || (x != lineLength && symbols[idx]))
                    goto hit;
                if (y != 0)
                {
                    int cidx = ((y - 1) * lineLength) + MAX(0, startX - 1);
                    int cidxEnd = ((y - 1) * lineLength) + MIN(lineLength - 1, x);
                    while (cidx <= cidxEnd)
                    {
                        if (symbols[cidx++])
                            goto hit;
                    }
                }
                if (y + 1 != lineCount)
                {
                    int cidx = ((y + 1) * lineLength) + MAX(0, startX - 1);
                    int cidxEnd = ((y + 1) * lineLength) + MIN(lineLength - 1, x);
                    while (cidx <= cidxEnd)
                    {
                        if (symbols[cidx++])
                            goto hit;
                    }
                }
            }
            continue;
        hit:
            sum1 += num;
        }
    }

    printf("%d\n", sum1);

    //
    // Part 2
    //

    int sum2 = 0;

    for (int gi = 0; gi < gears.size; ++gi)
    {
        int gidx = VUCTOR_GET(gears, int, gi);
        int y = gidx / lineLength;
        int x = gidx % lineLength;

        int numCount = 0;
        int mul = 1;

        for (int cy = MAX(0, y - 1); cy <= MIN(lineCount - 1, y + 1); ++cy)
        {
            for (int cx = MAX(0, x - 1); cx <= MIN(lineLength - 1, x + 1); ++cx)
            {
                if (numbers[(cy * lineLength) + cx])
                {
                    // find start of number
                    int startX = cx;
                    while (startX >= 1 && numbers[(cy * lineLength) + startX - 1])
                        --startX;
                    int endX = startX;
                    int num = 0;
                    while (endX < lineLength && numbers[(cy * lineLength) + endX])
                    {
                        num *= 10;
                        num += file.data[dataindex(cy, endX)] & 0xF;
                        ++endX;
                    }

                    mul *= num;
                    ++numCount;
                    cx = endX;
                }
            }
        }

        if (numCount == 2)
            sum2 += mul;
    }

    printf("%d\n", sum2);

    return 0;
}
