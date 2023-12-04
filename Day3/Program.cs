using System.Collections;

var lines = File.ReadAllLines("input.txt");
var lineLength = lines[0].Length;
var textLength = lines.Length * lineLength;

bool[] numbers = new bool[textLength];
bool[] symbols = new bool[textLength];
List<int> gears = new();

for (int y = 0; y < lines.Length; ++y)
{
    for (int x = 0; x < lineLength; ++x)
    {
        int idx = (y * lineLength) + x;
        if (char.IsDigit(lines[y][x]))
        {
            numbers[idx] = true;
        }
        else if (lines[y][x] != '.')
        {
            symbols[idx] = true;
            if (lines[y][x] == '*')
                gears.Add(y * lineLength + x);
        }
    }
}

//
// Part 1
//

int sum1 = 0;
for (int y = 0; y < lines.Length; ++y)
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
                int cidx = ((y - 1) * lineLength) + Math.Max(0, startX - 1);
                int cidxEnd = ((y - 1) * lineLength) + Math.Min(lineLength - 1, x);
                while (cidx <= cidxEnd)
                {
                    if (symbols[cidx++])
                        goto hit;
                }
            }
            if (y + 1 != lines.Length)
            {
                int cidx = ((y + 1) * lineLength) + Math.Max(0, startX - 1);
                int cidxEnd = ((y + 1) * lineLength) + Math.Min(lineLength - 1, x);
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

Console.WriteLine(sum1);

//
// Part 2
//

int sum2 = 0;
foreach (var gidx in gears)
{
    int y = gidx / lineLength;
    int x = gidx % lineLength;

    int numCount = 0;
    int mul = 1;

    for (int cy = Math.Max(0, y - 1); cy <= Math.Min(lines.Length - 1, y + 1); ++cy)
    {
        for (int cx = Math.Max(0, x - 1); cx <= Math.Min(lineLength - 1, x + 1); ++cx)
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

Console.WriteLine(sum2);