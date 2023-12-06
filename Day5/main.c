#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/vuctor.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define MAPPING_COUNT 7

typedef struct mapping
{
    uint64_t srcStart;
    uint64_t dstStart;
    uint64_t range;
} mapping;

static const char* const logspaces[] = {"  ", "    ", "      ", "        ", "          ", "            ", "              "};

static uint64_t part2_lookup(const vuctor* mappings, const int depth, const uint64_t curStart, const uint64_t curRange)
{
    const vuctor* mv = &mappings[depth];

    DEBUGLOG("%s[->] part2_lookup depth=%d [%" PRIu64 " + %" PRIu64 "]\n", logspaces[depth], depth, curStart, curRange);

    const uint64_t curEnd = curStart + curRange;
    uint64_t cur = curStart;
    uint64_t value = UINT64_MAX;
    while (cur < curEnd)
    {
        // find next mapping
        mapping* nextMapping = NULL;
        uint64_t nextMapStart = UINT64_MAX;
        for (int i = 0; i < mv->size; ++i)
        {
            mapping* m = VUCTOR_GET_PTR(*mv, mapping, i);
            if (cur < m->srcStart + m->range && m->srcStart < curEnd && m->srcStart < nextMapStart)
            {
                nextMapping = m;
                nextMapStart = m->srcStart;
            }
        }
        DEBUGLOG("%scur = %" PRIu64 ", nextMapStart = %" PRIu64 ", value = %" PRIu64 "\n", logspaces[depth], cur, nextMapStart, value);
        if (cur < nextMapStart)
        {
            // gap with no mapping
            const uint64_t range = MIN(curEnd, nextMapStart) - cur;
            DEBUGLOG("%s[->] gap, range [%" PRIu64 " + %" PRIu64 "]\n", logspaces[depth], cur, range);
            const uint64_t newValue = (depth == MAPPING_COUNT - 1) ? cur : part2_lookup(mappings, depth + 1, cur, range);
            value = MIN(value, newValue);
            DEBUGLOG("%s[<-] gap, range [%" PRIu64 " + %" PRIu64 "], newValue = %" PRIu64 ", value = %" PRIu64 "\n", logspaces[depth], cur, range, newValue, value);
            cur += range;
        }
        if (nextMapping)
        {
            // next mapping
            mapping m = *nextMapping;
            const uint64_t dstStart = m.dstStart + (cur > m.srcStart ? cur - m.srcStart : 0);
            const uint64_t range = MIN(m.range - (dstStart - m.dstStart), curEnd - cur);
            DEBUGLOG("%s[->] nextmap, range [%" PRIu64 " + %" PRIu64 "]\n", logspaces[depth], dstStart, range);
            const uint64_t newValue = (depth == MAPPING_COUNT - 1)? dstStart : part2_lookup(mappings, depth + 1, dstStart, range);
            value = MIN(value, newValue);
            DEBUGLOG("%s[<-] nextmap, range [%" PRIu64 " + %" PRIu64 "], newValue = %" PRIu64 ", value = %" PRIu64 "\n", logspaces[depth], dstStart, range, newValue, value);
            cur += range;
        }
    }
    DEBUGLOG("%s[<-] part2_lookup depth=%d [%" PRIu64 " + %" PRIu64 "] value = %" PRIu64 "\n", logspaces[depth], depth, curStart, curRange, value);
    return value;
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    vuctor seeds = VUCTOR_INIT;
    VUCTOR_RESERVE(seeds, uint32_t, 128);

    vuctor mappings[MAPPING_COUNT];
    memset(mappings, 0, sizeof(mappings));
    for (int i = 0; i < MAPPING_COUNT; ++i)
        VUCTOR_RESERVE(mappings[i], mapping, 128);

    int lineIdx = 7; // 'seeds: '

    while (lineIdx < file.size && file.data[lineIdx] != '\n')
    {
        uint32_t num = 0;
        while (isdigit(file.data[lineIdx]))
        {
            num *= 10;
            num += file.data[lineIdx] & 0xF;
            ++lineIdx;
        }
        ++lineIdx; // space
        VUCTOR_ADD(seeds, uint32_t, num);
    }

    vuctor* current = NULL;
    while (lineIdx < fileSize)
    {
        if (!isdigit(file.data[lineIdx]))
        {
            switch (file.data[lineIdx])
            {
                case '\n':
                    ++lineIdx;
                    continue;
                case 's':
                    current = (file.data[lineIdx+1] == 'e') ? &mappings[0] : &mappings[1];
                    break;
                case 'f':
                    current = &mappings[2];
                    break;
                case 'w':
                    current = &mappings[3];
                    break;
                case 'l':
                    current = &mappings[4];
                    break;
                case 't':
                    current = &mappings[5];
                    break;
                case 'h':
                    current = &mappings[6];
                    break;
                default:
                    printf("got unexpected line\n");
                    return 1;
            }
            // next line
            while (lineIdx < file.size && file.data[lineIdx] != '\n')
                ++lineIdx;
            ++lineIdx;
            continue;
        }

        // source start
        uint32_t srcStart = 0, dstStart = 0, range = 0;
        while (isdigit(file.data[lineIdx]))
        {
            dstStart *= 10;
            dstStart += file.data[lineIdx] & 0xF;
            ++lineIdx;
        }
        ++lineIdx;
        while (isdigit(file.data[lineIdx]))
        {
            srcStart *= 10;
            srcStart += file.data[lineIdx] & 0xF;
            ++lineIdx;
        }
        ++lineIdx;
        while (isdigit(file.data[lineIdx]))
        {
            range *= 10;
            range += file.data[lineIdx] & 0xF;
            ++lineIdx;
        }
        ++lineIdx;

        mapping* map = VUCTOR_ADD_NOINIT(*current, mapping);
        map->srcStart = srcStart;
        map->dstStart = dstStart;
        map->range = range;
    }

    //
    // Part 1
    //

    uint64_t sum1 = UINT64_MAX;

    for (int seedIdx = 0; seedIdx < seeds.size; ++seedIdx)
    {
        uint32_t seed = VUCTOR_GET(seeds, uint32_t, seedIdx);

        uint64_t next = seed;
        for (int m = 0; m < MAPPING_COUNT; ++m)
        {
            for (int i = 0; i < mappings[m].size; ++i)
            {
                mapping map = VUCTOR_GET(mappings[m], mapping, i);
                uint64_t diff = next - map.srcStart; // abuse negative overflow
                if (diff < map.range)
                {
                    next = map.dstStart + diff;
                    break;
                }
            }
        }

        sum1 = MIN(sum1, next);
    }

    print_uint64(sum1);
 
    //
    // Part 2
    //

    uint64_t sum2 = UINT64_MAX;

    for (int seedIdx = 0; seedIdx < seeds.size; seedIdx += 2)
    {
        uint32_t seedStart = VUCTOR_GET(seeds, uint32_t, seedIdx);
        uint32_t seedRange = VUCTOR_GET(seeds, uint32_t, seedIdx + 1);
        DEBUGLOG("checking seed range [%u + %u]\n", seedStart, seedRange);

        uint64_t value = part2_lookup(mappings, 0, seedStart, seedRange);
        sum2 = MIN(sum2, value);
        DEBUGLOG("checked seed range [%u + %u], value = %" PRIu64 ", sum2 = %" PRIu64 "\n", seedStart, seedRange, value, sum2);
    }

    print_uint64(sum2);

    return 0;
}

