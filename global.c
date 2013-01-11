#include "global.h"

void *c_malloc(size_t size)
{
	malloc_c++;
	mem_c += size;

	return malloc(size);
}

void *c_calloc(size_t count, size_t size)
{
	malloc_c++;
	mem_c += count*size;

	return calloc(count, size);
}