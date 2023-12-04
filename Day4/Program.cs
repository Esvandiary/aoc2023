using System;

var lines = File.ReadAllLines("input.txt");

List<Card> cards = new(lines.Length);
foreach (var line in lines)
{
    bool[] winning = new bool[100];
    byte[] played = new byte[25];

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
        if (line[idx] >= '0' && line[idx] <= '9')
            played[i] += (byte)(10 * (line[idx] - '0'));
        ++idx;
        played[i] += (byte)(line[idx] - '0');
        idx += 2;
    }

    cards.Add(new Card(winning, played));
}

//
// Part 1
//

int sum1 = 0;
foreach (var card in cards)
{
    card.MatchCount = (byte)card.PlayedNumbers.Count(p => card.WinningNumbers[p]);
    card.Score = unchecked((ushort)(1U << (card.MatchCount - 1)));
    sum1 += card.Score;
}

Console.WriteLine(sum1);

//
// Part 2
//

long[] counts = new long[cards.Count];
for (int i = 0; i < cards.Count; ++i)
{
    counts[i] += 1;
    for (int ni = 1; i + ni < cards.Count && ni <= cards[i].MatchCount; ++ni)
        counts[i + ni] += counts[i];
}

long sum2 = counts.Sum();

Console.WriteLine(sum2);

//
// Structs
//

class Card
{
    public Card(bool[] winning, byte[] played)
    {
        WinningNumbers = winning;
        PlayedNumbers = played;
    }

    public bool[] WinningNumbers;
    public byte[] PlayedNumbers;
    public byte MatchCount;
    public ushort Score;
}