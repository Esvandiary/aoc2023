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
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y,x) ((y) * lineLength + (x))
#define dataY(idx) ((idx) / lineLength)
#define dataX(idx) ((idx) % lineLength)

int getcount(chartype* const cstart, chartype* cs, const chartype* const end, const int* const nums, const size_t numsCount)
{
    while (cs != end && *cs != '?')
        ++cs;
    if (cs == end)
    {
        DEBUGLOG("cs = end, checking\n");
        // check if this makes sense
        cs = cstart;
        int nextNum = 0;
        int cur = 0;
        while (cs < end)
        {
            if (*cs++ == '#')
            {
                ++cur;
            }
            else if (cur > 0)
            {
                DEBUGLOG("nextNum = %d, numsCount = %zu\n", nextNum, numsCount);
                if (nextNum >= numsCount)
                    return 0;
                DEBUGLOG("in range, next num = %d, cur = %d\n", nums[nextNum], cur);
                if (nums[nextNum++] != cur)
                    return 0;
                cur = 0;
            }
        }
        DEBUGLOG("END %d vs %zu, %s\n", nextNum, numsCount, nextNum == numsCount ? "***MATCH***" : "no match");
        return (nextNum == numsCount) ? 1 : 0;
    }
    else
    {
        int result = 0;
        *cs = '.';
        result += getcount(cstart, cs + 1, end, nums, numsCount);
        *cs = '#';
        result += getcount(cstart, cs + 1, end, nums, numsCount);
        *cs = '?';
        ++cs;
        return result;
    }
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    int64_t sum1 = 0, sum2 = 0;

    while (data < end)
    {
        int nums[64];
        size_t numCount = 0;

        chartype* cstart = data;
        while (*data++ != ' ');

        const chartype* cend = data;
        do
        {
            int num = 0;
            while (isdigit(*data))
            {
                num *= 10;
                num += *data++ & 0xF;
            }
            nums[numCount++] = num;
        } while (*data++ != '\n');

        sum1 += getcount(cstart, cstart, cend, nums, numCount);
    }

    DSTOPWATCH_END(logic);

    print_int64(sum1);
    print_int64(sum2);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

