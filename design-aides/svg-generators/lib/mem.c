/*
 *  moto-design SGV utils.
 */

#define _GNU_SOURCE
#define _ISOC99_SOURCE

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "log.h"
#include "mem.h"

void *mem_alloc(size_t size)
{
	void *p = malloc(size);

	if (!p) {
		error("malloc %lu failed: %s.\n", (unsigned long)size,
			strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}
	memset(p, 0, size);
	return p;
}

void *mem_realloc(void *p, size_t size)
{
	void *n = realloc(p, size);

	if (!n) {
		error("realloc %lu failed: %s.\n", (unsigned long)size,
			strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}
	return n;
}

void mem_free(void *p)
{
	if (!p) {
		error("null free.\n");
		assert(0);
		exit(EXIT_FAILURE);
	}

	free(p);
}
