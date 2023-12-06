#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')

typedef struct LookResult
{
    int8_t digit;
    int8_t closest;
} LookResult;

static LookResult LookForward(const view* line)
{
    const size_t lineLength = line->size;
    int16_t letter = -1;
    int i;
    for (i = 0; i < lineLength; ++i)
    {
        switch (line->data[i])
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
                return (LookResult) { .digit = (int8_t)(line->data[i] & 0xF), .closest = (int8_t)(line->data[i] & 0xF) };
            case 'o':
                if (i + 2 < lineLength && line->data[i + 1] == 'n' && line->data[i + 2] == 'e')
                {
                    letter = 1;
                    i += 3;
                    goto digitonly;
                }
                break;
            case 't':
                if (i + 2 < lineLength && line->data[i + 1] == 'w' && line->data[i + 2] == 'o')
                {
                    letter = 2;
                    i += 3;
                    goto digitonly;
                }
                else if (i + 4 < lineLength && line->data[i + 1] == 'h' && line->data[i + 2] == 'r' && line->data[i + 3] == 'e' && line->data[i + 4] == 'e')
                {
                    letter = 3;
                    i += 5;
                    goto digitonly;
                }
                break;
            case 'f':
                if (i + 3 < lineLength)
                {
                    if (line->data[i + 1] == 'o' && line->data[i + 2] == 'u' && line->data[i + 3] == 'r')
                    {
                        letter = 4;
                        i += 4;
                        goto digitonly;
                    }
                    else if (line->data[i + 1] == 'i' && line->data[i + 2] == 'v' && line->data[i + 3] == 'e')
                    {
                        letter = 5;
                        i += 4;
                        goto digitonly;
                    }
                }
                break;
            case 's':
                if (i + 2 < lineLength && line->data[i + 1] == 'i' && line->data[i + 2] == 'x')
                {
                    letter = 6;
                    i += 3;
                    goto digitonly;
                }
                else if (i + 4 < lineLength && line->data[i + 1] == 'e' && line->data[i + 2] == 'v' && line->data[i + 3] == 'e' && line->data[i + 4] == 'n')
                {
                    letter = 7;
                    i += 5;
                    goto digitonly;
                }
                break;
            case 'e':
                if (i + 4 < lineLength && line->data[i + 1] == 'i' && line->data[i + 2] == 'g' && line->data[i + 3] == 'h' && line->data[i + 4] == 't')
                {
                    letter = 8;
                    i += 5;
                    goto digitonly;
                }
                break;
            case 'n':
                if (i + 3 < lineLength && line->data[i + 1] == 'i' && line->data[i + 2] == 'n' && line->data[i + 3] == 'e')
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
        if (isdigit(line->data[i]))
            return (LookResult) { .digit = (int8_t)(line->data[i] & 0xF), .closest = letter };
    }
    return (LookResult) { .digit = -1, .closest = letter };
}

static LookResult LookBackward(const view* line)
{
    const size_t lineLength = line->size;
    int16_t letter = -1;
    int i;
    for (i = lineLength - 1; i >= 0; --i)
    {
        switch (line->data[i])
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
                return (LookResult) { .digit = (int8_t)(line->data[i] & 0xF), .closest = (int8_t)(line->data[i] & 0xF) };
            case 'e':
                if (i >= 2 && line->data[i - 1] == 'n' && line->data[i - 2] == 'o')
                {
                    letter = 1;
                    i -= 3;
                    goto digitonly;
                }
                else if (i >= 4 && line->data[i - 1] == 'e' && line->data[i - 2] == 'r' && line->data[i - 3] == 'h' && line->data[i - 4] == 't')
                {
                    letter = 3;
                    i -= 5;
                    goto digitonly;
                }
                else if (i >= 3 && line->data[i - 1] == 'v' && line->data[i - 2] == 'i' && line->data[i - 3] == 'f')
                {
                    letter = 5;
                    i -= 4;
                    goto digitonly;
                }
                else if (i >= 3 && line->data[i - 1] == 'n' && line->data[i - 2] == 'i' && line->data[i - 3] == 'n')
                {
                    letter = 9;
                    i -= 4;
                    goto digitonly;
                }
                break;
            case 'o':
                if (i >= 2 && line->data[i - 1] == 'w' && line->data[i - 2] == 't')
                {
                    letter = 2;
                    i -= 3;
                    goto digitonly;
                }
                break;
            case 'r':
                if (i >= 3 && line->data[i - 1] == 'u' && line->data[i - 2] == 'o' && line->data[i - 3] == 'f')
                {
                    letter = 4;
                    i -= 4;
                    goto digitonly;
                }
                break;
            case 'x':
                if (i >= 2 && line->data[i - 1] == 'i' && line->data[i - 2] == 's')
                {
                    letter = 6;
                    i -= 3;
                    goto digitonly;
                }
                break;
            case 'n':
                if (i >= 4 && line->data[i - 1] == 'e' && line->data[i - 2] == 'v' && line->data[i - 3] == 'e' && line->data[i - 4] == 's')
                {
                    letter = 7;
                    i -= 5;
                    goto digitonly;
                }
                break;
            case 't':
                if (i >= 4 && line->data[i - 1] == 'h' && line->data[i - 2] == 'g' && line->data[i - 3] == 'i' && line->data[i-4] == 'e')
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
        if (isdigit(line->data[i]))
            return (LookResult) { .digit = (int8_t)(line->data[i] & 0xF), .closest = letter };
    }
    return (LookResult) { .digit = -1, .closest = letter };
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

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
            if (file.data[idx] == '\n')
                break;
        }
        if (UNLIKELY(idx - lineStart < 2))
        {
            ++idx;
            continue;
        }
        view line = { .data = file.data + lineStart, .size = idx - lineStart };

        LookResult first = LookForward(&line);
        LookResult last = LookBackward(&line);
        sum1 += (10 * first.digit) + last.digit;
        sum2 += (10 * first.closest) + last.closest;

        ++idx;
    }

    print_uint64(sum1);
    print_uint64(sum2);

    return 0;
}
