#ifndef TYPES_H
#define TYPES_H

#include <string.h>
#include "symtab.h"
#include "global.h"

#define ARR_A -1

// Property-access macros for the three types
#define oa(t, p) t->p
#define aa(t, p) t->p
#define sa(t, p) t->p

#define ATOM_UNION char *string; \
	struct { 	\
		double 	doble;	\
		int		exp;	\
	} number;	\
	truthval_t truthval;

typedef enum { String, Doble, Truthval, Object, Array, Scalar, Thing } type_t;

struct llm_t_st {	// General linked list member/data container.
	void *data;
	struct llm_t_st *next;
};
typedef struct llm_t_st llm_t;

union atom_t_st {
	ATOM_UNION;
};
typedef union atom_t_st atom_t;

struct thing_t_st {
	union {
		
		struct {	/* Scalar */
			union {
				ATOM_UNION;
			};
			char *stringval;
			type_t stype;
		};
		
		struct {	/* Object */
			unsigned int olen;
			char 			  **keys;
			struct thing_t_st **vals;
			hashtable_t  	  *ht;
		};
		
		struct {	/* Array */
			unsigned int alen;	// Current length
			unsigned int maxlength; // Max possible length
			struct thing_t_st **c;	// Contents: dynamic array of pointers to llm_t's
		};
	};

	type_t type;	/* Type */
};

typedef struct thing_t_st thing_t;

thing_t * new_scal(char *, type_t);
void del_scal(thing_t *);
thing_t * new_arr(unsigned int);
int addelem(thing_t *, int, thing_t *);
thing_t * getarrval(thing_t *, unsigned int);
thing_t * new_obj(unsigned int);
nament_t * addkv(thing_t *, char *, thing_t *);
thing_t * getobjval(thing_t *, char *);
void print_scalar(thing_t *);
void print_arr(thing_t *);
void print_obj(thing_t *);
void print_thing(thing_t *);


/* TYPES_H */
#endif