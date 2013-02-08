#include "global.h"

int malloc_c = 0, mem_c = 0, free_c = 0;

#ifdef MEM_DEBUG
void inline *c_malloc(size_t size)
{	
	malloc_c++;
	mem_c += size;
	return malloc(size);
}

void inline *c_calloc(size_t count, size_t size)
{
	malloc_c++;
	mem_c += size * count;
	return calloc(count, size);
}

void inline c_free(void *mem)
{
	free_c++;
	free(mem);
}

void JDAD_DLL print_mem_use(void)
{
	fprintf(stderr, "\nMEM STATS:\nMallocs: %d\nFrees: %d\nBytes: %d\n",
		malloc_c, free_c, mem_c);
}
#endif

unsigned int jstrlen(jchar *jstr)
{
	int len;

	for (len = 0; jstr[len] != 0; len++);

	return len;
}

/*
double inline jstrtod(jchar *jstr, jchar **rest)
{
	return strtod((const char *)jstr, rest);
}

long inline jstrtol(jchar *jstr, jchar **rest, unsigned int base)
{
	return strtol((const char *)jstr, rest, base);
}
*/
