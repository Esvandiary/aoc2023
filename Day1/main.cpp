#include <algorithm>
#include "../common/mmap.hpp"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

struct LookResult
{
    int16_t digit;
    int16_t closest;
};

static LookResult LookForward(const view<chartype> line)
{
    const size_t lineLength = line.size();
    int16_t letter = -1;
    int i;
    for (i = 0; i < lineLength; ++i)
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
                return LookResult {int16_t(line[i] - '0'), int16_t(line[i] - '0')};
            case 'o':
                if (i + 2 < lineLength && line[i + 1] == 'n' && line[i + 2] == 'e')
                {
                    letter = 1;
                    i += 3;
                    goto digitonly;
                }
                break;
            case 't':
                if (i + 2 < lineLength && line[i + 1] == 'w' && line[i + 2] == 'o')
                {
                    letter = 2;
                    i += 3;
                    goto digitonly;
                }
                else if (i + 4 < lineLength && line[i + 1] == 'h' && line[i + 2] == 'r' && line[i + 3] == 'e' && line[i + 4] == 'e')
                {
                    letter = 3;
                    i += 5;
                    goto digitonly;
                }
                break;
            case 'f':
                if (i + 3 < lineLength)
                {
                    if (line[i + 1] == 'o' && line[i + 2] == 'u' && line[i + 3] == 'r')
                    {
                        letter = 4;
                        i += 4;
                        goto digitonly;
                    }
                    else if (line[i + 1] == 'i' && line[i + 2] == 'v' && line[i + 3] == 'e')
                    {
                        letter = 5;
                        i += 4;
                        goto digitonly;
                    }
                }
                break;
            case 's':
                if (i + 2 < lineLength && line[i + 1] == 'i' && line[i + 2] == 'x')
                {
                    letter = 6;
                    i += 3;
                    goto digitonly;
                }
                else if (i + 4 < lineLength && line[i + 1] == 'e' && line[i + 2] == 'v' && line[i + 3] == 'e' && line[i + 4] == 'n')
                {
                    letter = 7;
                    i += 5;
                    goto digitonly;
                }
                break;
            case 'e':
                if (i + 4 < lineLength && line[i + 1] == 'i' && line[i + 2] == 'g' && line[i + 3] == 'h' && line[i + 4] == 't')
                {
                    letter = 8;
                    i += 5;
                    goto digitonly;
                }
                break;
            case 'n':
                if (i + 3 < lineLength && line[i + 1] == 'i' && line[i + 2] == 'n' && line[i + 3] == 'e')
                {
                    letter = 9;
                    i += 4;
                    goto digitonly;
                }
                break;
            default:
                break;
        }
    }
digitonly:
    for (; i < lineLength; ++i)
    {
        if (isdigit(line[i]))
            return LookResult {int16_t(line[i] - '0'), letter};
    }
    return LookResult {-1, letter};
}

static LookResult LookBackward(const view<chartype> line)
{
    const size_t lineLength = line.size();
    int16_t letter = -1;
    int i;
    for (i = lineLength - 1; i >= 0; --i)
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
                return LookResult { int16_t(line[i] - '0'), int16_t(line[i] - '0') };
            case 'e':
                if (i >= 2 && line[i - 1] == 'n' && line[i - 2] == 'o')
                {
                    letter = 1;
                    i -= 3;
                    goto digitonly;
                }
                else if (i >= 4 && line[i - 1] == 'e' && line[i - 2] == 'r' && line[i - 3] == 'h' && line[i - 4] == 't')
                {
                    letter = 3;
                    i -= 5;
                    goto digitonly;
                }
                else if (i >= 3 && line[i - 1] == 'v' && line[i - 2] == 'i' && line[i - 3] == 'f')
                {
                    letter = 5;
                    i -= 4;
                    goto digitonly;
                }
                else if (i >= 3 && line[i - 1] == 'n' && line[i - 2] == 'i' && line[i - 3] == 'n')
                {
                    letter = 9;
                    i -= 4;
                    goto digitonly;
                }
                break;
            case 'o':
                if (i >= 2 && line[i - 1] == 'w' && line[i - 2] == 't')
                {
                    letter = 2;
                    i -= 3;
                    goto digitonly;
                }
                break;
            case 'r':
                if (i >= 3 && line[i - 1] == 'u' && line[i - 2] == 'o' && line[i - 3] == 'f')
                {
                    letter = 4;
                    i -= 4;
                    goto digitonly;
                }
                break;
            case 'x':
                if (i >= 2 && line[i - 1] == 'i' && line[i - 2] == 's')
                {
                    letter = 6;
                    i -= 3;
                    goto digitonly;
                }
                break;
            case 'n':
                if (i >= 4 && line[i - 1] == 'e' && line[i - 2] == 'v' && line[i - 3] == 'e' && line[i - 4] == 's')
                {
                    letter = 7;
                    i -= 5;
                    goto digitonly;
                }
                break;
            case 't':
                if (i >= 4 && line[i - 1] == 'h' && line[i - 2] == 'g' && line[i - 3] == 'i' && line[i-4] == 'e')
                {
                    letter = 8;
                    i -= 5;
                    goto digitonly;
                }
                break;
            default:
                break;
        }
    }
digitonly:
    for (; i >= 0; --i)
    {
        if (isdigit(line[i]))
            return LookResult {int16_t(line[i] - '0'), letter};
    }
    return LookResult {-1, letter};
}

int main(int argc, char** argv)
{
    auto file = mmap_file::open_ro("input.txt");
    const int fileSize = static_cast<int>(file.size());

    //
    // Part 1 + 2
    //

    int sum1 = 0, sum2 = 0;
    int idx = 0;
    while (idx < fileSize)
    {
        const int lineStart = idx;
        for (; idx < fileSize; ++idx)
        {
            if (file.data()[idx] == '\n')
                break;
        }
        if (UNLIKELY(idx - lineStart < 2))
        {
            ++idx;
            continue;
        }
        const view<chartype> line = file.slice(lineStart, idx - lineStart);

        LookResult first = LookForward(line);
        LookResult last = LookBackward(line);
        sum1 += (10 * first.digit) + last.digit;
        sum2 += (10 * first.closest) + last.closest;

        ++idx;
    }

    printf("%d\n", sum1);
    printf("%d\n", sum2);

    return 0;
}
