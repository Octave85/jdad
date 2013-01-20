#ifndef TYPES_H
#define TYPES_H

#include <string.h>
#include "symtab.h"
#include "global.h"

#define ARR_A -1
#define HT_THRESH 257

// Property-access macros for the three types
#define oa(t, p) t->p
#define aa(t, p) t->p
#define sa(t, p) t->p
#define la(l, f, t) *(l).*((t)data).f

#define ATOM_UNION char *string; \
	struct { 	\
		double 	doble;	\
		int		exp;	\
	} number;	\
	truthval_t truthval;

typedef enum { Object, Array, Scalar   } type_t;	// Thing type

typedef enum { String, Doble, Truthval } stype_t;	// Scalar type

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
			stype_t stype;
		};
		
		struct {	/* Object */
			unsigned int olen;
			llm_t *key_first;	// Data type: pair_t *
			llm_t *key_last;
 
			hashtable_t *ht;
			unsigned int useht;
		};
		
		struct {	/* Array */
			unsigned int alen;	// Current length
			unsigned int maxlength; // Max possible length
			llm_t *c_first;	// Data type: thing_t *
			llm_t *c_last;	
		};
	};

	type_t type;	/* Type */
};

typedef struct thing_t_st thing_t;

struct pair_t_st {
	char *key;
	thing_t *val;
};
typedef struct pair_t_st pair_t;


llm_t *new_llm(void *, llm_t *);

thing_t * new_scal(char *, stype_t);
void del_scal(thing_t *);

thing_t * new_arr(unsigned int);
int addelem(thing_t *, thing_t *);
thing_t * getarrval(thing_t *, unsigned int);
void del_arr(thing_t *);

thing_t * new_obj(unsigned int);
pair_t * addkv(thing_t *, pair_t *);
pair_t * getobjval(thing_t *, char *);
pair_t * new_pair(char *, thing_t *);
void del_obj(thing_t *);

/* TYPES_H */
#endif