#include "global.h"

/*
void inline *c_malloc(size_t size)
{	
	return malloc(size);
}

void inline *c_calloc(size_t count, size_t size)
{
	return calloc(count, size);
}

void inline c_free(void *mem)
{
	free(mem);
}*/

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
