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

typedef struct mappingdata
{
    mapping mapping[1024];
    size_t size;
} mappingdata;

static const char* const logspaces[] = {"  ", "    ", "      ", "        ", "          ", "            ", "              "};

static uint64_t part2_lookup(const mappingdata* mappings, const int depth, const uint64_t curStart, const uint64_t curRange)
{
    const mappingdata* mv = &mappings[depth];

    DEBUGLOG("%s[->] part2_lookup depth=%d [%" PRIu64 " + %" PRIu64 "]\n", logspaces[depth], depth, curStart, curRange);

    const uint64_t curEnd = curStart + curRange;
    uint64_t cur = curStart;
    uint64_t value = UINT64_MAX;
    while (cur < curEnd)
    {
        // find next mapping
        const mapping* nextMapping = NULL;
        uint64_t nextMapStart = UINT64_MAX;
        for (int i = 0; i < mv->size; ++i)
        {
            const mapping* m = mv->mapping + i;
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

static uint32_t seeds[256];
static mappingdata maps[MAPPING_COUNT];

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    size_t seedCount = 0;

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
        seeds[seedCount++] = num;
    }

    mappingdata* current = NULL;
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
                    current = maps + ((file.data[lineIdx+1] == 'e') ? 0 : 1);
                    break;
                case 'f':
                    current = maps + 2;
                    break;
                case 'w':
                    current = maps + 3;
                    break;
                case 'l':
                    current = maps + 4;
                    break;
                case 't':
                    current = maps + 5;
                    break;
                case 'h':
                    current = maps + 6;
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

        current->mapping[current->size].srcStart = srcStart;
        current->mapping[current->size].dstStart = dstStart;
        current->mapping[current->size].range = range;
        ++current->size;

    }

    //
    // Part 1
    //

    uint64_t sum1 = UINT64_MAX;

    for (int seedIdx = 0; seedIdx < seedCount; ++seedIdx)
    {
        uint32_t seed = seeds[seedIdx];

        uint64_t next = seed;
        for (int m = 0; m < MAPPING_COUNT; ++m)
        {
            for (int i = 0; i < maps[m].size; ++i)
            {
                mapping map = maps[m].mapping[i];
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

    for (int seedIdx = 0; seedIdx < seedCount; seedIdx += 2)
    {
        uint32_t seedStart = seeds[seedIdx + 0];
        uint32_t seedRange = seeds[seedIdx + 1];
        DEBUGLOG("checking seed range [%u + %u]\n", seedStart, seedRange);

        uint64_t value = part2_lookup(maps, 0, seedStart, seedRange);
        sum2 = MIN(sum2, value);
        DEBUGLOG("checked seed range [%u + %u], value = %" PRIu64 ", sum2 = %" PRIu64 "\n", seedStart, seedRange, value, sum2);
    }

    print_uint64(sum2);

    return 0;
}

