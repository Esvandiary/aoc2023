#pragma once

#include "common.h"

#include <string.h>

typedef struct vuctor
{
    void* data;
    size_t size;
    size_t capacity;
} vuctor;

#define VUCTOR_INIT { .data = NULL, .size = 0, .capacity = 0 };

#define VUCTOR_GET(v, type, i) (((type*)((v).data))[i])
#define VUCTOR_GET_PTR(v, type, i) (((type*)((v).data)) + (i))
#define VUCTOR_ADD(v, type, value) _vuctor_add(&(v), sizeof(type), &(value))
#define VUCTOR_ADD_NOINIT(v, type) (type*)_vuctor_add_noinit(&(v), sizeof(type))
#define VUCTOR_RESERVE(v, type, capacity) _vuctor_reserve(&(v), sizeof(type), (capacity))
#define VUCTOR_RESIZE(v, type, size) _vuctor_resize(&(v), sizeof(type), (size))
#define VUCTOR_CLEAR(v, type) ((v).size = 0)
#define VUCTOR_FREE(v) _vuctor_free(v)

static inline FORCEINLINE void _vuctor_reserve(vuctor* v, size_t elemSize, size_t capacity)
{
    if (capacity > v->capacity)
    {
        v->data = realloc(v->data, capacity * elemSize);
        v->capacity = capacity;
    }
}

static inline FORCEINLINE void _vuctor_resize(vuctor* v, size_t elemSize, size_t size)
{
    _vuctor_reserve(v, elemSize, size);
    v->size = size;
}

static inline FORCEINLINE void* _vuctor_add_noinit(vuctor* v, size_t elemSize)
{
    if (v->size >= v->capacity)
        _vuctor_reserve(v, elemSize, MAX(v->capacity * 8, 32));
    ++v->size;
    return (char*)v->data + elemSize * (v->size - 1);
}

static inline FORCEINLINE void _vuctor_add(vuctor* v, size_t elemSize, const void* value)
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
