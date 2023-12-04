using System.Text.RegularExpressions;

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
int LookupNumber(string s)
{
    switch (s[0])
    {
        case 'z': return 0;
        case 'o': return 1;
        case 't': return (s.Length == 3) ? 2 : 3;
        case 'f': return (s[1] == 'o') ? 4 : 5;
        case 's': return (s.Length == 3) ? 6 : 7;
        case 'e': return 8;
        case 'n': return 9;
        default: return 0;
    }
}

var re2 = new Regex("^.*?([0-9]|one|two|three|four|five|six|seven|eight|nine).*([0-9]|one|two|three|four|five|six|seven|eight|nine).*?$", RegexOptions.Compiled);
var re1 = new Regex("^.*?([0-9]|one|two|three|four|five|six|seven|eight|nine).*$", RegexOptions.Compiled);

int sum2 = 0;
foreach (var line in lines)
{
    var m = re2.Match(line);
    if (m.Success)
    {
        int n1 = (m.Groups[1].Value.Length == 1) ? (m.Groups[1].Value[0] - '0') : LookupNumber(m.Groups[1].Value);
        int n2 = (m.Groups[2].Length == 1) ? (m.Groups[2].Value[0] - '0') : LookupNumber(m.Groups[2].Value);
        sum2 += (10 * n1) + n2;
    }
    else
    {
        m = re1.Match(line);
        if (m.Success)
        {
            int n1 = (m.Groups[1].Value.Length == 1) ? (m.Groups[1].Value[0] - '0') : LookupNumber(m.Groups[1].Value);
            sum2 += (10 * n1) + n1;
        }
        else
        {
            Console.WriteLine($"regex failed for line: {line}");
        }
    }
}

Console.WriteLine(sum2);
