#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #define ENABLE_DEBUGLOG
// #define ENABLE_DSTOPWATCH
#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/stopwatch.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define dataindex(y, x) ((y)*d.lineLength + (x))

#define M_UNTYPED 0
#define M_BROADCAST 1
#define M_FLIPFLOP 2
#define M_CONJUNCTION 3

static const char* modnames[] = {"untyped", "broadcaster", "flipflop", "conjunction"};

#define MIDX_BROADCAST 1023

#define MIDX(c1, c2) ((((c1) & 0x1F) << 5) | ((c2) & 0x1F))
#define MICHARS(midx) ((midx) != MIDX_BROADCAST ? (((midx) >> 5) + 96) : 'B'), ((midx) != MIDX_BROADCAST ? (((midx) & 0x1F) + 96) : 'C')

typedef struct mport
{
    uint16_t module;
    uint8_t index;
} mport;

typedef struct mpulse
{
    mport port;
    uint8_t pulse;
} mpulse;

typedef struct module
{
    uint8_t type;
    uint8_t num_inputs;
    mport outputs[10];
} module;

typedef struct p2state
{
    uint16_t module;
    uint16_t value;
} p2state;

static inline void run(module* modules, uint16_t* mstate, uint64_t* pulsecounts, p2state* p2state)
{
    mpulse queue[4096] = {0};
    size_t qidxr = 0, qidxw = 0;
    queue[qidxw++] = (mpulse) { .port = (mport) { .module = MIDX_BROADCAST, .index = 0 }, .pulse = 0 };
    while (qidxr != qidxw)
    {
        const mpulse qpulse = queue[qidxr++];
        const mport qp = qpulse.port;
        ++pulsecounts[qpulse.pulse];

        // DEBUGLOG("[%zu] popped %c%c[%u], pulse %u, queue length now %zu\n", qidxr - 1, MICHARS(qp.module), qp.index, qpulse.pulse, qidxw - qidxr);

        switch (modules[qp.module].type)
        {
            case M_UNTYPED:
                mstate[qp.module] = qpulse.pulse;
                break;
            case M_BROADCAST:
                for (int i = 0; i < 16 && modules[qp.module].outputs[i].module; ++i)
                    queue[qidxw++] = (mpulse) { .port = modules[qp.module].outputs[i], .pulse = qpulse.pulse };
                break;
            case M_FLIPFLOP:
                if (qpulse.pulse == 0)
                {
                    mstate[qp.module] = (~mstate[qp.module]) & 1;
                    for (int i = 0; i < 16 && modules[qp.module].outputs[i].module; ++i)
                        queue[qidxw++] = (mpulse) { .port = modules[qp.module].outputs[i], .pulse = mstate[qp.module] };
                }
                break;
            case M_CONJUNCTION:
            {
                if (qpulse.pulse)
                {
                    mstate[qp.module] |= (1U << qp.index);
                    if (p2state && qp.module == p2state->module)
                        p2state->value = mstate[qp.module];
                }
                else
                {
                    mstate[qp.module] &= ~(1U << qp.index);
                }

                const uint8_t value = ((mstate[qp.module] - (uint16_t)((1U << modules[qp.module].num_inputs) - 1)) >> 15) & 1;
                for (int i = 0; i < 16 && modules[qp.module].outputs[i].module; ++i)
                    queue[qidxw++] = (mpulse) { .port = modules[qp.module].outputs[i], .pulse = value };
                break;
            }
        }
    }
}

static inline FORCEINLINE uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b != 0)
    {
        a %= b;
        a ^= b;
        b ^= a;
        a ^= b;
    }
    return a;
}

static inline uint64_t lcm(const uint64_t* a, size_t size)
{
    if (size > 2)
    {
        uint64_t b = lcm(a + 1, size - 1);
        return a[0] * b / gcd(a[0], b);
    }
    else
    {
        return a[0] * a[1] / gcd(a[0], a[1]);
    }
}

int main(int argc, char** argv)
{
    DSTOPWATCH_START(part1);

    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    register const chartype* data = file.data;
    const chartype* const end = file.data + fileSize;

    uint64_t sum1 = 0, sum2 = 0;

    module modules[32*32] = {0};
    uint16_t rxinput = 0;

    while (data < end)
    {
        chartype c1 = *data;
        module m = {0};
        uint16_t midx;
        uint8_t moutcount = 0;
        switch (c1)
        {
            case '%':
                m.type = M_FLIPFLOP;
                goto parse_input_name;
            case '&':
                m.type = M_CONJUNCTION;
            parse_input_name:
                midx = ((*++data & 0x1F) << 5) | ((*++data) & 0x1F);
                data += 3; // 'N -'
                break;
            case 'b':
                m.type = M_BROADCAST;
                midx = MIDX_BROADCAST;
                data += 13; // 'broadcaster -'
                // ...
                break;
        }
        while (*data != '\n')
        {
            data += 2; // '> ' or ', '
            uint16_t outidx = ((*data++ & 0x1F) << 5) | (*data++ & 0x1F);
            m.outputs[moutcount++].module = outidx;

            if (outidx == MIDX('r','x'))
                rxinput = midx;
        }
        modules[midx] = m;
        ++data;
    }

    for (int i = 0; i < 32*32; ++i)
    {
        if (modules[i].type)
        {
            for (int j = 0; j < 16 && modules[i].outputs[j].module; ++j)
                modules[i].outputs[j].index = modules[modules[i].outputs[j].module].num_inputs++;
        }
    }

#if defined(ENABLE_DEBUGLOG)
    for (int i = 0; i < 32*32; ++i)
    {
        if (modules[i].num_inputs || modules[i].outputs[0].module)
        {
            DEBUGLOG("module %c%c: type %s, num_inputs %u, outputs:", MICHARS(i), modnames[modules[i].type], modules[i].num_inputs);
            for (int j = 0; j < 16 && modules[i].outputs[j].module; ++j)
                DEBUGLOG(" %c%c[%u]", MICHARS(modules[i].outputs[j].module), modules[i].outputs[j].index);
            DEBUGLOG("\n");
        }
    }
#endif

    uint16_t mstate[32*32] = {0};

    uint64_t pulsecount[2] = {0};

    DEBUGLOG("rx input: %c%c\n", MICHARS(rxinput));
    p2state p2 = { .module = rxinput, .value = 0 };
    uint64_t p2period[16] = {0};

    size_t iter;
    for (iter = 0; iter < 1000; ++iter)
    {
        run(modules, mstate, pulsecount, &p2);
    }
    DEBUGLOG("lo: %lu, hi: %lu\n", pulsecount[0], pulsecount[1]);
    sum1 = pulsecount[0] * pulsecount[1];

    print_uint64(sum1);
    DSTOPWATCH_END(part1);
    DSTOPWATCH_START(part2);

    uint16_t p2icount = modules[rxinput].num_inputs;
    while (true)
    {
        run(modules, mstate, pulsecount, &p2);

        if (p2.value)
        {
            const uint16_t idx = __builtin_ctz(p2.value);
            if (!p2period[idx])
            {
                p2period[idx] = iter + 1;
                if (--p2icount == 0)
                    break;
            }
            p2.value = 0;
        }
        ++iter;
    }

    sum2 = lcm(p2period, modules[rxinput].num_inputs);

    print_uint64(sum2);
    DSTOPWATCH_END(part2);

    DSTOPWATCH_PRINT(part1);
    DSTOPWATCH_PRINT(part2);

    return 0;
}

