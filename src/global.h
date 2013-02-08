#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <stdint.h> 
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#ifdef BUILDING_JDAD_DLL
#	define JDAD_DLL  __declspec(dllexport)
#else
#	define JDAD_DLL  __declspec(dllimport) 
#endif
#else
#	define JDAD_DLL
#endif


#define array_length(arr) sizeof(arr)/sizeof(*arr)

#define jchar char

#define _s(str) (jchar *)str

enum Truthval_E { False, True, Null };
typedef enum Truthval_E truthval_t;

/* Used in print.c and types.c */
static const char tv2str[][6] = {
	"false", "true", "null",
};


#define SENTINEL NULL

typedef enum {
	eInternalMem,
	eInternal,
	eToken,
	eParse,
} error_t;

#ifdef __cplusplus
extern "C" {
#endif // Prevent name-mangling

#define MEM_DEBUG

#ifdef MEM_DEBUG

void *c_malloc(size_t);
void *c_calloc(size_t, size_t);
void c_free(void *);
void JDAD_DLL print_mem_use(void);

#else

#define c_malloc(sz) malloc(sz)
#define c_calloc(ct, sz) calloc(ct, sz)
#define c_free(bl) free(bl)

#endif

double jstrtod(jchar *, jchar **);
long jstrtol(jchar *, jchar **, unsigned int);


#define jstrtod(str, rest) strtod(str, rest)
#define jstrtol(str, rest, base) strtol(str, rest, base)

unsigned int jstrlen(jchar *);

#ifdef __cplusplus
}
#endif

/* GLOBAL_H */
#endif 