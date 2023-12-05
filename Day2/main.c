#include <stdint.h>
#include "../common/mmap.h"
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct GrabResult
{
    uint16_t RedCount;
    uint16_t GreenCount;
    uint16_t BlueCount;
} GrabResult;

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    vuctor games = VUCTOR_INIT;
    VUCTOR_RESERVE(games, GrabResult, 512);

    int lineIdx = 0;
    while (lineIdx < fileSize)
    {
        const int lineStart = lineIdx;
        for (; lineIdx < fileSize; ++lineIdx)
        {
            if (file.data[lineIdx] == '\n')
                break;
        }
        if (UNLIKELY(lineIdx - lineStart < 2))
        {
            ++lineIdx;
            continue;
        }
        const view line = { file.data + lineStart, lineIdx - lineStart };

        const size_t lineLength = line.size;
        size_t idx = 0;
        while (idx < lineLength && line.data[idx] != ':')
            ++idx;
        idx += 2;

        GrabResult current = { .RedCount = 0, .GreenCount = 0, .BlueCount = 0 };
        while (idx < lineLength)
        {
            // number
            uint16_t num = 0;
            while (isdigit(line.data[idx]))
            {
                num *= 10;
                num += (line.data[idx++] - '0');
            }
            // space
            ++idx;
            // colour
            switch (line.data[idx])
            {
                case 'r':
                    current.RedCount = MAX(current.RedCount, num);
                    idx += 5; // 'red[,;] '
                    break;
                case 'g':
                    current.GreenCount = MAX(current.GreenCount, num);
                    idx += 7; // 'green[,;] '
                    break;
                case 'b':
                    current.BlueCount = MAX(current.BlueCount, num);
                    idx += 6; // 'blue[,;] '
                    break;
                default:
                    printf("unexpected input from line\n");
                    return 1;
            }
        }
        VUCTOR_ADD(games, GrabResult, current);

        ++lineIdx;
    }

    //
    // Part 1
    //

    int sum1 = 0;
    for (int i = 0; i < games.size; ++i)
    {
        const int mask = ((VUCTOR_GET(games, GrabResult, i).RedCount - 13) & (VUCTOR_GET(games, GrabResult, i).GreenCount - 14) & (VUCTOR_GET(games, GrabResult, i).BlueCount - 15)) >> 31;
        sum1 += (i + 1) & mask;
    }

    printf("%d\n", sum1);

    //
    // Part 2
    //

    int sum2 = 0;
    for (int i = 0; i < games.size; ++i)
        sum2 += VUCTOR_GET(games, GrabResult, i).RedCount * VUCTOR_GET(games, GrabResult, i).GreenCount * VUCTOR_GET(games, GrabResult, i).BlueCount;

    printf("%d\n", sum2);

    return 0;
}
