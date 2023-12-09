#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

static const int8_t signs[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 00
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 10
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1, 1,  // 20
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 50
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 70
};

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int32_t dlist[32 * 128];

    //
    // Part 1 + 2
    //

    size_t idx = 0;
    int64_t sum1 = 0, sum2 = 0;

    while (idx < file.size)
    {
        size_t zcount = 0;
        while (idx < file.size)
        {
            int32_t num = 0;
            int32_t sign = signs[file.data[idx]];
            idx += (-(sign - 1) >> 1);
            while (isdigit(file.data[idx]))
            {
                num *= 10;
                num += file.data[idx++] & 0xF;
            }
            num *= sign;

            dlist[zcount++] = num;
            if (file.data[idx++] == '\n')
                break;
        }

        int32_t firsts[128];
        int32_t lasts[128];

        for (int i = 0; i < 32; ++i)
        {
            int32_t* dcur = dlist + i*128;
            int32_t* dnext = dlist + (i+1)*128;
            int32_t last = firsts[i] = dcur[0];
            bool constant = true;
            for (int n = 1; n < (zcount - i); ++n)
            {
                const int32_t cn = dcur[n];
                constant &= (cn == last);
                int32_t cndiff = cn - last;
                dnext[n-1] = cndiff;
                last = cn;
            }
            lasts[i] = last;

            if (constant)
            {
                int32_t fnum = firsts[i], lnum = lasts[i];
                for (i = i - 1; i >= 0; --i)
                {
                    fnum = firsts[i] - fnum;
                    lnum += lasts[i];
                }

                DEBUGLOG("lnum = %d, fnum = %d\n", lnum, fnum);
                sum1 += lnum;
                sum2 += fnum;
                break;
            }
        }
    }

    print_int64(sum1);
    print_int64(sum2);

    return 0;
}

