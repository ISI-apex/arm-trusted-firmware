#ifndef A53_H
#define A53_H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define ASSERT(cond) \
{	assert(cond); \
	if (!(cond)) printf("%s: Assertion failed\n", __func__);};

// Define DEBUG to 1 in the source file that you want to debug
// before the #include statement for this header.
#ifndef DEBUG
#define DEBUG 0
#endif // undef DEBUG

#define DPRINTF(...) \
        if (DEBUG) printf(__VA_ARGS__)

void dump_buf(const char *name, uint32_t *buf, unsigned words);

#endif // A53_H
