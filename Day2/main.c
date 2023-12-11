#include <stdint.h>
#include "../common/mmap.h"
#include "../common/print.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct ColourResult
{
    int32_t Skip;
    int32_t Count;
} ColourResult;

#define CR_R (0x12)
#define CR_G (0x07)
#define CR_B (0x02)

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    int sum1 = 0, sum2 = 0;

    ColourResult results[32];
    results[CR_R].Skip = 3;
    results[CR_G].Skip = 5;
    results[CR_B].Skip = 4;

    const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;
    int lineNum = 1;
    while (data < end)
    {
        results[CR_R].Count = 0;
        results[CR_G].Count = 0;
        results[CR_B].Count = 0;

        while (*data != ':')
            ++data;

        while (*data != '\n')
        {
            data += 2;
            // number
            uint32_t num = 0;
            while (isdigit(*data))
            {
                num *= 10;
                num += (*data++ & 0xF);
            }
            ++data; // ' '
            // colour
            const uint8_t ridx = *data & 0x1F;
            results[ridx].Count = MAX(results[ridx].Count, num);
            data += results[ridx].Skip;
        }

        const int mask = ((results[CR_R].Count - 13) & (results[CR_G].Count - 14) & (results[CR_B].Count - 15)) >> 31;
        sum1 += lineNum & mask;

        sum2 += results[CR_R].Count * results[CR_G].Count * results[CR_B].Count;

        ++data; // '\n'
        ++lineNum;
    }

    //
    // Part 1
    //

    print_uint64(sum1);

    //
    // Part 2
    //

    print_uint64(sum2);

    return 0;
}
