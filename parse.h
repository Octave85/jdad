#ifndef PARSE_H
#define PARSE_H

#include "scan.h"
#include "print.h"

#define copy_sansquotes(str) new_copy(str + 1, strlen(str) - 2)

typedef enum {
	pThing,
	pDoble,
	pString,
	pObjKey,
	pObjVal,
	pArr,
} pstate_t;

typedef struct {
	scanner_t *scan;
	token_t la;
	pstate_t state;
	thing_t *data;
} parser_t;

struct node_t_st {
	thing_t *thing;
	struct node_t_st *next;
}; 

typedef struct node_t_st node_t;
parser_t * new_parser(char *);
int parse(parser_t *p);

thing_t * thing(parser_t *);
thing_t * object(parser_t *);
thing_t * string(parser_t *);
thing_t * doble(parser_t *);

/* PARSE_H */
#endif