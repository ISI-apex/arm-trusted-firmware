#define DEBUG 0

#include <stdio.h>

#include "sleep.h"

#define MAX_TIME ~0

static volatile unsigned busyloop_factor = 0;

#if CONFIG_SLEEP_TIMER
static volatile unsigned time = 0; // cycles @ clk Hz
static volatile unsigned clk = 0;
#endif // CONFIG_SLEEP_TIMER

void sleep_set_busyloop_factor(unsigned f)
{
    VERBOSE("SLEEP: busyloop factor <- %u\r\n", f);
    busyloop_factor = f;
}

#if CONFIG_SLEEP_TIMER
void sleep_set_clock(unsigned f)
{
    VERBOSE("SLEEP: clock <- %u Hz\r\n", f);
    clk = f;
}

// Call this from a timer ISR
void sleep_tick(unsigned delta_cycles)
{
    time += delta_cycles;
    DPRINTF("SLEEP: time += %u -> %u\r\n", delta_cycles, time);
}
void msleep(unsigned ms)
{
    ASSERT(clk && "sleep clk not set");

    unsigned c = ms * (clk / 1000); // msec to cycles

    unsigned wakeup = time + c; // % MAX_TIME implicitly
    DPRINTF("time %u, wakeup %u\r\n", time, wakeup);
    int remaining = wakeup >= time ? wakeup - time : (MAX_TIME - time) + wakeup;
    unsigned last_tick = time;
    while (remaining > 0) {
	DPRINTF("SLEEP: sleeping for %u cycles...\r\n", remaining);
        asm("wfi"); // TODO: change to WFE and add SEV to sleep_tick

	unsigned elapsed = time >= last_tick ? time - last_tick : (MAX_TIME - last_tick) + time;
        last_tick = time;
        remaining -= elapsed;
    }
    DPRINTF("SLEEP: awake\r\n");
}
#endif // CONFIG_SLEEP_TIMER

void mdelay(unsigned ms)
{
    // WARNING: don't add any VERBOSE statements into this functions, they
    // interfere with the timing, despite being outside of the busyloop itself.
    unsigned iters = ms * busyloop_factor;

    if (iters == 0)
        return;

    // Inline assembly, to avoid non-determinism (or total brakage) by
    // compiler optimizations. This asm is portable across armv7 and
    // aarch32, but won't necessary take the same time, so each CPU will
    // need its own calibration factor anyway (see above).
    asm (
     "delayloop:\n"
     "  nop\n"
     "  sub %0, %0, #1\n"
     "  cmp %0, #0\n"
     "  bne delayloop\n"
     : : "r" (iters) :);
}
