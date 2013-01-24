#ifndef PARSE_H
#define PARSE_H

#include "scan.h"
#include "print.h"

#define copy_sansquotes(str) new_copy(str + sizeof(jchar), jstrlen(str) - 2 * sizeof(jchar))

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

#ifdef __cplusplus
extern "C" {
#endif // Prevent name-mangling

parser_t * JDAD_DLL new_parser(jchar *);
int JDAD_DLL parse(parser_t *);
parser_t * JDAD_DLL parser_reopen(parser_t *, jchar *);
void JDAD_DLL parser_quit(parser_t *);

thing_t * thing(parser_t *);
thing_t * object(parser_t *);
thing_t * string(parser_t *);
thing_t * doble(parser_t *);

#ifdef __cplusplus
}
#endif

/* PARSE_H */
#endif