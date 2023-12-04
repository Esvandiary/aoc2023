using System.Text.RegularExpressions;

var lines = File.ReadAllLines("input.txt");

List<Game> games = new(lines.Length);
foreach (var line in lines)
{
    int colonIdx = line.IndexOf(':');

    int idx = colonIdx + 2;
    List<List<GrabResult>> grabs = new();
    List<GrabResult> current = new();
    while (idx < line.Length)
    {
        // number
        int num = 0;
        while (char.IsDigit(line[idx]))
        {
            num *= 10;
            num += (line[idx++] - '0');
        }
        // space
        ++idx;
        int redCount = 0, greenCount = 0, blueCount = 0;
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
                throw new ApplicationException("u wot m8");
        }

        current.Add(new GrabResult { RedCount = redCount, GreenCount = greenCount, BlueCount = blueCount });
        if (idx >= line.Length || line[idx] == ';')
        {
            grabs.Add(current);
            current = new();
        }
        // space
        idx += 2;
    }
    games.Add(new Game { Grabs = grabs });
}

//
// Part 1
//

bool IsOK(List<GrabResult> grabs, int maxRed, int maxGreen, int maxBlue)
{
    foreach (var grab in grabs)
    {
        if (grab.RedCount > maxRed || grab.GreenCount > maxGreen || grab.BlueCount > maxBlue)
            return false;
    }
    return true;
}

int sum1 = 0;
for (int i = 0; i < games.Count; ++i)
{
    if (games[i].Grabs.All(t => IsOK(t, 12, 13, 14)))
        sum1 += (i + 1);
}

Console.WriteLine(sum1);

//
// Part 2
//

int sum2 = 0;
foreach (var game in games)
{
    int maxRed = game.Grabs.Max(t => t.Max(u => u.RedCount));
    int maxGreen = game.Grabs.Max(t => t.Max(u => u.GreenCount));
    int maxBlue = game.Grabs.Max(t => t.Max(u => u.BlueCount));

    sum2 += (maxRed * maxGreen * maxBlue);
}

Console.WriteLine(sum2);

//
// Structs
//

struct GrabResult
{
    public int RedCount;
    public int GreenCount;
    public int BlueCount;

    public override string ToString()
    {
        return $"{Math.Max(RedCount, Math.Max(GreenCount, BlueCount))} {((RedCount > 0) ? "Red" : (GreenCount > 0) ? "Green" : "Blue")}";
    }
}

struct Game
{
    public List<List<GrabResult>> Grabs;
}