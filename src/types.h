#ifndef TYPES_H
#define TYPES_H

#include <limits.h>
#include <float.h>
#include <errno.h>
#include <string.h>
#include "global.h"

#ifndef DECIMAL_DIG
 /* Approximates decimal-binary digit ratio */
#define DECIMAL_DIG (sizeof(double)*8 / 3) 
#endif

#define ARR_A -1
#define HT_THRESH 257

// Property-access macros for the three types
#define oa(t, p) t->p
#define aa(t, p) t->p
#define sa(t, p) t->p

#define ATOM_UNION 

typedef enum { Object, Array, Scalar   } type_t;	// Thing type

typedef enum { String, Doble, Integer, BigInt, BigDob, Truthval } stype_t;	// Scalar type

struct llm_t_st {	// General linked list member/data container.
	void *data;
	struct llm_t_st *next;
};
typedef struct llm_t_st llm_t;


/* Number in scientific notation. */
struct number_t {
	union {
		double doble;
		long integer;
	};
	int exponent;
};

/*  The main data type, thing_t, represents any JSON value.
**  It's implemented as an anonymous union of anonymous structs 
**  which lets us refer to its fields directly. The fields used are
**  based on its type. Type is the only member outside the outermost union.
*/
struct thing_t_st {
	union {
		
		struct {	/* Scalar */
			union {
				jchar *string;
				struct number_t number;
				truthval_t truthval;			
			};
			
			jchar *stringval;
			stype_t stype;
			unsigned int len;
		};
		
		struct {	/* Object */
			unsigned int olen;
			llm_t *key_first;	// Data type: pair_t *
			llm_t *key_last;
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
	jchar *key;
	thing_t *val;
};
typedef struct pair_t_st pair_t;


#ifdef __cplusplus
extern "C" {
#endif // Prevent name-mangling

llm_t *new_llm(void *, llm_t *);

thing_t * JDAD_DLL new_scal(stype_t);
thing_t * JDAD_DLL new_string(jchar *);
thing_t * JDAD_DLL new_doble(double, int);
thing_t * JDAD_DLL new_integer(long, int);
thing_t * JDAD_DLL new_bignum(jchar *);
thing_t * JDAD_DLL new_truthval(truthval_t);
void JDAD_DLL del_scal(thing_t *);

thing_t * JDAD_DLL new_arr(unsigned int);
int JDAD_DLL addelem(thing_t *, thing_t *);
thing_t * JDAD_DLL getarrval(thing_t *, unsigned int);
void JDAD_DLL del_arr(thing_t *);

thing_t * JDAD_DLL new_obj(unsigned int);
pair_t * JDAD_DLL addkv(thing_t *, pair_t *);
pair_t * JDAD_DLL getobjval(thing_t *, jchar *);
pair_t * JDAD_DLL new_pair(jchar *, thing_t *);
void JDAD_DLL del_obj(thing_t *);

void JDAD_DLL del_thing(thing_t *);

#ifdef __cplusplus
}
#endif

/* TYPES_H */
#endif