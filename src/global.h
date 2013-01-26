#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <stdint.h> 
#include <stdio.h>
#include <string.h>

#ifdef BUILDING_JDAD_DLL
#	define JDAD_DLL  __declspec(dllexport)
#else
#	define JDAD_DLL  __declspec(dllimport) 
#endif



#define array_length(arr) sizeof(arr)/sizeof(*arr)

//#ifdef _WIN32 || _WIN64
//	typedef unsigned int jchar;
//#else
#define jchar char

#define _s(str) (jchar *)str
//#endif

extern int malloc_c;
extern size_t mem_c;
extern int free_c;

extern jchar bbuf[128];
extern int bbuf_len;

enum Truthval_E { False, True, Null };
typedef enum Truthval_E truthval_t;

#define TRUE 1
#define FALSE 0

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

void JDAD_DLL *c_malloc(size_t);
void JDAD_DLL *c_calloc(size_t, size_t);
void JDAD_DLL c_free(void *);

unsigned int jstrlen(jchar *);
double jstrtod(jchar *, jchar **);
long jstrtol(jchar *, jchar **, unsigned int);

void bputc(int);
void bflush(void);

#ifdef __cplusplus
}
#endif

/* GLOBAL_H */
#endif 