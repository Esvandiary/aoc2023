#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#define isdigit(c) ((c) >= '0' && (c) <= '9')

static inline char FirstDigit(const std::string& s)
{
    for (auto it = s.begin(); it != s.end(); ++it)
        if (isdigit(*it))
            return *it;
    return '\0';
}

static inline char LastDigit(const std::string& s)
{
    for (auto it = s.rbegin(); it != s.rend(); ++it)
        if (isdigit(*it))
            return *it;
    return '\0';
}

static int LookForward(const std::string& line)
{
    const size_t lineLength = line.length();
    for (int i = 0; i < lineLength; ++i)
    {
        switch (line[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return (line[i] - '0');
            case 'o':
                if (i + 2 < lineLength && line[i + 1] == 'n' && line[i + 2] == 'e')
                    return 1;
                break;
            case 't':
                if (i + 2 < lineLength && line[i + 1] == 'w' && line[i + 2] == 'o')
                    return 2;
                else if (i + 4 < lineLength && line[i + 1] == 'h' && line[i + 2] == 'r' && line[i + 3] == 'e' && line[i + 4] == 'e')
                    return 3;
                break;
            case 'f':
                if (i + 3 < lineLength)
                {
                    if (line[i + 1] == 'o' && line[i + 2] == 'u' && line[i + 3] == 'r')
                        return 4;
                    else if (line[i + 1] == 'i' && line[i + 2] == 'v' && line[i + 3] == 'e')
                        return 5;
                }
                break;
            case 's':
                if (i + 2 < lineLength && line[i + 1] == 'i' && line[i + 2] == 'x')
                    return 6;
                else if (i + 4 < lineLength && line[i + 1] == 'e' && line[i + 2] == 'v' && line[i + 3] == 'e' && line[i + 4] == 'n')
                    return 7;
                break;
            case 'e':
                if (i + 4 < lineLength && line[i + 1] == 'i' && line[i + 2] == 'g' && line[i + 3] == 'h' && line[i + 4] == 't')
                    return 8;
                break;
            case 'n':
                if (i + 3 < lineLength && line[i + 1] == 'i' && line[i + 2] == 'n' && line[i + 3] == 'e')
                    return 9;
                break;
            default:
                break;
        }
    }
    return 0;
}

static int LookBackward(const std::string& line)
{
    const size_t lineLength = line.length();
    for (int i = lineLength - 1; i >= 0; --i)
    {
        switch (line[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return (line[i] - '0');
            case 'e':
                if (i >= 2 && line[i - 1] == 'n' && line[i - 2] == 'o')
                    return 1;
                else if (i >= 4 && line[i - 1] == 'e' && line[i - 2] == 'r' && line[i - 3] == 'h' && line[i - 4] == 't')
                    return 3;
                else if (i >= 3 && line[i - 1] == 'v' && line[i - 2] == 'i' && line[i - 3] == 'f')
                    return 5;
                else if (i >= 3 && line[i - 1] == 'n' && line[i - 2] == 'i' && line[i - 3] == 'n')
                    return 9;
                break;
            case 'o':
                if (i >= 2 && line[i - 1] == 'w' && line[i - 2] == 't')
                    return 2;
                break;
            case 'r':
                if (i >= 3 && line[i - 1] == 'u' && line[i - 2] == 'o' && line[i - 3] == 'f')
                    return 4;
                break;
            case 'x':
                if (i >= 2 && line[i - 1] == 'i' && line[i - 2] == 's')
                    return 6;
                break;
            case 'n':
                if (i >= 4 && line[i - 1] == 'e' && line[i - 2] == 'v' && line[i - 3] == 'e' && line[i - 4] == 's')
                    return 7;
                break;
            case 't':
                if (i >= 4 && line[i - 1] == 'h' && line[i - 2] == 'g' && line[i - 3] == 'i' && line[i-4] == 'e')
                    return 8;
                break;
            default:
                break;
        }
    }
    return 0;
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

    //
    // Part 1
    //

    int sum1 = 0;
    for (const auto& line : lines)
    {
        char c1 = FirstDigit(line);
        char c2 = LastDigit(line);
        if (c1 != 0 && c2 != 0)
            sum1 += (10 * (c1 - '0')) + (c2 - '0');
    }

    printf("%d\n", sum1);

    //
    // Part 2
    //

    int sum2 = 0;
    for (const auto& line : lines)
    {
        int fwd = LookForward(line);
        int bkd = LookBackward(line);
        sum2 += (10 * fwd) + bkd;
    }

    printf("%d\n", sum2);

    return 0;
}