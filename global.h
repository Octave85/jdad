#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <stdint.h> 
#include <stdio.h>

extern int malloc_c;
extern size_t mem_c;

enum Truthval_E { False, True, Null };
typedef enum Truthval_E truthval_t;

#define TRUE 1
#define FALSE 0

#define SENTINEL NULL

void *c_malloc(size_t size);

/* GLOBAL_H */
#endif 