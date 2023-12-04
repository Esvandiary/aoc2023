#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

int main(int argc, char** argv)
{
    std::ifstream file("input.txt");
    std::vector<std::string> lines;
    {
        std::string line;
        while (std::getline(file, line))
            lines.emplace_back(std::move(line));
    }
    const int lineCount = lines.size();

    uint8_t* cards = (uint8_t*)malloc(sizeof(uint8_t) * lineCount);
    bool winning[100];
    for (int i = 0; i < lineCount; ++i)
    {
        const auto& line = lines[i];
        memset(winning, 0, sizeof(winning));
        uint8_t matchCount = 0;

        int idx = 10; // 'Card NNN: '
        for (int i = 0; i < 10; ++i)
        {
            int num = 0;
            if (line[idx] >= '0' && line[idx] <= '9')
                num += (uint8_t)(10 * (line[idx] - '0'));
            ++idx;
            num += (uint8_t)(line[idx] - '0');
            winning[num] = true;
            idx += 2;
        }
        idx += 2; // '| '
        for (int i = 0; i < 25; ++i)
        {
            int num = 0;
            if (line[idx] >= '0' && line[idx] <= '9')
                num += (uint8_t)(10 * (line[idx] - '0'));
            ++idx;
            num += (uint8_t)(line[idx] - '0');
            idx += 2;

            if (winning[num])
                ++matchCount;
        }

        cards[i] = matchCount;
    }

    //
    // Part 1
    //

    int sum1 = 0;

    for (int i = 0; i < lineCount; ++i)
        sum1 += (1U << (cards[i] - 1));

    printf("%d\n", sum1);

    //
    // Part 2
    //

    uint64_t* counts = (uint64_t*)calloc(lineCount, sizeof(uint64_t));
    for (int i = 0; i < lineCount; ++i)
    {
        counts[i] += 1;
        for (int ni = 1; i + ni < lineCount && ni <= cards[i]; ++ni)
            counts[i + ni] += counts[i];
    }

    uint64_t sum2 = 0;
    for (int i = 0; i < lineCount; ++i)
        sum2 += counts[i];

    printf("%zu\n", sum2);

    return 0;
}