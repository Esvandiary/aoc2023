#pragma once

#include "common.h"
#include <time.h>

#define _DSVN(name, suffix) _dstopwatch_ ## name ## _ ## suffix

#if defined(ENABLE_DEBUGLOG) || defined(ENABLE_DSTOPWATCH)
#define DSTOPWATCH_START(name) struct timespec _DSVN(name, start); clock_gettime(CLOCK_MONOTONIC, &_DSVN(name, start));
#define DSTOPWATCH_END(name) struct timespec _DSVN(name, end); clock_gettime(CLOCK_MONOTONIC, &_DSVN(name, end));
#define DSTOPWATCH_PRINT(name) printf("%s took %luns\n", #name, (_DSVN(name, end).tv_sec - _DSVN(name, start).tv_sec) * 1000000000 + _DSVN(name, end).tv_nsec - _DSVN(name, start).tv_nsec);
#else
#define DSTOPWATCH_START(name) (void)0;
#define DSTOPWATCH_END(name) (void)0;
#define DSTOPWATCH_PRINT(name) (void)0;
#endif