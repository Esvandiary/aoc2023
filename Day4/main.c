#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/vuctor.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

static uint8_t cards[2048];
static uint64_t counts[2048] = {0};

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int sum1 = 0;

    bool winning[100];

    int idx = 0;
    int lineNum = 0;
    while (idx < fileSize)
    {
        memset(winning, 0, sizeof(winning));
        uint8_t matchCount = 0;

        while (idx < fileSize && file.data[idx] != ':')
            ++idx;
        idx += 2; // ': '
        while (idx < fileSize && file.data[idx] != '|')
        {
            int num = 0;
            if (isdigit(file.data[idx]))
                num += (uint8_t)(10 * (file.data[idx] & 0xF));
            ++idx;
            num += (uint8_t)(file.data[idx] & 0xF);
            winning[num] = true;
            idx += 2;
        }
        idx += 1; // '|'
        while (idx < fileSize && file.data[idx] != '\n')
        {
            ++idx;
            int num = 0;
            if (isdigit(file.data[idx]))
                num += (uint8_t)(10 * (file.data[idx] & 0xF));
            ++idx;
            num += (uint8_t)(file.data[idx] & 0xF);
            ++idx;

            if (winning[num])
                ++matchCount;
        }

        cards[lineNum++] = matchCount;
        sum1 += (1U << matchCount) >> 1;
        ++idx;
    }

    //
    // Part 1
    //

    print_uint64(sum1);

    //
    // Part 2
    //

    uint64_t sum2 = 0;

    for (int i = 0; i < lineNum; ++i)
    {
        const int lineMax = MIN(lineNum, i + cards[i] + 1);
        counts[i] += 1;
        for (int ni = i + 1; ni < lineMax; ++ni)
            counts[ni] += counts[i];
        sum2 += counts[i];
    }

    print_uint64(sum2);

    return 0;
}

