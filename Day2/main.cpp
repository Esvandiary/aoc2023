#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

struct GrabResult
{
    uint16_t RedCount;
    uint16_t GreenCount;
    uint16_t BlueCount;
};

static bool IsOK(const std::vector<std::vector<GrabResult>>& game, short maxRed, short maxGreen, short maxBlue)
{
    for (const auto& grabs : game)
    {
        for (const auto& grab : grabs)
        {
            if (grab.RedCount > maxRed || grab.GreenCount > maxGreen || grab.BlueCount > maxBlue)
                return false;
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    std::ifstream file("input.txt");
    std::vector<std::string> lines;
    {
        std::string line;
        while (std::getline(file, line))
            lines.emplace_back(std::move(line));
    }

    std::vector<std::vector<std::vector<GrabResult>>> games;
    games.reserve(lines.size());
    for (const auto& line : lines)
    {
        const size_t lineLength = line.length();
        size_t colonIdx = line.find_first_of(':');

        size_t idx = colonIdx + 2;
        std::vector<std::vector<GrabResult>> grabs;
        std::vector<GrabResult> current;
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
                    redCount = num;
                    idx += 3;
                    break;
                case 'g':
                    greenCount = num;
                    idx += 5;
                    break;
                case 'b':
                    blueCount = num;
                    idx += 4;
                    break;
                default:
                    printf("unexpected input from line: %s\n", line.c_str());
                    return 1;
            }

            current.push_back(GrabResult { redCount, greenCount, blueCount });
            if (idx >= lineLength || line[idx] == ';')
                grabs.push_back(std::move(current));
            // space
            idx += 2;
        }
        games.push_back(std::move(grabs));
    }

    //
    // Part 1
    //

    int sum1 = 0;
    for (int i = 0; i < games.size(); ++i)
    {
        if (IsOK(games[i], 12, 13, 14))
            sum1 += (i + 1);
    }

    printf("%d\n", sum1);

    //
    // Part 2
    //

    int sum2 = 0;
    for (const auto& game : games)
    {
        uint16_t maxRed = 0, maxGreen = 0, maxBlue = 0;
        for (const auto& grabs : game)
        {
            for (const auto& grab : grabs)
            {
                maxRed = (std::max)(maxRed, grab.RedCount);
                maxGreen = (std::max)(maxGreen, grab.GreenCount);
                maxBlue = (std::max)(maxBlue, grab.BlueCount);
            }
        }

        sum2 += (maxRed * maxGreen * maxBlue);
    }

    printf("%d\n", sum2);

    return 0;
}