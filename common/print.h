#pragma once

#include <stdio.h>
#include "common.h"

static inline FORCEINLINE void print_uint64(uint64_t n)
{
    char buf[100];
    char* p = buf + 100;
    *--p = '\0';
    do
    {
        *--p = '0' + (n % 10);
        n /= 10;
    } while (n);

    puts(p);
}