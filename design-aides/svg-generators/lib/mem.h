/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_MEM_H)
#define _MD_GENERATOR_MEM_H

#include <stdlib.h>

void *mem_alloc(size_t size);
void *mem_realloc(void *p, size_t size);
void mem_free(void *p);

#endif /* _MD_GENERATOR_MEM_H */
