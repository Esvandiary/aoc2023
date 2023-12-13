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
#include "../common/vuctor.h"
#include "../common/uthash/uthash.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define dataindex(y,x) ((y) * lineLength + (x))
#define dataY(idx) ((idx) / lineLength)
#define dataX(idx) ((idx) % lineLength)

#define ENTRYKEYSZ (offsetof(entry, result))
#define NEXTNUM(e) ((e).nums[sizeof((e).nums) - (e).numsCount])
typedef struct entry
{
    __uint128_t broken;
    __uint128_t unknown;
    uint32_t length;
    uint8_t nums[62];
    uint8_t numsCount;
    uint8_t curlen;
    uint64_t result;
    UT_hash_handle hh;
} entry;

static uint64_t getcount(vuctor* cachestore, entry** cache, entry curentry)
{
    entry* foundentry = NULL;
    HASH_FIND(hh, *cache, &curentry, offsetof(entry, result), foundentry);
    if (foundentry != NULL)
    {
        DEBUGLOG("using cached value at length %u, result %lu\n", foundentry->length, foundentry->result);
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
            entry* dotcache = VUCTOR_ADD_NOINIT(*cachestore, entry);
            *dotcache = nxtentry;
            dotcache->result = resultDot;
            HASH_ADD_KEYPTR(hh, *cache, dotcache, ENTRYKEYSZ, dotcache);

            // DEBUGLOG("branching ? -> # at %zu, curlen %d, numsDone %d\n", cs - cstart, curlen, numsDone);
            nxtentry.broken |= 1;
            uint64_t resultHash = getcount(cachestore, cache, nxtentry);
            entry* hashcache = VUCTOR_ADD_NOINIT(*cachestore, entry);
            *hashcache = nxtentry;
            hashcache->result = resultHash;
            HASH_ADD_KEYPTR(hh, *cache, hashcache, ENTRYKEYSZ, hashcache);

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

int main(int argc, char** argv)
{
    DSTOPWATCH_START(logic);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    vuctor cachestore = VUCTOR_INIT;
    VUCTOR_RESERVE(cachestore, entry, 4194304);

    entry* cache = NULL;

    while (data < end)
    {
        uint8_t nums[16];
        size_t numCount = 0;

        chartype* cstart = data;
        entry* curentry = VUCTOR_ADD_NOINIT(cachestore, entry);
        while (*data != ' ')
        {
            const uint8_t hex30 = ((*data >> 4) & 1);
            curentry->broken |= ((*data & ~hex30) & 1) << curentry->length;
            curentry->unknown |= hex30 << curentry->length;
            ++curentry->length;
            ++data;
        }

        const chartype* cend = data - 1;
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
        memcpy(curentry->nums + sizeof(curentry->nums) - numCount, nums, sizeof(uint8_t) * numCount);
        curentry->numsCount = numCount;

        DEBUGLOG("part 1\n");

        uint64_t result = getcount(&cachestore, &cache, *curentry);
        sum1 += result;

        entry* p2entry = VUCTOR_ADD_NOINIT(cachestore, entry);
        *p2entry = *curentry;

        curentry->result = result;
        HASH_ADD_KEYPTR(hh, cache, curentry, ENTRYKEYSZ, curentry);

        DEBUGLOG("part 2\n");

        for (int i = 1; i < 5; ++i)
        {
            p2entry->broken = (p2entry->broken << 1);
            p2entry->broken = (p2entry->broken << curentry->length) | curentry->broken;
            p2entry->unknown = (p2entry->unknown << 1) | 1;
            p2entry->unknown = (p2entry->unknown << curentry->length) | curentry->unknown;
        }
        p2entry->length = (curentry->length * 5) + 4;

        for (int i = 1; i < 5; ++i)
        {
            memcpy(p2entry->nums + sizeof(curentry->nums) - (numCount*(i+1)), &(NEXTNUM(*curentry)), sizeof(uint8_t) * numCount);
        }
        p2entry->numsCount *= 5;

        uint64_t p2result = getcount(&cachestore, &cache, *p2entry);
        sum2 += p2result;

        p2entry->result = p2result;
        HASH_ADD_KEYPTR(hh, cache, p2entry, ENTRYKEYSZ, p2entry);

        DEBUGLOG("done line, sum2 now %ld; length %u\n", sum2, curentry->length);
    }

    DEBUGLOG("vuctor size: %zu\n", cachestore.size);

    DSTOPWATCH_END(logic);

    print_int64(sum1);
    print_int64(sum2);

    DSTOPWATCH_PRINT(logic);

    return 0;
}

