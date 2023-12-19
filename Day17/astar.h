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

#define ASTAR_DIR_NONE -1
#define ASTAR_DIR_UP 0
#define ASTAR_DIR_LEFT 1
#define ASTAR_DIR_DOWN 2
#define ASTAR_DIR_RIGHT 3

//
// minheap
//

#define ASTAR_MINHEAP_PARENT(i) (((i)-1)/2)
#define ASTAR_MINHEAP_LCHILD(i) (((i)*2)+1)
#define ASTAR_MINHEAP_RCHILD(i) (((i)*2)+2)

static inline FORCEINLINE void astar_minheap_swap(astar_node** x, astar_node** y)
{
    astar_node* temp = *x;
    *x = *y;
    *y = temp;
}

typedef struct astar_minheap
{
    astar_node** harr; // pointer to array of elements in heap
    int capacity; // maximum possible size of min heap
    int heap_size; // Current number of elements in min heap
} astar_minheap;

static inline FORCEINLINE astar_minheap* astar_minheap_init(int capacity)
{
    astar_minheap* h = (astar_minheap*)malloc(sizeof(astar_minheap));
    h->heap_size = 0;
    h->capacity = capacity;
    h->harr = (astar_node**)calloc(capacity, sizeof(astar_node*));
    return h;
}

static inline FORCEINLINE void astar_minheap_destroy(astar_minheap* h)
{
    free(h->harr);
    free(h);
}

static inline FORCEINLINE bool astar_minheap_contains(astar_minheap* h, astar_node* k)
{
    for (int i = 0; i < h->heap_size; ++i)
        if (h->harr[i] == k)
            return true;
    return false;
}

static inline int astar_minheap_insert(astar_minheap* h, astar_node* k)
{
    if (h->heap_size == h->capacity)
        return -1;
 
    // First insert the new key at the end
    int i = h->heap_size++;
    h->harr[i] = k;
 
    // Fix the min heap property if it is violated
    while (i != 0 && h->harr[ASTAR_MINHEAP_PARENT(i)] > h->harr[i])
    {
       astar_minheap_swap(&h->harr[i], &h->harr[ASTAR_MINHEAP_PARENT(i)]);
       i = ASTAR_MINHEAP_PARENT(i);
    }
    return 0;
}

void astar_minheap_decrease_key(astar_minheap* h, int i, int new_score)
{
    h->harr[i]->fScore = new_score;
    while (i != 0 && h->harr[ASTAR_MINHEAP_PARENT(i)]->fScore > h->harr[i]->fScore)
    {
       astar_minheap_swap(&h->harr[i], &h->harr[ASTAR_MINHEAP_PARENT(i)]);
       i = ASTAR_MINHEAP_PARENT(i);
    }
}

static inline astar_node* astar_minheap_extract_min(astar_minheap* h);

void astar_minheap_delete(astar_minheap* h, astar_node* node)
{
    for (int i = 0; i < h->heap_size; ++i)
    {
        if (h->harr[i] == node)
        {
            astar_minheap_decrease_key(h, i, INT32_MIN);
            astar_minheap_extract_min(h);
            return;
        }
    }
}

// A recursive method to heapify a subtree with the root at given index
// This method assumes that the subtrees are already heapified
static inline void astar_minheap_minheapify(astar_minheap* h, int i)
{
    int l = ASTAR_MINHEAP_LCHILD(i);
    int r = ASTAR_MINHEAP_RCHILD(i);
    int smallest = i;
    if (l < h->heap_size && h->harr[l]->fScore < h->harr[i]->fScore)
        smallest = l;
    if (r < h->heap_size && h->harr[r]->fScore < h->harr[smallest]->fScore)
        smallest = r;
    if (smallest != i)
    {
        astar_minheap_swap(&h->harr[i], &h->harr[smallest]);
        astar_minheap_minheapify(h, smallest);
    }
}

static inline astar_node* astar_minheap_extract_min(astar_minheap* h)
{
    if (h->heap_size <= 0)
        return NULL;
    if (h->heap_size == 1)
    {
        h->heap_size--;
        return h->harr[0];
    }
 
    // Store the minimum value, and remove it from heap
    astar_node* root = h->harr[0];
    h->harr[0] = h->harr[h->heap_size-1];
    h->heap_size--;
    astar_minheap_minheapify(h, 0);
 
    return root;
}

static int16_t xdiff[4] = {0, -1, 0, 1};
static int16_t ydiff[4] = {-1, 0, 1, 0};

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
        if (current->pos.x == goal.x && current->pos.y == goal.y)
        {
            return current->fScore;
        }

        if (current->traversed)
            continue;
        current->traversed = true;

        for (int dir = 0; dir < 4; ++dir)
        {
            if (dir == current->dir)
                continue;
            if (((dir + 2) % 4) == current->dir)
                continue;
            
            int move_cost = 0;
            int cx = current->pos.x;
            int cy = current->pos.y;
            for (int i = 1; i <= max_steps; ++i)
            {
                cx += xdiff[dir];
                cy += ydiff[dir];
                if (cx < 0 || cx >= xc - 1 || cy < 0 || cy >= yc)
                    continue;
                
                astar_node* node = nodes + nodeindex(yc, xc, cy, cx, dir);
                move_cost += node->heatloss;
                node->fScore = current->fScore + move_cost;
                if (i < min_steps)
                    continue;
                
                astar_minheap_insert(openset, node);
            }
        }
    }
    return -1;
}
