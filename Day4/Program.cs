using System;

var lines = File.ReadAllLines("input.txt");

List<byte> cards = new(lines.Length);
foreach (var line in lines)
{
    bool[] winning = new bool[100];
    byte matchCount = 0;

    int idx = 10; // 'Card NNN: '
    for (int i = 0; i < 10; ++i)
    {
        int num = 0;
        if (line[idx] >= '0' && line[idx] <= '9')
            num += (byte)(10 * (line[idx] - '0'));
        ++idx;
        num += (byte)(line[idx] - '0');
        winning[num] = true;
        idx += 2;
    }
    idx += 2; // '| '
    for (int i = 0; i < 25; ++i)
    {
        int num = 0;
        if (line[idx] >= '0' && line[idx] <= '9')
            num += (byte)(10 * (line[idx] - '0'));
        ++idx;
        num += (byte)(line[idx] - '0');
        idx += 2;

        if (winning[num])
            ++matchCount;
    }

    cards.Add(matchCount);
}

//
// Part 1
//

int sum1 = 0;
foreach (var matchCount in cards)
    sum1 += unchecked((ushort)(1U << (matchCount - 1)));

Console.WriteLine(sum1);

//
// Part 2
//

long[] counts = new long[cards.Count];
for (int i = 0; i < cards.Count; ++i)
{
    counts[i] += 1;
    for (int ni = 1; i + ni < cards.Count && ni <= cards[i]; ++ni)
        counts[i + ni] += counts[i];
}

long sum2 = counts.Sum();

Console.WriteLine(sum2);
