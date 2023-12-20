#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define dataindex(y, x) ((y)*d.lineLength + (x))

#define ACCEPT (32767)
#define REJECT (32766)

typedef struct wfcondition
{
    uint16_t idx;
    uint16_t min;
    uint16_t max;
    uint16_t next;
} wfcondition;

typedef struct workflow
{
    wfcondition conditions[4];
    uint16_t passnext;
} workflow;

#define WFIDX(a, b, c) ((((a) & 0x1F) << 10) | (((b) & 0x1F) << 5) | ((c) & 0x1F))
#define WFINIT { 0 };

#define WFCHARS(idx) ((idx) == ACCEPT ? 'A' : ((idx) == REJECT ? 'R' : (((idx) >> 10) & 0x1F) + 96)), ((idx) < 32760 ? ((((idx) >> 5) & 0x1F) + 96) : ' '), ((((idx) < 32760) && (((idx) & 0x1F) != 0)) ? (((idx) & 0x1F) + 96) : ' ')

static inline FORCEINLINE uint8_t chidx(chartype c)
{
    switch (c)
    {
        case 'x': return 0;
        case 'm': return 1;
        case 'a': return 2;
        case 's': return 3;
    }
    return -1;
}

static uint64_t count_accepts(workflow* workflows, uint16_t wfidx, uint16_t c, uint16_t min[4], uint16_t max[4], int depth)
{
    if (wfidx == REJECT)
    {
        for (int i = 0; i < depth; ++i)
            DEBUGLOG("  ");
        DEBUGLOG("count_accepts(wf, REJECT, %u, [%u,%u,%u,%u], [%u,%u,%u,%u]) --> 0\n", c, min[0], min[1], min[2], min[3], max[0], max[1], max[2], max[3]);
        return 0;
    }
    if (wfidx == ACCEPT)
    {
        const uint64_t result = (int64_t)(1 + max[0] - min[0]) * (int64_t)(1 + max[1] - min[1]) * (int64_t)(1 + max[2] - min[2]) * (int64_t)(1 + max[3] - min[3]);
        for (int i = 0; i < depth; ++i)
            DEBUGLOG("  ");
        DEBUGLOG("count_accepts(wf, ACCEPT, %u, [%u,%u,%u,%u], [%u,%u,%u,%u]) --> %ld\n", c, min[0], min[1], min[2], min[3], max[0], max[1], max[2], max[3], result);
        return result;
    }
    
    for (int i = 0; i < depth; ++i)
        DEBUGLOG("  ");
    DEBUGLOG("count_accepts(wf, [%c%c%c], %u, [%u,%u,%u,%u], [%u,%u,%u,%u])\n", WFCHARS(wfidx), c, min[0], min[1], min[2], min[3], max[0], max[1], max[2], max[3]);

    uint64_t count = 0;
    const uint16_t cidx = workflows[wfidx].conditions[c].idx;
    const uint16_t cimin = workflows[wfidx].conditions[c].min;
    const uint16_t cimax = workflows[wfidx].conditions[c].max;
    if (min[cidx] < cimax || max[cidx] > cimin)
    {
        uint16_t cmin[4] = {min[0], min[1], min[2], min[3]};
        uint16_t cmax[4] = {max[0], max[1], max[2], max[3]};
        cmin[cidx] = MAX(cmin[cidx], cimin);
        cmax[cidx] = MIN(cmax[cidx], cimax);
        count += count_accepts(workflows, workflows[wfidx].conditions[c].next, 0, cmin, cmax, depth + 1);
    }
    bool lastc = (c == 3 || workflows[wfidx].conditions[c+1].next == 0);
    if (min[cidx] < cimin)
    {
        uint16_t cmin[4] = {min[0], min[1], min[2], min[3]};
        uint16_t cmax[4] = {max[0], max[1], max[2], max[3]};
        cmax[cidx] = MIN(cmax[cidx], cimin - 1);
        if (!lastc)
            count += count_accepts(workflows, wfidx, c+1, cmin, cmax, depth + 1);
        else
            count += count_accepts(workflows, workflows[wfidx].passnext, 0, cmin, cmax, depth + 1);
    }
    if (max[cidx] > cimax)
    {
        uint16_t cmin[4] = {min[0], min[1], min[2], min[3]};
        uint16_t cmax[4] = {max[0], max[1], max[2], max[3]};
        cmin[cidx] = MAX(cmin[cidx], cimax + 1);
        if (!lastc)
            count += count_accepts(workflows, wfidx, c+1, cmin, cmax, depth + 1);
        else
            count += count_accepts(workflows, workflows[wfidx].passnext, 0, cmin, cmax, depth + 1);
    }

    for (int i = 0; i < depth; ++i)
        DEBUGLOG("  ");
    DEBUGLOG("count_accepts(wf, [%c%c%c], %u, [%u,%u,%u,%u], [%u,%u,%u,%u]) --> %ld\n", WFCHARS(wfidx), c, min[0], min[1], min[2], min[3], max[0], max[1], max[2], max[3], count);
    return count;
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(part1);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    workflow workflows[32768];

    uint64_t sum1 = 0, sum2 = 0;

    while (*data != '\n')
    {
        DEBUGLOG("starting line\n");
        chartype a = *data++;
        chartype b = *data++;
        chartype c = (*data != '{') ? *data++ : 0;
        workflow wf = WFINIT;
        size_t condcount = 0;
        while (*data != '}')
        {
            ++data;
            chartype c1 = *data++;
            chartype c2 = *data++;
            switch (c2)
            {
                case '}':
                    wf.passnext = (c1 == 'A') ? ACCEPT : REJECT;
                    goto afterbrace;
                case '>':
                case '<':
                    {
                        wf.conditions[condcount].idx = chidx(c1);
                        uint16_t num = 0;
                        while (isdigit(*data))
                        {
                            num *= 10;
                            num += *data++ & 0xF;
                        }
                        wf.conditions[condcount].min = (c2 == '>') ? num + 1 : 1;
                        wf.conditions[condcount].max = (c2 == '>') ? 4000 : num - 1;
                        ++data; // ':'
                        chartype nc1 = *data++;
                        switch (nc1)
                        {
                            case 'A':
                                wf.conditions[condcount].next = ACCEPT;
                                goto afterfn;
                            case 'R':
                                wf.conditions[condcount].next = REJECT;
                                goto afterfn;
                            default:
                                {
                                    chartype nc2 = *data++;
                                    chartype nc3 = (*data != ',') ? *data++ : 0;
                                    wf.conditions[condcount].next = WFIDX(nc1, nc2, nc3);
                                }
                                break;
                        }
                    afterfn:
                        ++condcount;
                    }
                    break;
                default:
                    {
                        chartype c3 = (*data != '}') ? *data++ : 0;
                        wf.passnext = WFIDX(c1, c2, c3);
                    }
                    break;
            }
        }
        ++data; // '}'
    afterbrace:
        DEBUGLOG("    saving workflow %c%c%c --> %u (%c%c%c)\n", a, b, c ? c : ' ', WFIDX(a,b,c), WFCHARS(WFIDX(a,b,c)));
        workflows[WFIDX(a,b,c)] = wf;
        ++data; // '\n'
    }
    ++data;
    while (data < end)
    {
        uint16_t part[4] = {0};
        for (int i = 0; i < 4; ++i)
        {
            data += 3; // '{x=', ',m=', ',a=', ',s='
            while (isdigit(*data))
            {
                part[i] *= 10;
                part[i] += *data++ & 0xF;
            }
        }
        data += 2; // '}\n'

        uint16_t wfidx = WFIDX('i','n',0);
        DEBUGLOG("part {x=%u,m=%u,a=%u,s=%u}: in", part[0], part[1], part[2], part[3]);
        while (wfidx < 32760)
        {
            workflow* wf = workflows + wfidx;
            for (int c = 0; c < 4 && wf->conditions[c].next; ++c)
            {
                if (part[wf->conditions[c].idx] >= wf->conditions[c].min && part[wf->conditions[c].idx] <= wf->conditions[c].max)
                {
                    wfidx = wf->conditions[c].next;
                    goto nextwf;
                }
            }
            wfidx = wf->passnext;
        nextwf:
            DEBUGLOG(" --> %c%c%c", WFCHARS(wfidx));
        }
        DEBUGLOG("\n");

        if (wfidx == ACCEPT)
        {
            sum1 += part[0] + part[1] + part[2] + part[3];
        }
    }

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    uint16_t min[4] = {1, 1, 1, 1};
    uint16_t max[4] = {4000, 4000, 4000, 4000};
    sum2 += count_accepts(workflows, WFIDX('i','n',0), 0, min, max, 0);

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

