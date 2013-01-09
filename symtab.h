#ifndef SYMTAB_H
#define SYMTAB_H

#include "global.h"
#include "types.h"
#include <string.h>

#define BUCKETS 1024
#define NOADDR -1

#define TYPE_STRING 1
#define TYPE_INT 2


/* Metadata format:
** 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
** a a a a a a a a a  a  a  a  t  t  t  t  t  t  t  t  p  p  p  p  p  p  p  p  f  f  f  f
** a: address
** t: type
** p: pointers
** f: flags
*/
typedef uint32_t meta_t;


/* Hash table is organized like this:
bucket_t[BUCKETS] hashtable		Full table contains: All entries

bucket_t hashtable[n] = 		Single element of hashtable has:
	nament_t[m] entries		Linked list of all entries w/same hash value
								m = count of entries with that hash value
nament_t entry 				Linked list of all entries with that same name
								HEAD of this will have the sought-after value.

In summary, the hashtable is an array of lists of lists. 
The final lists represent different uses (scopes) of that name, 
or symbols of different types with that name.
*/

// Stack layer (linked list member)
struct layer_t_st
{
	meta_t metadata;
	void *val;
	struct layer_t_st *next;
};

typedef struct layer_t_st layer_t;

// Linked-list member, stack (name)
struct nament_t_st
{
	char *name;
	layer_t *first;	// Head of the layer stack
	struct nament_t_st *next;
};

typedef struct nament_t_st nament_t;

// Linked-list container
typedef struct 
{
	unsigned int hash;
	nament_t *first;
	nament_t *last;
} bucket_t;

struct hashtable_t_st
{
	unsigned int length;
	bucket_t *b;
};

typedef struct hashtable_t_st hashtable_t;

#define META_ADDR 	20
#define META_TYPE	12
#define META_POIN	4

#define META_BITS_ADDR	0xfff00000
#define META_BITS_TYPE	0x000ff000
#define META_BITS_POIN	0x00000ff0
#define META_BITS_FLAG	0x0000000f

/*#define TYPE_UNKNOWN 0
#define TYPE_STRING	 1
#define TYPE_INT	 2
#define TYPE_FP		 4

#define TYPE_POINT	 8
#define TYPE_LIST	 16*/

#define FLAG_SET	 1

/* Macros for metadata; 
** A bunch of bitshifting for accessing different values.
*/
#define makemetadata(addr, type, poin, flag) ((addr << META_ADDR) | (type << META_TYPE) | (poin << META_POIN) | flag)
#define geta(md) ((md & META_BITS_ADDR) >> META_ADDR)
#define gett(md) ((md & META_BITS_TYPE) >> META_TYPE)
#define getp(md) ((md & META_BITS_POIN) >> META_POIN)
#define getf(md) ((md & META_BITS_FLAG))
#define getset(md) (md & 0x1)
#define seta(md, addr) (md = (addr << META_ADDR) | (md & ~META_BITS_ADDR))
#define sett(md, type) (md = (type << META_TYPE) | (md & ~META_BITS_TYPE))
#define setp(md, poin) (md = (poin << META_POIN) | (md & ~META_BITS_POIN))
#define setf(md, flag) (md = (flag 			   ) | (md & ~META_BITS_FLAG))

#define getstr(ob) (ob.value.string)
#define getint(ob) (ob.value.integer)
#define getdoub(ob) (ob.value.fp)


// Macro for a hashtable lookup. 
// Returns a pointer to the name entry specified by str.
#define getentry(ht, str) (lookup_entry(ht, str, lookup_bucket(ht, str)))

// Function-making macro. Takes a condition ('cur' contains current layer)
// and a name for the function, and makes a function that loops through
// all the layers in a given name entry until condition is true.
#define findwherefun(cond, name) \
	layer_t * name(nament_t * nament)	 \
	{	\
		layer_t *cur = nament->first; 	 \
		while ( ! ((cur == SENTINEL) || (cond))) \
		{	\
			cur = cur->next;			 \
		}	\
		return cur;						 \
	}

/* Prototpyes */

unsigned int lookup_bucket(hashtable_t *, char *);
nament_t * lookup_entry(hashtable_t *, char *, unsigned int);

truthval_t push_layer(nament_t *, meta_t);
meta_t pop_layer(nament_t *);

nament_t * addentry(hashtable_t *, char *);
truthval_t delnament(nament_t *);
truthval_t delentry(hashtable_t *, char *);


/* SYMTAB_H */
#endif 