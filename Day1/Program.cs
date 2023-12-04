var lines = File.ReadAllLines("input.txt");
int sum1 = 0;

//
// Part 1
//

foreach (var line in lines)
{
    char c1 = line.FirstOrDefault(t => char.IsDigit(t));
    char c2 = line.LastOrDefault(t => char.IsDigit(t));
    if (c1 != 0 && c2 != 0)
        sum1 += (10 * (c1 - '0')) + (c2 - '0');
}

Console.WriteLine(sum1);

//
// Part 2
//
int LookForward(string line)
{
    for (int i = 0; i < line.Length; ++i)
    {
        switch (line[i])
        {
            case >= '0' and <= '9':
                return (line[i] - '0');
            case 'o':
                if (i + 2 < line.Length && line[i + 1] == 'n' && line[i + 2] == 'e')
                    return 1;
                break;
            case 't':
                if (i + 2 < line.Length && line[i + 1] == 'w' && line[i + 2] == 'o')
                    return 2;
                else if (i + 4 < line.Length && line[i + 1] == 'h' && line[i + 2] == 'r' && line[i + 3] == 'e' && line[i + 4] == 'e')
                    return 3;
                break;
            case 'f':
                if (i + 3 < line.Length)
                {
                    if (line[i + 1] == 'o' && line[i + 2] == 'u' && line[i + 3] == 'r')
                        return 4;
                    else if (line[i + 1] == 'i' && line[i + 2] == 'v' && line[i + 3] == 'e')
                        return 5;
                }
                break;
            case 's':
                if (i + 2 < line.Length && line[i + 1] == 'i' && line[i + 2] == 'x')
                    return 6;
                else if (i + 4 < line.Length && line[i + 1] == 'e' && line[i + 2] == 'v' && line[i + 3] == 'e' && line[i + 4] == 'n')
                    return 7;
                break;
            case 'e':
                if (i + 4 < line.Length && line[i + 1] == 'i' && line[i + 2] == 'g' && line[i + 3] == 'h' && line[i + 4] == 't')
                    return 8;
                break;
            case 'n':
                if (i + 3 < line.Length && line[i + 1] == 'i' && line[i + 2] == 'n' && line[i + 3] == 'e')
                    return 9;
                break;
            default:
                break;
        }
    }
    return -1;
}

int LookBackward(string line)
{
    for (int i = line.Length - 1; i >= 0; --i)
    {
        switch (line[i])
        {
            case >= '0' and <= '9':
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
    return -1;
}

int sum2 = 0;
foreach (var line in lines)
{
    int fwd = LookForward(line);
    int bkd = LookBackward(line);
    if (fwd != -1 && bkd != -1)
        sum2 += (10 * fwd) + bkd;
    else
        Console.WriteLine($"lookup failed for line: {line}");
}

Console.WriteLine(sum2);
