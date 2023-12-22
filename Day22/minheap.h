#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../common/common.h"
#include "../common/uthash/uthash.h"

#define nodeindex(yc, xc, y, x, dir) (((y)*(xc) + (x)) << 2 | (dir))

typedef struct bpos
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
} bpos;

typedef uint32_t brickid;

typedef struct brick
{
    brickid id;
    bpos startpos;
    bpos endpos;
    brickid supportedby[8];
    uint16_t supportedbyCount;
    brickid supporting[8];
    uint16_t supportingCount;
    uint16_t dependentsCount;
} brick;

//
// minheap
//

#define BRICK_MINHEAP_PARENT(i) (((i)-1)/2)
#define BRICK_MINHEAP_LCHILD(i) (((i)*2)+1)
#define BRICK_MINHEAP_RCHILD(i) (((i)*2)+2)

static inline FORCEINLINE void brick_minheap_swap(brick** x, brick** y)
{
    brick* temp = *x;
    *x = *y;
    *y = temp;
}

typedef struct brick_minheap
{
    brick** harr; // pointer to array of elements in heap
    int capacity; // maximum possible size of min heap
    int heap_size; // Current number of elements in min heap
} brick_minheap;

#define BRICK_MINHEAP_SCORE(node) ((node)->startpos.z)

static inline FORCEINLINE brick_minheap* brick_minheap_init(int capacity)
{
    brick_minheap* h = (brick_minheap*)malloc(sizeof(brick_minheap));
    h->heap_size = 0;
    h->capacity = capacity;
    h->harr = (brick**)calloc(capacity, sizeof(brick*));
    return h;
}

static inline FORCEINLINE void brick_minheap_destroy(brick_minheap* h)
{
    free(h->harr);
    free(h);
}

static inline FORCEINLINE bool brick_minheap_contains(brick_minheap* h, brick* k)
{
    for (int i = 0; i < h->heap_size; ++i)
        if (h->harr[i] == k)
            return true;
    return false;
}

void brick_minheap_decrease_key(brick_minheap* h, int i, int64_t new_score)
{
    BRICK_MINHEAP_SCORE(h->harr[i]) = new_score;
    while (i != 0 && BRICK_MINHEAP_SCORE(h->harr[BRICK_MINHEAP_PARENT(i)]) > BRICK_MINHEAP_SCORE(h->harr[i]))
    {
       brick_minheap_swap(&h->harr[i], &h->harr[BRICK_MINHEAP_PARENT(i)]);
       i = BRICK_MINHEAP_PARENT(i);
    }
}

static inline brick* brick_minheap_extract_min(brick_minheap* h);

void brick_minheap_delete(brick_minheap* h, brick* node)
{
    for (int i = 0; i < h->heap_size; ++i)
    {
        if (h->harr[i] == node)
        {
            brick_minheap_decrease_key(h, i, INT64_MIN);
            brick_minheap_extract_min(h);
            return;
        }
    }
}

// A recursive method to heapify a subtree with the root at given index
// This method assumes that the subtrees are already heapified
static inline void brick_minheap_minheapify_tb(brick_minheap* h, int i)
{
    if (h->heap_size <= 1)
        return;

    int l = BRICK_MINHEAP_LCHILD(i);
    int r = BRICK_MINHEAP_RCHILD(i);
    int smallest = i;
    if (l < h->heap_size && BRICK_MINHEAP_SCORE(h->harr[l]) < BRICK_MINHEAP_SCORE(h->harr[smallest]))
        smallest = l;
    if (r < h->heap_size && BRICK_MINHEAP_SCORE(h->harr[r]) < BRICK_MINHEAP_SCORE(h->harr[smallest]))
        smallest = r;
    if (smallest != i)
    {
        brick_minheap_swap(&h->harr[i], &h->harr[smallest]);
        brick_minheap_minheapify_tb(h, smallest);
    }
}

static inline void brick_minheap_minheapify_bt(brick_minheap* h, int i)
{
    if (BRICK_MINHEAP_SCORE(h->harr[BRICK_MINHEAP_PARENT(i)]) > BRICK_MINHEAP_SCORE(h->harr[i]))
    {
        brick_minheap_swap(&h->harr[BRICK_MINHEAP_PARENT(i)], &h->harr[i]);
        brick_minheap_minheapify_bt(h, BRICK_MINHEAP_PARENT(i));
    }
}

static inline int brick_minheap_insert(brick_minheap* h, brick* k)
{
    if (h->heap_size == h->capacity)
        return -1;
 
    // First insert the new key at the end
    int i = h->heap_size++;
    h->harr[i] = k;
 
    brick_minheap_minheapify_bt(h, i);
    return 0;
}

static inline brick* brick_minheap_extract_min(brick_minheap* h)
{
    if (h->heap_size <= 0)
        return NULL;
    if (h->heap_size == 1)
    {
        h->heap_size--;
        return h->harr[0];
    }
 
    // Store the minimum value, and remove it from heap
    brick* root = h->harr[0];
    h->harr[0] = h->harr[--h->heap_size];
    brick_minheap_minheapify_tb(h, 0);
 
    return root;
}