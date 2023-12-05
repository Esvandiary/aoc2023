#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mmap.h"
#include "../common/vuctor.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    vuctor cards = VUCTOR_INIT;
    VUCTOR_RESERVE(cards, uint8_t, 1024);

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

        VUCTOR_ADD(cards, uint8_t, matchCount);
        sum1 += (1U << matchCount) >> 1;
        ++idx;
        ++lineNum;
    }

    //
    // Part 1
    //

    printf("%d\n", sum1);

    //
    // Part 2
    //

    uint64_t* counts = (uint64_t*)calloc(lineNum, sizeof(uint64_t));
    for (int i = 0; i < lineNum; ++i)
    {
        counts[i] += 1;
        for (int ni = 1; i + ni < lineNum && ni <= VUCTOR_GET(cards, uint8_t, i); ++ni)
            counts[i + ni] += counts[i];
    }

    uint64_t sum2 = 0;
    for (int i = 0; i < lineNum; ++i)
        sum2 += counts[i];

    printf("%zu\n", sum2);

    return 0;
}

