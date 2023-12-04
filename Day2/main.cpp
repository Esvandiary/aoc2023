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
    const int fileSize = static_cast<int>(file.size());

    std::vector<GrabResult> games;
    games.reserve(512);
    
    int lineIdx = 0;
    while (lineIdx < fileSize)
    {
        const int lineStart = lineIdx;
        for (; lineIdx < fileSize; ++lineIdx)
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
        while (colonIdx < lineLength && line[colonIdx] != ':')
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
            switch (line[idx])
            {
                case 'r':
                    current.RedCount = (std::max)(current.RedCount, num);
                    idx += 5; // 'red[,;] '
                    break;
                case 'g':
                    current.GreenCount = (std::max)(current.GreenCount, num);
                    idx += 7; // 'green[,;] '
                    break;
                case 'b':
                    current.BlueCount = (std::max)(current.BlueCount, num);
                    idx += 6; // 'blue[,;] '
                    break;
                default:
                    printf("unexpected input from line\n");
                    return 1;
            }
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
        const int mask = ((games[i].RedCount - 13) & (games[i].GreenCount - 14) & (games[i].BlueCount - 15)) >> 31;
        sum1 += (i + 1) & mask;
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
