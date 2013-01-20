#include "global.h"

int malloc_c = 0;
size_t mem_c = 0;
int free_c   = 0;

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

inline void c_free(void *mem)
{
	free_c++;

	free(mem);
}

void bputc(int c)
{
	bbuf[bbuf_len++] = c;
}

void bflush(void)
{
	printf("%s", bbuf);
	memset(bbuf, 0, bbuf_len);
	bbuf_len = 0;
}