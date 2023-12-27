#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#if defined(__linux__)
#include <sys/sysinfo.h>
#endif
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
    astar_edge next[4];
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

static inline FORCEINLINE int32_t idx_in_dir(const fdata* const d, const int32_t idx, const int32_t dir)
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

typedef struct slowstate
{
    bool traversed[32768];
    nextnode nextnodes[32768][4];
} slowstate;

static nextnode getnextnode(const fdata* const d, uint32_t idx, int32_t prevdir, uint16_t cost)
{
    int validcount = 0;
    int32_t lastvalid = prevdir;
    uint32_t lastnextidx = idx;
    while (true)
    {
        validcount = 0;
        if (idx >= d->lineLength && prevdir != PD_OPPOSITE(PD_UP) && isvalid2[d->data[idx_in_dir(d, idx, PD_UP)]])
        {
            ++validcount;
            lastvalid = PD_UP;
            lastnextidx -= d->lineLength;
        }
        if (idx > 0 && prevdir != PD_OPPOSITE(PD_LEFT) && isvalid2[d->data[idx_in_dir(d, idx, PD_LEFT)]])
        {
            ++validcount;
            lastvalid = PD_LEFT;
            lastnextidx -= 1;
        }
        if (idx + 1 < d->size && prevdir != PD_OPPOSITE(PD_RIGHT) && isvalid2[d->data[idx_in_dir(d, idx, PD_RIGHT)]])
        {
            ++validcount;
            lastvalid = PD_RIGHT;
            lastnextidx += 1;
        }
        if (idx + d->lineLength < d->size && prevdir != PD_OPPOSITE(PD_DOWN) && isvalid2[d->data[idx_in_dir(d, idx, PD_DOWN)]])
        {
            ++validcount;
            lastvalid = PD_DOWN;
            lastnextidx += d->lineLength;
        }
        if (validcount != 1)
            break;
        idx = lastnextidx;
        prevdir = lastvalid;
        ++cost;
    }
    // DEBUGLOG("getnextnode((%d,%d) prevdir %s cost %d): %s\n", dataY(*d, idx), dataX(*d, idx), pdnames[prevdir], cost, (validcount == 0 ? "invalid" : validcount == 1 ? "continue" : "node"));

    switch (validcount)
    {
        case 0:
            if (idx != d->finishIndex)
                return (nextnode) { .idx = -1 };
        default:
            return (nextnode) { .idx = idx, .prevdir = prevdir, .cost = cost };
    }
}

static void buildgraph(const fdata* const d, slowstate* const state, astar_node* const nodes, astar_node* const curnode)
{
    // DEBUGLOG("[%d,%d] checking node from %s\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), ednames[curnode->prevdir]);
    state->traversed[curnode->idx] = true;

    for (int dir = 0; dir < 4; ++dir)
    {
        const int32_t dataidx = idx_in_dir(d, curnode->idx, dir);
        // DEBUGLOG("[%d,%d]    checking %s at (%d,%d)\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, dataidx), dataX(*d, dataidx));
        if (curnode->prevdir != PD_OPPOSITE(dir) && dataidx >= 0 && dataidx < d->size && isvalid2[d->data[dataidx]])
        {
            if (!state->nextnodes[dataidx][dir].idx)
                state->nextnodes[dataidx][dir] = getnextnode(d, dataidx, dir, 0);
            const nextnode diridx = state->nextnodes[dataidx][dir];
            if (diridx.idx >= 0)
            {
                if (state->traversed[diridx.idx])
                    continue;
                astar_node* const dirnode = nodes + (diridx.idx << 2) + diridx.prevdir;
                // DEBUGLOG("[%d,%d]    next in dir %s: [%d,%d from %s] with cost %u\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, diridx.idx), dataX(*d, diridx.idx), ednames[diridx.prevdir], diridx.cost);
                dirnode->idx = diridx.idx;
                dirnode->prevdir = diridx.prevdir;

                curnode->next[dir].node = dirnode;
                curnode->next[dir].cost = diridx.cost;

                buildgraph(d, state, nodes, dirnode);
            }
        }
    }

    state->traversed[curnode->idx] = false;
}

typedef struct bpworkstate
{
    astar_node* node;
    int64_t cost;
    bool* traversed;
} bpworkstate;

struct bpsetupstate;

typedef struct bpsegment
{
    pthread_t thread;
    const fdata* d;
    astar_node* nodes;
    struct bpsetupstate* state;
    uint32_t firstwork;
    nextnode nextnodes[32768][4];
} bpsegment;

typedef struct bpsetupstate
{
    slowstate* state;
    bpsegment* segments;
    uint32_t segmentCount;
    bpworkstate workstates[512];
    uint32_t workstatesCount;
    uint32_t nextworkstate;
} bpsetupstate;

static void buildgraphpar_run(const fdata* const d, bool* const traversed, nextnode (* const nextnodes)[4], astar_node* const nodes, astar_node* const curnode)
{
    // DEBUGLOG("[%d,%d] checking node from %s\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), ednames[curnode->prevdir]);
    traversed[curnode->idx] = true;

    for (int dir = 0; dir < 4; ++dir)
    {
        const int32_t dataidx = idx_in_dir(d, curnode->idx, dir);
        // DEBUGLOG("[%d,%d]    checking %s at (%d,%d)\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, dataidx), dataX(*d, dataidx));
        if (curnode->prevdir != PD_OPPOSITE(dir) && dataidx >= 0 && dataidx < d->size && isvalid2[d->data[dataidx]])
        {
            if (!nextnodes[dataidx][dir].idx)
                nextnodes[dataidx][dir] = getnextnode(d, dataidx, dir, 0);
            const nextnode diridx = nextnodes[dataidx][dir];
            if (diridx.idx >= 0)
            {
                if (traversed[diridx.idx])
                    continue;
                astar_node* const dirnode = nodes + (diridx.idx << 2) + diridx.prevdir;
                // DEBUGLOG("[%d,%d]    next in dir %s: [%d,%d from %s] with cost %u\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, diridx.idx), dataX(*d, diridx.idx), ednames[diridx.prevdir], diridx.cost);
                dirnode->idx = diridx.idx;
                dirnode->prevdir = diridx.prevdir;

                curnode->next[dir].node = dirnode;
                curnode->next[dir].cost = diridx.cost;

                buildgraphpar_run(d, traversed, nextnodes, nodes, dirnode);
            }
        }
    }

    traversed[curnode->idx] = false;
}

static void* buildgraphpar_thread(void* vseg)
{
    bpsegment* seg = (bpsegment*)vseg;

    int workidx = seg->firstwork;
    do
    {
        buildgraphpar_run(
            seg->d,
            seg->state->workstates[workidx].traversed,
            seg->nextnodes,
            seg->nodes,
            seg->state->workstates[workidx].node);
    } while ((workidx = __atomic_fetch_add(&seg->state->nextworkstate, 1, __ATOMIC_ACQ_REL)) < seg->state->workstatesCount);
    return NULL;
}

static void buildgraphpar_setup(const fdata* const d, bpsetupstate* const state, astar_node* nodes, astar_node* const curnode, int depth, int targetdepth)
{
    state->state->traversed[curnode->idx] = true;
    int64_t newcost = -1;
    if (depth == targetdepth)
    {
        for (int dir = 0; dir < 4; ++dir)
        {
            const int32_t dataidx = idx_in_dir(d, curnode->idx, dir);
            // DEBUGLOG("[%d,%d]    checking %s at (%d,%d)\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, dataidx), dataX(*d, dataidx));
            if (curnode->prevdir != PD_OPPOSITE(dir) && dataidx >= 0 && dataidx < d->size && isvalid2[d->data[dataidx]])
            {
                if (!state->state->nextnodes[dataidx][dir].idx)
                    state->state->nextnodes[dataidx][dir] = getnextnode(d, dataidx, dir, 0);
                const nextnode diridx = state->state->nextnodes[dataidx][dir];
                if (diridx.idx >= 0)
                {
                    if (state->state->traversed[diridx.idx])
                        continue;
                    astar_node* const dirnode = nodes + (diridx.idx << 2) + diridx.prevdir;
                    // DEBUGLOG("[%d,%d]    next in dir %s: [%d,%d from %s] with cost %u\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, diridx.idx), dataX(*d, diridx.idx), ednames[diridx.prevdir], diridx.cost);
                    dirnode->idx = diridx.idx;
                    dirnode->prevdir = diridx.prevdir;

                    curnode->next[dir].node = dirnode;
                    curnode->next[dir].cost = diridx.cost;

                    int wsidx = state->workstatesCount;
                    state->workstates[wsidx].node = curnode->next[dir].node;
                    state->workstates[wsidx].traversed = (bool*)malloc(sizeof(bool)*32768);
                    memcpy(state->workstates[wsidx].traversed, &state->state->traversed, sizeof(state->state->traversed));

                    ++state->workstatesCount;
                }
            }
        }
    }
    else
    {
        for (int dir = 0; dir < 4; ++dir)
        {
            const int32_t dataidx = idx_in_dir(d, curnode->idx, dir);
            // DEBUGLOG("[%d,%d]    checking %s at (%d,%d)\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, dataidx), dataX(*d, dataidx));
            if (curnode->prevdir != PD_OPPOSITE(dir) && dataidx >= 0 && dataidx < d->size && isvalid2[d->data[dataidx]])
            {
                if (!state->state->nextnodes[dataidx][dir].idx)
                    state->state->nextnodes[dataidx][dir] = getnextnode(d, dataidx, dir, 0);
                const nextnode diridx = state->state->nextnodes[dataidx][dir];
                if (diridx.idx >= 0)
                {
                    if (state->state->traversed[diridx.idx])
                        continue;
                    astar_node* const dirnode = nodes + (diridx.idx << 2) + diridx.prevdir;
                    // DEBUGLOG("[%d,%d]    next in dir %s: [%d,%d from %s] with cost %u\n", dataY(*d, curnode->idx), dataX(*d, curnode->idx), pdnames[dir], dataY(*d, diridx.idx), dataX(*d, diridx.idx), ednames[diridx.prevdir], diridx.cost);
                    dirnode->idx = diridx.idx;
                    dirnode->prevdir = diridx.prevdir;

                    curnode->next[dir].node = dirnode;
                    curnode->next[dir].cost = diridx.cost;

                    buildgraphpar_setup(d, state, nodes, dirnode, depth + 1, targetdepth);
                }
            }
        }
    }
    state->state->traversed[curnode->idx] = false;
}

static void buildgraphpar(const fdata* const d, slowstate* const state, astar_node* const nodes, astar_node* const node)
{
#if defined(__linux__)
    int nthreads = get_nprocs();
#elif defined(_WIN32)
    int nthreads = atoi(getenv("NUMBER_OF_PROCESSORS"));
#else
    return findmaxlen2slowly(d, state, node, cost);
#endif
    DEBUGLOG("bg nthreads %d\n", nthreads);

    int targetdepth = 6;

    bpsetupstate pss = { .state = state, .segments = (bpsegment*)calloc(32, sizeof(bpsegment)) };
    buildgraphpar_setup(d, &pss, nodes, node, 0, targetdepth);

    DEBUGLOG("bg work states %u\n", pss.workstatesCount);

    nthreads = MIN(nthreads, 32);

    int64_t newcost = 0;
    pss.nextworkstate = nthreads;
    for (int i = 0; i < nthreads; ++i)
    {
        pss.segments[i].d = d;
        pss.segments[i].firstwork = i;
        pss.segments[i].nodes = nodes;
        pss.segments[i].state = &pss;
        memcpy(pss.segments[i].nextnodes, pss.state->nextnodes, sizeof(pss.state->nextnodes));

        pthread_create(&pss.segments[i].thread, NULL, buildgraphpar_thread, pss.segments + i);        
    }
    for (int i = 0; i < nthreads; ++i)
    {
        pthread_join(pss.segments[i].thread, NULL);
    }
}

static void printgraph(const fdata* d, slowstate* state, astar_node* node, int depth)
{
    state->traversed[node->idx] = true;
    for (int i = 0; i < depth; ++i)
        DEBUGLOG("  ");
    DEBUGLOG("(%d,%d) from %s\n", dataY(*d, node->idx), dataX(*d, node->idx), ednames[node->prevdir]);
    for (int i = 0; i < 4; ++i)
    {
        if (node->next[i].node)
        {
            if (state->traversed[node->next[i].node->idx])
                continue;
            printgraph(d, state, node->next[i].node, depth + 1);
        }
    }
    state->traversed[node->idx] = false;
}

static int64_t findmaxlen2slowly(const fdata* const d, slowstate* const state, const astar_node* const node, const int64_t cost)
{
    const uint32_t nidx = node->idx;
    if (nidx == d->finishIndex)
        return cost;
    
    state->traversed[nidx] = true;
    int64_t newcost = -1;
    for (int i = 0; i < 4; ++i)
    {
        if (!node->next[i].node)
            continue;
        if (state->traversed[node->next[i].node->idx])
            continue;
        const int64_t ncost = findmaxlen2slowly(d, state, node->next[i].node, node->next[i].cost);
        newcost = MAX(newcost, ncost);
    }
    state->traversed[nidx] = false;
    return 1 + cost + newcost;
}

typedef struct parworkstate
{
    slowstate* state;
    astar_node* node;
    int64_t cost;
} parworkstate;

struct parsetupstate;

typedef struct segment
{
    pthread_t thread;
    const fdata* d;
    struct parsetupstate* state;
    uint32_t firstwork;
    int64_t maxlen; // out
} segment;

typedef struct parsetupstate
{
    slowstate* state;
    uint32_t visited[8];
    uint32_t visitedCount;
    segment segments[32];
    uint32_t segmentCount;
    parworkstate workstates[512];
    uint32_t workstatesCount;
    uint32_t nextworkstate;
} parsetupstate;

static void* findmaxlenpar2_thread(void* vseg)
{
    segment* seg = (segment*)vseg;

    int workidx = seg->firstwork;
    do
    {
        int64_t result = findmaxlen2slowly(
            seg->d,
            seg->state->workstates[workidx].state,
            seg->state->workstates[workidx].node,
            seg->state->workstates[workidx].cost);
        seg->maxlen = MAX(seg->maxlen, result);
    } while ((workidx = __atomic_fetch_add(&seg->state->nextworkstate, 1, __ATOMIC_ACQ_REL)) < seg->state->workstatesCount);
    return NULL;
}

static void findmaxlen2par_setup(const fdata* const d, parsetupstate* const state, const astar_node* const node, int64_t cost, int depth, int targetdepth)
{
    state->state->traversed[node->idx] = true;
    state->visited[state->visitedCount++] = node->idx;
    int64_t newcost = -1;
    if (depth == targetdepth)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (!node->next[i].node)
                continue;
            if (state->state->traversed[node->next[i].node->idx])
                continue;
            
            int wsidx = state->workstatesCount;
            state->workstates[wsidx].node = node->next[i].node;
            state->workstates[wsidx].cost = cost + node->next[i].cost;
            state->workstates[wsidx].state = (slowstate*)calloc(1, sizeof(slowstate));
            for (int vs = 0; vs < state->visitedCount; ++vs)
                state->workstates[wsidx].state->traversed[state->visited[vs]] = true;

            ++state->workstatesCount;
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            if (!node->next[i].node)
                continue;
            if (state->state->traversed[node->next[i].node->idx])
                continue;
            findmaxlen2par_setup(d, state, node->next[i].node, cost + 1 + node->next[i].cost, depth + 1, targetdepth);
        }
    }
    --state->visitedCount;
    state->state->traversed[node->idx] = false;
}

static int64_t findmaxlen2par(const fdata* const d, slowstate* const state, const astar_node* const node, int64_t cost)
{
#if defined(__linux__)
    int nthreads = get_nprocs();
#elif defined(_WIN32)
    int nthreads = atoi(getenv("NUMBER_OF_PROCESSORS"));
#else
    return findmaxlen2slowly(d, state, node, cost);
#endif
    DEBUGLOG("nthreads %d\n", nthreads);

    int targetdepth = 6;

    parsetupstate pss = { .state = state };
    findmaxlen2par_setup(d, &pss, node, cost, 0, targetdepth);

    DEBUGLOG("work states %u\n", pss.workstatesCount);

    int64_t newcost = 0;
    pss.nextworkstate = nthreads;
    for (int i = 0; i < nthreads; ++i)
    {
        pss.segments[i].d = d;
        pss.segments[i].firstwork = i;
        pss.segments[i].state = &pss;

        pthread_create(&pss.segments[i].thread, NULL, findmaxlenpar2_thread, pss.segments + i);        
    }
    for (int i = 0; i < nthreads; ++i)
    {
        pthread_join(pss.segments[i].thread, NULL);
        int64_t ncost = 1 + pss.segments[i].maxlen;
        newcost = MAX(newcost, ncost);
    }
    return newcost;
}

static int64_t findmaxlen2(fdata* d)
{
    astar_node* nodes = (astar_node*)calloc(32768*4, sizeof(astar_node));

    astar_node curnode = {
        .idx = d->startIndex,
        .prevdir = PD_NONE,
    };

    slowstate state = {0};
    DEBUGLOG("building graph\n");
    DSTOPWATCH_START(graph);
    // buildgraph(d, &state, nodes, &curnode);
    buildgraphpar(d, &state, nodes, &curnode);
    DSTOPWATCH_END(graph);
    // printgraph(d, &state, &curnode, 0);
    DSTOPWATCH_PRINT(graph);

    return findmaxlen2par(d, &state, &curnode, 0);
    // return findmaxlen2slowly(d, &state, &curnode, 0);
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

