#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../common/common.h"

#if !defined(MINHEAP_NAME) || !defined(MINHEAP_TYPE) || !defined(MINHEAP_SCORE)
    #error Minheap requires MINHEAP_NAME, MINHEAP_TYPE and MINHEAP_SCORE(x) to be set
#endif

#define _MINHEAP_CONCAT2A(x, y) x ## y
#define _MINHEAP_CONCAT2(x, y) _MINHEAP_CONCAT2A(x, y)
#define _MINHEAP_CONCAT3A(x, y, z) x ## y ## z
#define _MINHEAP_CONCAT3(x, y, z) _MINHEAP_CONCAT3A(x, y, z)
#define _MINHEAP_OBJ _MINHEAP_CONCAT2(MINHEAP_NAME, _minheap)
#define _MINHEAP_FN_NAME(fn) _MINHEAP_CONCAT3(MINHEAP_NAME, _minheap_, fn)
#define _MINHEAP_FN_NAME_INT(fn) _MINHEAP_CONCAT2(_, _MINHEAP_CONCAT3(MINHEAP_NAME, _minheap_, fn))

#define _MINHEAP_PARENT(i) (((i)-1)/2)
#define _MINHEAP_LCHILD(i) (((i)*2)+1)
#define _MINHEAP_RCHILD(i) (((i)*2)+2)

static inline FORCEINLINE void _MINHEAP_FN_NAME_INT(swap)(MINHEAP_TYPE** x, MINHEAP_TYPE** y)
{
    MINHEAP_TYPE* temp = *x;
    *x = *y;
    *y = temp;
}

typedef struct _MINHEAP_OBJ
{
    MINHEAP_TYPE** harr; // pointer to array of elements in heap
    int capacity; // maximum possible size of min heap
    int heap_size; // Current number of elements in min heap
} _MINHEAP_OBJ;

static inline FORCEINLINE _MINHEAP_OBJ* _MINHEAP_FN_NAME(init)(int capacity)
{
    _MINHEAP_OBJ* h = (_MINHEAP_OBJ*)malloc(sizeof(_MINHEAP_OBJ));
    h->heap_size = 0;
    h->capacity = capacity;
    h->harr = (MINHEAP_TYPE**)calloc(capacity, sizeof(MINHEAP_TYPE*));
    return h;
}

static inline FORCEINLINE void _MINHEAP_FN_NAME(destroy)(_MINHEAP_OBJ* h)
{
    free(h->harr);
    free(h);
}

static inline FORCEINLINE bool _MINHEAP_FN_NAME(contains)(_MINHEAP_OBJ* h, MINHEAP_TYPE* k)
{
    for (int i = 0; i < h->heap_size; ++i)
        if (h->harr[i] == k)
            return true;
    return false;
}

static inline MINHEAP_TYPE* _MINHEAP_FN_NAME(extract_min)(_MINHEAP_OBJ* h);

void _MINHEAP_FN_NAME(delete)(_MINHEAP_OBJ* h, MINHEAP_TYPE* node)
{
    for (int i = 0; i < h->heap_size; ++i)
    {
        if (h->harr[i] == node)
        {
            while (i != 0)
            {
                _MINHEAP_FN_NAME_INT(swap)(&h->harr[i], &h->harr[_MINHEAP_PARENT(i)]);
                i = _MINHEAP_PARENT(i);
            }
            _MINHEAP_FN_NAME(extract_min)(h);
            return;
        }
    }
}

// A recursive method to heapify a subtree with the root at given index
// This method assumes that the subtrees are already heapified
static inline void _MINHEAP_FN_NAME_INT(minheapify_tb)(_MINHEAP_OBJ* h, int i)
{
    if (h->heap_size <= 1)
        return;

    int l = _MINHEAP_LCHILD(i);
    int r = _MINHEAP_RCHILD(i);
    int smallest = i;
    if (l < h->heap_size && MINHEAP_SCORE(h->harr[l]) < MINHEAP_SCORE(h->harr[smallest]))
        smallest = l;
    if (r < h->heap_size && MINHEAP_SCORE(h->harr[r]) < MINHEAP_SCORE(h->harr[smallest]))
        smallest = r;
    if (smallest != i)
    {
        _MINHEAP_FN_NAME_INT(swap)(&h->harr[i], &h->harr[smallest]);
        _MINHEAP_FN_NAME_INT(minheapify_tb)(h, smallest);
    }
}

static inline void _MINHEAP_FN_NAME_INT(minheapify_bt)(_MINHEAP_OBJ* h, int i)
{
    if (MINHEAP_SCORE(h->harr[_MINHEAP_PARENT(i)]) > MINHEAP_SCORE(h->harr[i]))
    {
        _MINHEAP_FN_NAME_INT(swap)(&h->harr[_MINHEAP_PARENT(i)], &h->harr[i]);
        _MINHEAP_FN_NAME_INT(minheapify_bt)(h, _MINHEAP_PARENT(i));
    }
}

static inline int _MINHEAP_FN_NAME(insert)(_MINHEAP_OBJ* h, MINHEAP_TYPE* k)
{
    if (h->heap_size == h->capacity)
        return -1;
 
    // First insert the new key at the end
    int i = h->heap_size++;
    h->harr[i] = k;
 
    _MINHEAP_FN_NAME_INT(minheapify_bt)(h, i);
    return 0;
}

static inline MINHEAP_TYPE* _MINHEAP_FN_NAME(extract_min)(_MINHEAP_OBJ* h)
{
    if (h->heap_size <= 0)
        return NULL;
    if (h->heap_size == 1)
    {
        h->heap_size--;
        return h->harr[0];
    }
 
    // Store the minimum value, and remove it from heap
    MINHEAP_TYPE* root = h->harr[0];
    h->harr[0] = h->harr[--h->heap_size];
    _MINHEAP_FN_NAME_INT(minheapify_tb)(h, 0);
 
    return root;
}

#undef _MINHEAP_PARENT
#undef _MINHEAP_LCHILD
#undef _MINHEAP_RCHILD
#undef _MINHEAP_OBJ
#undef _MINHEAP_FN_NAME
#undef _MINHEAP_FN_NAME_INT
#undef _MINHEAP_CONCAT2A
#undef _MINHEAP_CONCAT2
#undef _MINHEAP_CONCAT3A
#undef _MINHEAP_CONCAT3
