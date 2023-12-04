#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../common/mmap.hpp"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

int main(int argc, char** argv)
{
    auto file = mmap_file::open_ro("input.txt");

    std::vector<uint8_t> cards;
    cards.reserve(1024);
    bool winning[100];

    int lineIdx = 0;
    int lineNum = 0;
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

        memset(winning, 0, sizeof(winning));
        uint8_t matchCount = 0;

        int idx = 0;
        while (idx < line.size() && line[idx] != ':')
            ++idx;
        idx += 2; // ': '
        while (idx < line.size() && line[idx] != '|')
        {
            int num = 0;
            if (isdigit(line[idx]))
                num += (uint8_t)(10 * (line[idx] - '0'));
            ++idx;
            num += (uint8_t)(line[idx] - '0');
            winning[num] = true;
            idx += 2;
        }
        idx += 2; // '| '
        while (idx < line.size())
        {
            int num = 0;
            if (isdigit(line[idx]))
                num += (uint8_t)(10 * (line[idx] - '0'));
            ++idx;
            num += (uint8_t)(line[idx] - '0');
            idx += 2;

            if (winning[num])
                ++matchCount;
        }

        cards.push_back(matchCount);
        ++lineNum;
    }

    //
    // Part 1
    //

    int sum1 = 0;

    for (uint8_t c : cards)
        sum1 += (c ? (1U << (c - 1)) : 0);

    printf("%d\n", sum1);

    //
    // Part 2
    //

    uint64_t* counts = (uint64_t*)calloc(lineNum, sizeof(uint64_t));
    for (int i = 0; i < lineNum; ++i)
    {
        counts[i] += 1;
        for (int ni = 1; i + ni < lineNum && ni <= cards[i]; ++ni)
            counts[i + ni] += counts[i];
    }

    uint64_t sum2 = 0;
    for (int i = 0; i < lineNum; ++i)
        sum2 += counts[i];

    printf("%zu\n", sum2);

    return 0;
}

