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

    const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;
    int lineNum = 1;
    while (data < end)
    {
        int redCount = 0, greenCount = 0, blueCount = 0;

        while (data < end && *data != ':')
            ++data;

        while (data < end && *data != '\n')
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
            switch (*data)
            {
                case 'r':
                    redCount = MAX(redCount, num);
                    data += 3;
                    break;
                case 'g':
                    greenCount = MAX(greenCount, num);
                    data += 5;
                    break;
                case 'b':
                    blueCount = MAX(blueCount, num);
                    data += 4;
                    break;
                default:
                    __builtin_unreachable();
                    break;
            }
        }

        const int mask = ((redCount - 13) & (greenCount - 14) & (blueCount - 15)) >> 31;
        sum1 += lineNum & mask;

        sum2 += redCount * greenCount * blueCount;

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
