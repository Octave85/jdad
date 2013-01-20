#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <stdint.h> 
#include <stdio.h>
#include <string.h>

#define array_length(arr) sizeof(arr)/sizeof(*arr)

extern int malloc_c;
extern size_t mem_c;
extern int free_c;

extern char bbuf[128];
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

void *c_malloc(size_t);
void *c_calloc(size_t, size_t);
inline void c_free(void *);

void bputc(int);
void bflush(void);

/* GLOBAL_H */
#endif 