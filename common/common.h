#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// #define ENABLE_DEBUGLOG
// #define ENABLE_CHECKS

typedef unsigned char chartype;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#if defined(_MSC_VER) && !defined(__clang__)
#define FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__) || defined(__GNUC_MINOR__)
#define FORCEINLINE __attribute__((always_inline))
#else
#warning Unknown platform, no force-inline support
#define FORCEINLINE
#endif

// Add compiler hints to help out branch prediction
#if defined(__GNUC__)
#define LIKELY(expr)    (__builtin_expect(!!(expr), 1))
#define UNLIKELY(expr)  (__builtin_expect(!!(expr), 0))
#else
#define LIKELY(expr) (expr)
#define UNLIKELY(expr) (expr)
#endif

#define RESTRICT __restrict

#if defined(ENABLE_DEBUGLOG)
#define DEBUGLOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUGLOG(...) (void)0
#endif

#if defined(ENABLE_CHECKS)
#define CHECK(x, ...) if (!(x)) { fprintf(stderr, __VA_ARGS__); exit(1); }
#else
#define CHECK(x, ...) (void)0
#endif
