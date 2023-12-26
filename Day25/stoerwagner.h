#pragma once

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "../common/common.h"

#define maxn 2048
const int sw_inf = 1000000000;

typedef struct sw_state
{
    int n;
    int edge[maxn][maxn];
    int co_count[maxn];
    int dist[maxn];
    bool vis[maxn], bin[maxn];
} sw_state;

static void sw_init(sw_state* state, int n)
{
    memset(state->edge, 0, sizeof(state->edge));
    memset(state->bin, false, sizeof(state->bin));
    state->n = n;
    for (int i = 0; i < n; ++i)
        state->co_count[i] = 1;
}

static int sw_contract(sw_state* state, int* s, int* t)
{
    memset(state->dist, 0, sizeof(state->dist));
    memset(state->vis, false, sizeof(state->vis));
    int i, j, k, mincut, maxc;

    for (i = 0; i < state->n; i++)
    {
        k = -1; maxc = -1;
        for (j = 0; j < state->n; j++)
        {
            if (!state->bin[j] && !state->vis[j] && state->dist[j] > maxc)
            {
                k = j;
                maxc = state->dist[j];
            }
        }
        if (k == -1)
        {
            // DEBUGLOG("early returning mincut %d\n", mincut);
            return mincut;
        }
        *s = *t;  *t = k;
        mincut = maxc;
        state->vis[k] = true;
        for (j = 0; j < state->n; j++)
            if (!state->bin[j] && !state->vis[j])  
                state->dist[j] += state->edge[k][j];
    }

    // DEBUGLOG("returning mincut %d\n", mincut);
    return mincut;  
}

static int sw_perform(sw_state* state)  
{  
    int mincut, i, j, s, t, ans, mt;

    for (mincut = sw_inf, i = 0; i < state->n - 1; i++)  
    {  
        ans = sw_contract(state, &s, &t);
        state->bin[t] = true;
        if (mincut > ans)
        {
            mincut = ans;
            mt = t;
        }
        if (mincut == 0)
            return 0;
        state->co_count[s] += state->co_count[t];
        for (j = 0; j < state->n; j++)
            if (!state->bin[j])
                state->edge[s][j] = (state->edge[j][s] += state->edge[j][t]);
    }

    return mincut | (mt << 8);
}

#undef maxn