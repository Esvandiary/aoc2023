#pragma once

#include <stdio.h>
#include "common.h"

static inline FORCEINLINE char* _write_uint64_buf(uint64_t n, char* const buf, const size_t size)
{
    char* p = buf + size;
    *--p = '\0';
    do
    {
        *--p = '0' + (n % 10);
        n /= 10;
    } while (n);
    return p;
}

static inline FORCEINLINE void print_uint64(uint64_t n)
{
    char buf[100];
    char* p = _write_uint64_buf(n, buf, 100);
    puts(p);
}

static inline FORCEINLINE void print_int64(int64_t n)
{
    char buf[100];
    char* p = _write_uint64_buf((n >= 0 ? n : -n), buf, 100);
    if (n < 0) *--p = '-';
    puts(p);
}