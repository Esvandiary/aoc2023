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
#define dataindex(d, y, x) ((y)*(d).lineLength + (x))
#define dataY(d, idx) ((idx) / (d).lineLength)
#define dataX(d, idx) ((idx) % (d).lineLength)

struct astar_node;

typedef struct astar_edge
{
    struct astar_node* node;
    uint32_t cost;
} astar_edge;

typedef struct astar_node
{
    uint32_t idx;
    int8_t prevdir;
    bool traversed;
    astar_edge next[5];
} astar_node;

typedef struct astar_state
{
    astar_node* node;
    astar_node* visited[64];
    uint32_t visitedCount;
    int64_t gScore;
    int64_t fScore;
} astar_state;

#define MINHEAP_NAME astar
#define MINHEAP_TYPE astar_state
#define MINHEAP_SCORE(node) ((node)->fScore)
#include "../common/minheap.h"

typedef struct fdata
{
    chartype* const data;
    const size_t size;
    const int lineLength;
    const int lineCount;
    const uint32_t startIndex;
    const uint32_t finishIndex;
} fdata;

#define PD_NONE -1
#define PD_UP 0
#define PD_LEFT 1
#define PD_DOWN 2
#define PD_RIGHT 3

#define PD_OPPOSITE(x) (((x) + 2) & 3)

static const char* const pdnamesraw[] = {"none", "up", "left", "down", "right"};
static const char* const* pdnames = pdnamesraw + 1;
static const char* const ednamesraw[] = {"none", "below", "the right", "above", "the left"};
static const char* const* ednames = ednamesraw + 1;

static inline FORCEINLINE int32_t idx_in_dir(const fdata* d, int32_t idx, int32_t dir)
{
    switch (dir)
    {
        case PD_UP:    return idx - d->lineLength;
        case PD_LEFT:  return idx - 1;
        case PD_RIGHT: return idx + 1;
        case PD_DOWN:  return idx + d->lineLength;
        default:       return idx;
    }
}

static int findmaxlen1(fdata* d, uint32_t idx, int32_t prevdir, int count)
{
    if (idx == d->finishIndex)
        return count;

    int maxlen = -1;
    const chartype orig = d->data[idx];
    d->data[idx] = '#';
    if (idx >= d->lineLength && prevdir != PD_DOWN && (d->data[idx - d->lineLength] == '.' || d->data[idx - d->lineLength] == '^'))
    {
        int ulen = findmaxlen1(d, idx - d->lineLength, PD_UP, count + 1);
        maxlen = MAX(maxlen, ulen);
    }
    if (idx > 0 && prevdir != PD_RIGHT && (d->data[idx - 1] == '.' || d->data[idx - 1] == '<'))
    {
        int llen = findmaxlen1(d, idx - 1, PD_LEFT, count + 1);
        maxlen = MAX(maxlen, llen);
    }
    if (idx + 1 < d->size && prevdir != PD_LEFT && (d->data[idx + 1] == '.' || d->data[idx + 1] == '>'))
    {
        int rlen = findmaxlen1(d, idx + 1, PD_RIGHT, count + 1);
        maxlen = MAX(maxlen, rlen);
    }
    if (idx + d->lineLength < d->size && prevdir != PD_UP && (d->data[idx + d->lineLength] == '.' || d->data[idx + d->lineLength] == 'v'))
    {
        int dlen = findmaxlen1(d, idx + d->lineLength, PD_DOWN, count + 1);
        maxlen = MAX(maxlen, dlen);
    }
    d->data[idx] = orig;
    return maxlen;
}

static const bool isvalid2[256] = { ['.'] = true, ['^'] = true, ['v'] = true, ['<'] = true, ['>'] = true };

typedef struct nextnode
{
    int32_t idx;
    int16_t prevdir;
    uint16_t cost;
} nextnode;

static nextnode getnextnode(fdata* d, uint32_t idx, int32_t prevdir, uint16_t cost)
{
    if (idx == d->finishIndex)
        return (nextnode) { .idx = idx, .prevdir = prevdir, .cost = cost };

    int validcount = 0;
    int32_t lastvalid = PD_NONE;
    uint32_t lastnextidx;
    if (idx >= d->lineLength && prevdir != PD_OPPOSITE(PD_UP) && isvalid2[d->data[idx_in_dir(d, idx, PD_UP)]])
    {
        ++validcount;
        lastvalid = PD_UP;
        lastnextidx = idx - d->lineLength;
    }
    if (idx > 0 && prevdir != PD_OPPOSITE(PD_LEFT) && isvalid2[d->data[idx_in_dir(d, idx, PD_LEFT)]])
    {
        ++validcount;
        lastvalid = PD_LEFT;
        lastnextidx = idx - 1;
    }
    if (idx + 1 < d->size && prevdir != PD_OPPOSITE(PD_RIGHT) && isvalid2[d->data[idx_in_dir(d, idx, PD_RIGHT)]])
    {
        ++validcount;
        lastvalid = PD_RIGHT;
        lastnextidx = idx + 1;
    }
    if (idx + d->lineLength < d->size && prevdir != PD_OPPOSITE(PD_DOWN) && isvalid2[d->data[idx_in_dir(d, idx, PD_DOWN)]])
    {
        ++validcount;
        lastvalid = PD_DOWN;
        lastnextidx = idx + d->lineLength;
    }

    // DEBUGLOG("getnextnode((%d,%d) prevdir %s cost %d): %s\n", dataY(*d, idx), dataX(*d, idx), pdnames[prevdir], cost, (validcount == 0 ? "invalid" : validcount == 1 ? "continue" : "node"));

    switch (validcount)
    {
        case 0:
            return (nextnode) { .idx = -1 };
        case 1:
            return getnextnode(d, lastnextidx, lastvalid, cost + 1);
        default:
            return (nextnode) { .idx = idx, .prevdir = prevdir, .cost = cost };
    }
}

static void buildgraph(fdata* d, astar_node* nodes, astar_node* curnode)
{
    // DEBUGLOG("[%d,%d] checking node from %s\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), ednames[curnode->prevdir]);
    curnode->traversed = true;

    for (int dir = 0; dir < 4; ++dir)
    {
        int32_t dataidx = idx_in_dir(d, curnode->idx, dir);
        // DEBUGLOG("[%d,%d]    checking %s at (%d,%d)\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, dataidx), dataX(*d, dataidx));
        if (curnode->prevdir != PD_OPPOSITE(dir) && dataidx >= 0 && dataidx < d->size && isvalid2[d->data[dataidx]])
        {
            nextnode diridx = getnextnode(d, dataidx, dir, 0);
            if (diridx.idx >= 0)
            {
                astar_node* dirnodebase = nodes + (diridx.idx << 2);
                if (dirnodebase[0].traversed || dirnodebase[1].traversed || dirnodebase[2].traversed || dirnodebase[3].traversed)
                    continue;
                astar_node* dirnode = dirnodebase + diridx.prevdir;
                // DEBUGLOG("[%d,%d]    next in dir %s: [%d,%d from %s] with cost %u\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, diridx.idx), dataX(*d, diridx.idx), ednames[diridx.prevdir], diridx.cost);
                dirnode->idx = diridx.idx;
                dirnode->prevdir = diridx.prevdir;

                curnode->next[dir].node = dirnode;
                curnode->next[dir].cost = diridx.cost;

                buildgraph(d, nodes, dirnode);
            }
        }
    }

    curnode->traversed = false;
}

static void printgraph(const fdata* d, astar_node* node, int depth)
{
    node->traversed = true;
    for (int i = 0; i < depth; ++i)
        DEBUGLOG("  ");
    DEBUGLOG("(%d,%d) from %s\n", dataY(*d, node->idx), dataX(*d, node->idx), ednames[node->prevdir]);
    for (int i = 0; i < 4; ++i)
    {
        if (node->next[i].node)
        {
            astar_node* dirnodebase = node->next[i].node - node->next[i].node->prevdir;
            if (dirnodebase[0].traversed || dirnodebase[1].traversed || dirnodebase[2].traversed || dirnodebase[3].traversed)
                continue;
            printgraph(d, node->next[i].node, depth + 1);
        }
    }
    node->traversed = false;
}

static int64_t findmaxlen2slowly(fdata* d, astar_node* node, int64_t cost)
{
    if (node->idx == d->finishIndex)
        return cost;
    
    node->traversed = true;
    int64_t newcost = -1;
    for (int i = 0; i < 4; ++i)
    {
        if (!node->next[i].node)
            continue;
        astar_node* dirnodebase = node->next[i].node - node->next[i].node->prevdir;
        if (dirnodebase[0].traversed || dirnodebase[1].traversed || dirnodebase[2].traversed || dirnodebase[3].traversed)
            continue;
        int64_t ncost = 1 + findmaxlen2slowly(d, node->next[i].node, cost + node->next[i].cost);
        newcost = MAX(newcost, ncost);
    }
    node->traversed = false;
    return newcost;
}

static int64_t findmaxlen2(fdata* d)
{
    astar_node* nodes = (astar_node*)calloc(32768*4, sizeof(astar_node));

    astar_node curnode = {
        .idx = d->startIndex,
        .prevdir = PD_NONE,
    };

    DEBUGLOG("building graph\n");
    buildgraph(d, nodes, &curnode);
    printgraph(d, &curnode, 0);
    DEBUGLOG("built graph\n");

    return findmaxlen2slowly(d, &curnode, 0);
    // return calculate(d, &curnode, nodes + ((d->finishIndex << 2) | PD_DOWN));
}

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    while (*data != '.')
        ++data;
    uint32_t sidx = (data - file.data);
    while (*data++ != '\n');
    const int lineLength = data - file.data;
    const int lineCount = (fileSize + lineLength / 2) / lineLength;

    DEBUGLOG("lineLength = %d, lineCount = %d\n", lineLength, lineCount);

    data = file.data + (lineCount - 1) * lineLength;
    uint32_t fidx;
    while (data < end)
    {
        if (*data == '.')
        {
            fidx = (data - file.data);
            break;
        }
        ++data;
    }

    fdata d = {
        .data = file.data,
        .size = fileSize,
        .lineLength = lineLength,
        .lineCount = lineCount,
        .startIndex = sidx,
        .finishIndex = fidx
    };

    DEBUGLOG("sidx = (%d,%d), fidx = (%d,%d)\n", dataY(d, sidx), dataX(d, sidx), dataY(d, fidx), dataX(d, fidx));

    int64_t sum1 = 0, sum2 = 0;

    DSTOPWATCH_START(part1);

    sum1 = findmaxlen1(&d, d.startIndex, PD_NONE, 0);

    print_int64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    sum2 = findmaxlen2(&d);

    print_int64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

