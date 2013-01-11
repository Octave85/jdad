#ifndef TYPES_H
#define TYPES_H

#include <string.h>
#include "symtab.h"
#include "global.h"

#define ARR_A -1

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
			type_t type;
		} scal;
		
		struct {	/* Object */
			unsigned int length;
			char 			  **keys;
			struct thing_t_st **vals;
			hashtable_t  	  *ht;
		} obj;
		
		struct {	/* Array */
			unsigned int length;	// Current length
			unsigned int maxlength; // Max possible length
			struct thing_t_st **c;	// Contents: dynamic array of pointers to llm_t's
		} arr;
	};

	type_t type;	/* Type */
};

typedef struct thing_t_st thing_t;


/* TYPES_H */
#endif