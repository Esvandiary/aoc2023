#include <algorithm>
#include <cstdint>
#include <cstdlib>
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

    const int lineLength = lines[0].length();
    const int lineCount = lines.size();
    const int textLength = lineCount * lineLength;

    bool* numbers = (bool*)calloc(textLength, sizeof(bool));
    bool* symbols = (bool*)calloc(textLength, sizeof(bool));
    std::vector<int> gears;
    gears.reserve(lineCount * 5);

    for (int y = 0; y < lineCount; ++y)
    {
        for (int x = 0; x < lineLength; ++x)
        {
            int idx = (y * lineLength) + x;
            if (isdigit(lines[y][x]))
            {
                numbers[idx] = true;
            }
            else if (lines[y][x] != '.')
            {
                symbols[idx] = true;
                if (lines[y][x] == '*')
                    gears.push_back(y * lineLength + x);
            }
        }
    }

    //
    // Part 1
    //

    int sum1 = 0;

    for (int y = 0; y < lineCount; ++y)
    {
        for (int x = 0; x < lineLength; ++x)
        {
            int startX = x;
            int startIdx = (y * lineLength + x);
            int idx = startIdx;
            int num = 0;
            while (x < lineLength && numbers[idx])
            {
                num *= 10;
                num += lines[y][x] - '0';
                ++x;
                ++idx;
            }
            if (idx != startIdx)
            {
                if ((startX != 0 && symbols[startIdx - 1]) || (x != lineLength && symbols[idx]))
                    goto hit;
                if (y != 0)
                {
                    int cidx = ((y - 1) * lineLength) + (std::max)(0, startX - 1);
                    int cidxEnd = ((y - 1) * lineLength) + (std::min)(lineLength - 1, x);
                    while (cidx <= cidxEnd)
                    {
                        if (symbols[cidx++])
                            goto hit;
                    }
                }
                if (y + 1 != lineCount)
                {
                    int cidx = ((y + 1) * lineLength) + (std::max)(0, startX - 1);
                    int cidxEnd = ((y + 1) * lineLength) + (std::min)(lineLength - 1, x);
                    while (cidx <= cidxEnd)
                    {
                        if (symbols[cidx++])
                            goto hit;
                    }
                }
            }
            continue;
        hit:
            sum1 += num;
        }
    }

    printf("%d\n", sum1);

    //
    // Part 2
    //

    int sum2 = 0;

    for (int gidx : gears)
    {
        int y = gidx / lineLength;
        int x = gidx % lineLength;

        int numCount = 0;
        int mul = 1;

        for (int cy = (std::max)(0, y - 1); cy <= (std::min)(lineCount - 1, y + 1); ++cy)
        {
            for (int cx = (std::max)(0, x - 1); cx <= (std::min)(lineLength - 1, x + 1); ++cx)
            {
                if (numbers[(cy * lineLength) + cx])
                {
                    // find start of number
                    int startX = cx;
                    while (startX >= 1 && numbers[(cy * lineLength) + startX - 1])
                        --startX;
                    int endX = startX;
                    int num = 0;
                    while (endX < lineLength && numbers[(cy * lineLength) + endX])
                    {
                        num *= 10;
                        num += lines[cy][endX] - '0';
                        ++endX;
                    }

                    mul *= num;
                    ++numCount;
                    cx = endX;
                }
            }
        }

        if (numCount == 2)
            sum2 += mul;
    }

    printf("%d\n", sum2);

    return 0;
}