#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "../common/mmap.hpp"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

struct GrabResult
{
    uint16_t RedCount = 0;
    uint16_t GreenCount = 0;
    uint16_t BlueCount = 0;
};

int main(int argc, char** argv)
{
    auto file = mmap_file::open_ro("input.txt");

    std::vector<GrabResult> games;
    
    int lineIdx = 0;
    while (lineIdx < file.size())
    {
        const int lineStart = lineIdx;
        for (; lineIdx < file.size(); ++lineIdx)
        {
            if (file.data()[lineIdx] == '\n')
                break;
        }
        if (UNLIKELY(lineIdx - lineStart < 2))
        {
            ++lineIdx;
            continue;
        }
        const view<chartype> line = file.slice(lineStart, lineIdx - lineStart);

        const size_t lineLength = line.size();
        size_t colonIdx = 0;
        while (colonIdx < line.size() && line[colonIdx] != ':')
            ++colonIdx;

        size_t idx = colonIdx + 2;
        GrabResult current;
        while (idx < lineLength)
        {
            // number
            uint16_t num = 0;
            while (isdigit(line[idx]))
            {
                num *= 10;
                num += (line[idx++] - '0');
            }
            // space
            ++idx;
            uint16_t redCount = 0, greenCount = 0, blueCount = 0;
            switch (line[idx])
            {
                case 'r':
                    current.RedCount = (std::max)(current.RedCount, num);
                    idx += 3;
                    break;
                case 'g':
                    current.GreenCount = (std::max)(current.GreenCount, num);
                    idx += 5;
                    break;
                case 'b':
                    current.BlueCount = (std::max)(current.BlueCount, num);
                    idx += 4;
                    break;
                default:
                    printf("unexpected input from line\n");
                    return 1;
            }
            // space
            idx += 2;
        }
        games.push_back(current);

        ++lineIdx;
    }

    //
    // Part 1
    //

    int sum1 = 0;
    for (int i = 0; i < games.size(); ++i)
    {
        if (games[i].RedCount <= 12 && games[i].GreenCount <= 13 && games[i].BlueCount <= 14)
            sum1 += (i + 1);
    }

    printf("%d\n", sum1);

    //
    // Part 2
    //

    int sum2 = 0;
    for (const auto& game : games)
    {
        sum2 += (game.RedCount * game.GreenCount * game.BlueCount);
    }

    printf("%d\n", sum2);

    return 0;
}
