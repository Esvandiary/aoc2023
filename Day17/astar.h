#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../common/common.h"
#include "../common/uthash/uthash.h"

#define nodeindex(yc, xc, y, x, dir) (((y)*(xc) + (x)) << 2 | (dir))

typedef struct astar_pos
{
    int32_t y;
    int32_t x;
} astar_pos;

typedef struct astar_node
{
    astar_pos pos;
    int8_t dir;
    int32_t heatloss;
    int64_t fScore;
    bool traversed;
    UT_hash_handle hh;
} astar_node;

#define MINHEAP_NAME astar
#define MINHEAP_TYPE astar_node
#define MINHEAP_SCORE(node) ((node)->fScore)
#include "../common/minheap.h"

#define ASTAR_DIR_NONE -1
#define ASTAR_DIR_UP 0
#define ASTAR_DIR_LEFT 1
#define ASTAR_DIR_DOWN 2
#define ASTAR_DIR_RIGHT 3

static int16_t xdiff[4] = {0, -1, 0, 1};
static int16_t ydiff[4] = {-1, 0, 1, 0};
static char dirchar[4] = {'N','W','S','E'};

static int64_t calculate(astar_node* nodes, int yc, int xc, astar_pos start, astar_pos goal, int min_steps, int max_steps)
{
    astar_minheap* openset = astar_minheap_init(8192);

    astar_node start_node = {
        .pos = (astar_pos) {.y = start.y, .x = start.x },
        .dir = ASTAR_DIR_NONE,
        .fScore = 0,
        .heatloss = 0,
        .traversed = false,
    };

    astar_minheap_insert(openset, &start_node);

    while (openset->heap_size)
    {
        astar_node* current = astar_minheap_extract_min(openset);
        DEBUGLOG("got min node @ [%d,%d dir %c] with fScore %ld\n", current->pos.y, current->pos.x, dirchar[current->dir], current->fScore);
        if (current->pos.x == goal.x && current->pos.y == goal.y)
        {
            DEBUGLOG("found score of %ld\n", current->fScore);
            return current->fScore;
        }

        if (current->traversed)
            continue;
        current->traversed = true;

        for (int dir = 0; dir < 4; ++dir)
        {
            if (dir == current->dir || ((dir + 2) % 4) == current->dir)
                continue;

            int move_cost = 0;
            int cx = current->pos.x;
            int cy = current->pos.y;
            for (int i = 1; i <= max_steps; ++i)
            {
                cx += xdiff[dir];
                cy += ydiff[dir];
                if (cx < 0 || cx >= xc - 1 || cy < 0 || cy >= yc)
                    break;

                astar_node* node = nodes + nodeindex(yc, xc, cy, cx, dir);
                move_cost += node->heatloss;
                int64_t newfscore = current->fScore + move_cost;
                if (i < min_steps || newfscore > node->fScore)
                    continue;

                node->fScore = newfscore;
                
                DEBUGLOG("node @ [%d,%d dir %c] fScore now %ld\n", node->pos.y, node->pos.x, dirchar[node->dir], node->fScore);
                astar_minheap_insert(openset, node);
            }
        }
    }
    return -1;
}
