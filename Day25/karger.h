#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/common.h"

typedef uint16_t node_t;

typedef struct ks_edge
{
    node_t head;
    node_t tail;
} ks_edge;

typedef struct ks_edgegraph
{
    uint32_t nvertices;
    uint32_t edgesCount;
    ks_edge edges[16382];
} ks_edgegraph;

typedef struct ks_unionfind_subset
{
    node_t id;
    node_t size;
} ks_unionfind_subset;

typedef struct ks_unionfind
{
    ks_unionfind_subset subsets[2046];
    uint32_t subsetsCount;
    uint32_t fullSubsetsCount;
} ks_unionfind;

static inline FORCEINLINE void ks_unionfind_init(ks_unionfind* const uf, const uint32_t n)
{
    uf->fullSubsetsCount = n;
    uf->subsetsCount = n;
    for (int i = 0; i < n; ++i)
        uf->subsets[i] = (ks_unionfind_subset) { .id = i, .size = 1 };
}

static inline node_t ks_unionfind_find(ks_unionfind* const uf, node_t x)
{
    node_t root = x;
    while (root != uf->subsets[root].id)
    {
        root = uf->subsets[root].id;
    }
    while (uf->subsets[x].id != root)
    {
        node_t id = uf->subsets[x].id;
        uf->subsets[x].id = root;
        x = id;
    }
    return root;
}

static inline void ks_unionfind_merge(ks_unionfind* const uf, const node_t x, const node_t y)
{
    const node_t i = ks_unionfind_find(uf, x);
    const node_t j = ks_unionfind_find(uf, y);
    if (i == j)
        return;
    if (uf->subsets[i].size < uf->subsets[j].size)
    {
        uf->subsets[i].id = j;
        uf->subsets[j].size += uf->subsets[i].size;
    }
    else
    {
        uf->subsets[j].id = i;
        uf->subsets[i].size += uf->subsets[j].size;
    }
    --uf->subsetsCount;
}

static inline FORCEINLINE bool ks_unionfind_connected(ks_unionfind* const uf, const node_t x, const node_t y)
{
    return ks_unionfind_find(uf, x) == ks_unionfind_find(uf, y);
}


typedef struct ks_graphcut
{
    ks_unionfind uf;
    size_t cut_size;
} ks_graphcut;

typedef struct ks_psresult
{
    uint32_t partition1Size;
    uint32_t partition2Size;
} ks_psresult;

typedef struct ks_presult
{
    ks_psresult sizes;
    node_t partition1[2046];
    node_t partition2[2046];
} ks_presult;

static inline FORCEINLINE void ks_graphcut_init(ks_graphcut* const gc, const uint32_t n)
{
    ks_unionfind_init(&gc->uf, n);
    gc->cut_size = 0;
}

static inline ks_presult ks_graphcut_get_partitions(const ks_graphcut* const gc)
{
    ks_presult result = {0};
    for (int i = 0; i < gc->uf.fullSubsetsCount; ++i)
    {
        if (gc->uf.subsets[i].id == gc->uf.subsets[0].id)
            result.partition1[result.sizes.partition1Size++] = i;
        else
            result.partition2[result.sizes.partition2Size++] = i;
    }
    return result;
}

static inline ks_psresult ks_graphcut_get_partition_sizes(const ks_graphcut* const gc)
{
    ks_psresult result = { .partition1Size = 1, .partition2Size = 0 };
    const node_t zeroid = gc->uf.subsets[0].id;
    const uint32_t scount = gc->uf.fullSubsetsCount;
    for (int i = 1; i < scount; ++i)
    {
        if (gc->uf.subsets[i].id == zeroid)
            ++result.partition1Size;
        else
            ++result.partition2Size;
    }
    return result;
}

static void ks_perform(ks_edgegraph* const graph, ks_graphcut* const gc)
{
    ks_graphcut_init(gc, graph->nvertices);
    int start = 0;
    for (int m = graph->edgesCount; gc->uf.subsetsCount != 2; ++start, --m)
    {
        const int midx = start + (rand() % m);
        const ks_edge tmp = graph->edges[start];
        graph->edges[start] = graph->edges[midx];
        graph->edges[midx] = tmp;

        ks_unionfind_merge(&gc->uf, graph->edges[start].tail, graph->edges[start].head);
    }

    for (int i = 0; i < graph->edgesCount; ++i)
    {
        if (!ks_unionfind_connected(&gc->uf, graph->edges[i].tail, graph->edges[i].head))
            ++gc->cut_size;
    }
}

