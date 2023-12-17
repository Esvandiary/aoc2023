#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"
#include "../common/vuctor.h"
#include "../common/uthash/uthash.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y,x) ((y) * lineLength + (x))
#define dataY(idx) ((idx) / lineLength)
#define dataX(idx) ((idx) % lineLength)

#define ENTRYKEYSZ (sizeof(entry))
#define NEXTNUM(e) ((e).nums[sizeof((e).nums) - (e).numsCount])
#pragma pack(push, 8)
typedef struct entry
{
    __uint128_t broken;
    __uint128_t unknown;
    uint8_t length;
    uint8_t numsCount;
    uint8_t curlen;
    uint8_t nums[37];
} entry;
#pragma pop

typedef struct hentry
{
    entry entry;
    uint64_t result;
    UT_hash_handle hh;
    uint64_t _padding0;
} hentry;

static uint64_t getcount(vuctor* cachestore, hentry** cache, entry curentry)
{
    hentry* foundentry = NULL;
    HASH_FIND(hh, *cache, &curentry, ENTRYKEYSZ, foundentry);
    if (foundentry != NULL)
    {
        DEBUGLOG("using cached value at length %u, result %lu\n", foundentry->entry.length, foundentry->result);
        return foundentry->result;
    }

    while (curentry.length)
    {
        if (curentry.broken & 1)
        {
            DEBUGLOG("[%u] #\n", curentry.length);
            if (curentry.numsCount == 0)
            {
                DEBUGLOG("done too many nums\n");
                return 0;
            }
            if (++curentry.curlen > NEXTNUM(curentry))
            {
                DEBUGLOG("curlen %d > nextnum %u\n", curentry.curlen, NEXTNUM(curentry));
                return 0;
            }
        }
        else if (curentry.unknown & 1)
        {
            DEBUGLOG("[%u] ?\n", curentry.length);
            entry nxtentry = curentry;
            nxtentry.unknown -= 1;
            // DEBUGLOG("branching ? -> . at %zu, curlen %d, numsDone %d\n", cs - cstart, curlen, numsDone);
            uint64_t resultDot = getcount(cachestore, cache, nxtentry);
            hentry* dotcache = VUCTOR_ADD_NOINIT(*cachestore, hentry);
            dotcache->entry = nxtentry;
            dotcache->result = resultDot;
            HASH_ADD_KEYPTR(hh, *cache, &dotcache->entry, ENTRYKEYSZ, dotcache);

            // DEBUGLOG("branching ? -> # at %zu, curlen %d, numsDone %d\n", cs - cstart, curlen, numsDone);
            nxtentry.broken |= 1;
            uint64_t resultHash = getcount(cachestore, cache, nxtentry);
            hentry* hashcache = VUCTOR_ADD_NOINIT(*cachestore, hentry);
            hashcache->entry = nxtentry;
            hashcache->result = resultHash;
            HASH_ADD_KEYPTR(hh, *cache, &hashcache->entry, ENTRYKEYSZ, hashcache);

            // DEBUGLOG("done branching on ? at %zu, result now %d\n", cs - cstart, result);
            DEBUGLOG("resultDot = %lu, resultHash = %lu\n", resultDot, resultHash);

            return resultDot + resultHash;
        }
        else
        {
            DEBUGLOG("[%u] .\n", curentry.length);
            if (curentry.curlen > 0)
            {
                if (curentry.curlen != NEXTNUM(curentry))
                {
                    DEBUGLOG("num fail: curlen %d != %d\n", curentry.curlen, NEXTNUM(curentry));
                    return 0;
                }
                // DEBUGLOG("done num %d, now done %d/%zu this pass\n", curlen, numsDone + 1, numsCount);
                NEXTNUM(curentry) = 0;
                --curentry.numsCount;
                DEBUGLOG("numsCount now %u, nextnum now %u\n", curentry.numsCount, NEXTNUM(curentry));
                curentry.curlen = 0;
            }
        }
    next:
        curentry.broken >>= 1;
        curentry.unknown >>= 1;
        --curentry.length;
    }
    if (curentry.curlen > 0)
    {
        if (curentry.curlen != NEXTNUM(curentry))
        {
            DEBUGLOG("num fail: curlen %d != %d\n", curentry.curlen, NEXTNUM(curentry));
            return 0;
        }
        // DEBUGLOG("done num %d, now done %d/%zu this pass\n", curlen, numsDone + 1, numsCount);
        NEXTNUM(curentry) = 0;
        --curentry.numsCount;
        curentry.curlen = 0;
    }

    DEBUGLOG("end: numsCount %u\n", curentry.numsCount);
    // for (const chartype* c = cstart; c < end; ++c)
    //     DEBUGLOG("%c", *c);
    // DEBUGLOG(", numsCount %zu vs %d, returning %d\n", numsCount, numsDone, (numsCount == numsDone) ? 1 : 0);
    return (curentry.numsCount == 0) ? 1 : 0;
}

typedef struct segment
{
    const chartype* start;
    const chartype* end;
    pthread_t thread;
    uint64_t sum1;
    uint64_t sum2;
} segment;

static segment segments[64] = {0};

static void* run_thread(void* vseg)
{
    segment* seg = (segment*)vseg;
    const chartype* data = seg->start;
    const chartype* const end = seg->end;

    vuctor cachestore = VUCTOR_INIT;
    VUCTOR_RESERVE(cachestore, hentry, 2097152);

    hentry* cache = NULL;

    uint64_t sum1 = 0, sum2 = 0;

    while (data < end)
    {
        uint8_t nums[16];
        size_t numCount = 0;

        entry curentry = {0};
        while (*data != ' ')
        {
            const uint8_t hex30 = ((*data >> 4) & 1);
            curentry.broken |= ((*data & ~hex30) & 1) << curentry.length;
            curentry.unknown |= hex30 << curentry.length;
            ++curentry.length;
            ++data;
        }

        ++data;
        do
        {
            int num = 0;
            while (isdigit(*data))
            {
                num *= 10;
                num += *data++ & 0xF;
            }
            nums[numCount++] = num;
        } while (*data++ != '\n');
        memcpy(curentry.nums + sizeof(curentry.nums) - numCount, nums, sizeof(uint8_t) * numCount);
        curentry.numsCount = numCount;

        DEBUGLOG("part 1\n");

        uint64_t result = getcount(&cachestore, &cache, curentry);
        sum1 += result;

        entry p2entry = curentry;

        hentry* h1 = VUCTOR_ADD_NOINIT(cachestore, hentry);
        h1->entry = curentry;
        h1->result = result;
        HASH_ADD_KEYPTR(hh, cache, &h1->entry, ENTRYKEYSZ, h1);

        DEBUGLOG("part 2\n");

        for (int i = 1; i < 5; ++i)
        {
            p2entry.broken = (p2entry.broken << 1);
            p2entry.broken = (p2entry.broken << curentry.length) | curentry.broken;
            p2entry.unknown = (p2entry.unknown << 1) | 1;
            p2entry.unknown = (p2entry.unknown << curentry.length) | curentry.unknown;
        }
        p2entry.length = (curentry.length * 5) + 4;

        for (int i = 1; i < 5; ++i)
        {
            memcpy(p2entry.nums + sizeof(curentry.nums) - (numCount*(i+1)), &(NEXTNUM(curentry)), sizeof(uint8_t) * numCount);
        }
        p2entry.numsCount *= 5;

        uint64_t p2result = getcount(&cachestore, &cache, p2entry);
        sum2 += p2result;

        hentry* h2 = VUCTOR_ADD_NOINIT(cachestore, hentry);
        h2->entry = p2entry;
        h2->result = p2result;
        HASH_ADD_KEYPTR(hh, cache, &h2->entry, ENTRYKEYSZ, h2);

        DEBUGLOG("done line, sum2 now %ld; length %u\n", sum2, curentry.length);
    }

    seg->sum1 = sum1;
    seg->sum2 = sum2;
    return NULL;
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    int numSegments = get_nprocs();
    for (int i = 0; i + 1 < numSegments; ++i)
    {
        segments[i].start = data;
        data += fileSize / numSegments;
        while (*(data - 1) != '\n')
            --data;
        segments[i].end = data;
    }
    segments[numSegments - 1].start = data;
    segments[numSegments - 1].end = end;

    for (int i = 0; i < numSegments; ++i)
    {
        pthread_create(&segments[i].thread, NULL, run_thread, segments + i);
    }
    for (int i = 0; i < numSegments; ++i)
    {
        pthread_join(segments[i].thread, NULL);
    }

    DSTOPWATCH_END(logic);

    for (int i = 0; i < numSegments; ++i)
    {
        sum1 += segments[i].sum1;
        sum2 += segments[i].sum2;
    }

    print_int64(sum1);
    print_int64(sum2);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

