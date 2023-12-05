#pragma once

#include "common.h"

#include <string.h>

typedef struct vuctor
{
    void* data;
    size_t size;
    size_t capacity;
} vuctor;

#define VUCTOR_GET(v, type, i) (((type*)((v).data))[i])
#define VUCTOR_ADD(v, type, value) _vuctor_add(&(v), sizeof(type), &(value))
#define VUCTOR_RESERVE(v, type, capacity) _vuctor_reserve(&(v), sizeof(type), (capacity));
#define VUCTOR_FREE(v) _vuctor_free(v)

static inline FORCEINLINE void _vuctor_reserve(vuctor* v, size_t elemSize, size_t capacity)
{
    if (capacity > v->capacity)
    {
        v->data = realloc(v->data, capacity * elemSize);
        v->capacity = capacity;
    }
}

static inline FORCEINLINE void _vuctor_add(vuctor* v, size_t elemSize, void* value)
{
    if (v->size >= v->capacity)
        _vuctor_reserve(v, elemSize, MAX(v->capacity * 8, 32));
    memcpy((char*)v->data + (elemSize * v->size), value, elemSize);
    ++v->size;
}

static inline FORCEINLINE void _vuctor_free(vuctor* v)
{
    free(v->data);
    memset(v, 0, sizeof(vuctor));
}
