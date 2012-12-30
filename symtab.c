/* String lookup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*** Stuff for Paul Hsieh's SuperFastHash function
**** This is licensed under LGPL 2.1.
***/
#include <stdint.h> /* Replace with <stdint.h> if appropriate */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash (const char * data, int len) {
uint32_t hash = len, tmp;
int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

/*** End SuperFastHash stuff ***/

#define MEMROWS 4096

#define BUCKETS 1024
#define NOADDR -1
#define SENTINEL NULL

typedef uint32_t meta_t;


/* Hash table is organized like this:
bucket_t[BUCKETS] hashtable		Full table contains: All entries

bucket_t hashtable[n] = 		Single element of hashtable has:
	nament_t[m] entries		All entries w/same hash value mod BUCKETS
								m = count of entries with that hash value
nament_t entry 				All entries with that same name
								HEAD of this will have the sought-after value.

In summary, the hashtable is an array of lists of lists. 
The final lists represent different uses (scopes) of that name, 
or symbols of different types with that name.
*/

// Stack layer (linked list member)
struct layer_t_st
{
	meta_t metadata;
	struct layer_t_st *next;
};

typedef struct layer_t_st layer_t;

// Linked-list member, stack (name)
struct nament_t_st
{
	char *name;
	layer_t *head;	// Head of the layer stack
	struct nament_t_st *next;
};

typedef struct nament_t_st nament_t;

// Independent entry - name and metadata only
typedef struct 
{
	char *name;
	meta_t metadata;
} entry_t;

// Linked-list container
typedef struct 
{
	unsigned int hash;
	nament_t *first;
	nament_t *last;
} bucket_t;


#define META_ADDR 	20
#define META_TYPE	12
#define META_POIN	4

#define META_BITS_ADDR	0xfff00000
#define META_BITS_TYPE	0x000ff000
#define META_BITS_POIN	0x00000ff0
#define META_BITS_FLAG	0x0000000f

#define TYPE_UNKNOWN 0
#define TYPE_STRING	 1
#define TYPE_INT	 2
#define TYPE_FP		 4

#define TYPE_POINT	 8
#define TYPE_LIST	 16

#define FLAG_SET	 1

/* Metadata format:
** 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
** a a a a a a a a a  a  a  a  t  t  t  t  t  t  t  t  p  p  p  p  p  p  p  p  f  f  f  f
** a: address
** t: type
** p: pointers
** f: flags
*/

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
#define getentry(str) (lookup_entry(str, lookup_bucket(str)))

// Function-making macro. Takes a condition ('cur' contains current layer)
// and a name for the function, and makes a function that loops through
// all the layers in a given name entry until condition is true.
#define findwherefun(cond, name) \
	layer_t * name(nament_t * nament)	 \
	{	\
		layer_t *cur = nament->head; 	 \
		while ( ! ((cur == SENTINEL) || (cond))) \
		{	\
			cur = cur->next;			 \
		}	\
		return cur;						 \
	}


bucket_t hashtable[BUCKETS];

unsigned int newbucket;			// Lowest bucket with no entry

void printval(object inquestion)
{
	switch (gett(inquestion.metadata))
	{
		case TYPE_INT:
			printf("%d", getint(inquestion));
			break;
		case TYPE_STRING:
			printf("%s", getstr(inquestion));
			break;
		case TYPE_FP:
			printf("%03f", getdoub(inquestion));
			break;
			
	}
}	

unsigned int lookup_bucket(char *name)
{
	return SuperFastHash(name, strlen(name)) % BUCKETS;
}

nament_t * lookup_entry(char * name, int bucket)
{
	if (hashtable[bucket].first == SENTINEL)
	{
		return SENTINEL;
	}

	nament_t * cur = hashtable[bucket].first;

	while (strcmp(name, cur->name))
	{
		cur = cur->next;
		if (cur == SENTINEL)
			break;		
	}

	return cur;
}

void push_layer(nament_t * entry, meta_t data)
{
	layer_t *newhead = malloc(sizeof(layer_t));

	newhead->metadata = data;

	newhead->next = entry->head;

	entry->head = newhead;
}

meta_t pop_layer(nament_t *entry)
{
	layer_t *oldhead = entry->head;

	meta_t data = oldhead->metadata;

	entry->head = oldhead->next;

	free(oldhead);

	return data;
}

void append_chain(nament_t * newentry, int bucket)
{
	// Check if this bucket has anything in it
	if (hashtable[bucket].first == SENTINEL)
	{
		hashtable[bucket].first = newentry;
	}
	else
	{
		(hashtable[bucket].last)->next = newentry;
	}

	hashtable[bucket].last = newentry;
}

meta_t addentry(char * string, meta_t type)
{
	int bucket = lookup_bucket(string);
	nament_t * entry = lookup_entry(string, bucket);

	if (entry == SENTINEL) // Not found...
	{
		// Allocate new stack for the new name
		nament_t *newentry = malloc(sizeof(nament_t));
		// Allocate new space for this particular instance of the name
		layer_t *newlayer = malloc(sizeof(layer_t));

		newlayer->metadata =
			makemetadata(newmem, type, 0, FLAG_SET);
		newlayer->next = SENTINEL;
		newentry->head = newlayer;

		newentry->name = string;
		
		newentry->next = SENTINEL;

		append_chain(newentry, bucket);

		newbucket++;
		return newlayer->metadata;
	}

	return 0;
}

/* Takes the name, pointed-to address, and type of a variable entry
	and returns either the address of the found entry or the newly-added one.
	Also adds the object in 'content' if the entry doesn't already exist.
*/
meta_t addwithval(char * string,	// Var name
		   meta_t type,				// Type of variable (POINT)
		   char * content)			// Object content
{
	addobject(content);
	return addentry(string, type);
}

findwherefun(gett(cur->metadata) == TYPE_STRING, findstr);

int main(int argc, char **argv)
{
	memset(hashtable, 0, BUCKETS * sizeof(bucket_t));
	
	meta_t hash_entry;
	nament_t *found_entry;
	
	hash_entry = addentry("a", TYPE_INT);
	found_entry = getentry("a");
	printf("Entering 'a' gives %d with type %d.\n" 
		   "Looking up 'a' gives %s with type %d.\n",
		hash_entry, TYPE_INT,
		found_entry->name, gett(found_entry->head->metadata));
	

	layer_t *found_layer = findstr(found_entry);

	if (found_layer != SENTINEL)
	{
		printf("Found %d, type %d\n", 
			found_layer->metadata,
			gett(found_layer->metadata));
	}
	else
		printf("Not found.\n");


	hash_entry = addentry("b", TYPE_STRING);
	found_entry = getentry("b");
	printf("Entering 'b' gives %d with type %d.\n" 
		   "Looking up 'b' gives %s with type %d.\n",
		hash_entry, TYPE_STRING,
		found_entry->name, gett(found_entry->head->metadata));

	return 0;
}
