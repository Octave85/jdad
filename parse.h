#ifndef PARSE_H
#define PARSE_H

#include "scan.h"

#define OBJ_GUESS 5

typedef enum {
	Thing,
	Doble,
	String,
	ObjKey,
	ObjVal,
	Arr,
} pstate_t;

typedef struct {
	scanner_t *scan;
	token_t la;
	pstate_t state;
	thing_t **data;
} parser_t;

struct node_t_st {
	thing_t *thing;
	struct node_t_st *next;
}; 

typedef struct node_t_st node_t;

thing_t * thing(parser_t *);
thing_t * object(parser_t *);
thing_t * string(parser_t *);
thing_t * doble(parser_t *);

/* PARSE_H */
#endif