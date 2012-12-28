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

// Memory object
typedef struct
{
	meta_t metadata;	// Address, type
	union
	{
		char	*string;
		long	integer;
		double	fp;
		void	*data;
	} value;
} object;

// Stack layer (linked list member)
struct layer_t_st
{
	meta_t metadata;
	struct layer_t_st *next;
};

typedef struct layer_t_st layer_t;

// Linked-list member, stack (name)
struct scopestack_t_st
{
	char *name;
	layer_t *head;	// Head of the scope stack
	struct scopestack_t_st *next;
};

typedef struct scopestack_t_st scopestack_t;

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
	scopestack_t *first;
	scopestack_t *last;
} bucket_t;


#define META_ADDR 	20
#define META_TYPE	12
#define META_POIN	4

#define META_BITS_ADDR	0xfff00000
#define META_BITS_TYPE	0x000ff000
#define META_BITS_POIN	0x00000ff0
#define META_BITS_FLAG	0x0000000f

#define TYPE_STRING	1
#define TYPE_INT	2
#define TYPE_FP		4

#define TYPE_POINT	8
#define TYPE_LIST	16

#define FLAG_SET	1

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


object memory[MEMROWS];

unsigned int newmem;			// Lowest mem row with no value

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

/*	These support multiple columns in each memory row, not needed right now
meta_t getnewestobject(meta_t row)
{
	if ( ! getset(memory[row][0].metadata)) return 0;

	int i;
	for (i = 0; i < MEMCOLS; i++)
		if ( ! getset(memory[row][i].metadata))
			return (i - 1);
}

object lookup(entry_t key)
{
	return memory[geta(key.metadata)]
	  [getnewestobject(geta(key.metadata))];
}*/

int lookup_bucket(char *name)
{
	unsigned int key = SuperFastHash(name, strlen(name));
	int i = 0;

	// Look for the bucket with a matching hash.
	// If not found, return the current index.
	for(i; i < newbucket; i++)
		if (hashtable[i].hash == key)
			break;

	// Set the found bucket (whether it had a hash before or not)
	// to our name's hash.
	hashtable[i].hash = key;

	return i;
}

meta_t lookup_entry(char * name, int bucket)
{
	if (hashtable[bucket].first == SENTINEL)
	{
		return 0;
	}

	scopestack_t * cur = hashtable[bucket].first;

	while (strcmp(name, cur->name))
	{
		cur = cur->next;
		if (cur == SENTINEL)
			break;		
	}

	return cur->head->metadata;
}

void push_scope(scopestack_t * entry, meta_t data)
{
	layer_t *newhead = malloc(sizeof(layer_t));

	newhead->metadata = data;

	newhead->next = entry->head;

	entry->head = newhead;
}

meta_t pop_scope(scopestack_t *entry)
{
	layer_t *oldhead = entry->head;

	meta_t data = oldhead->metadata;

	entry->head = oldhead->next;

	free(oldhead);

	return data;
}


void addobject(char * content) 
{
	object newobject;

	char type;

	long longval;

	longval = strtol(content, &content, 0);

	if (content[0] == '\0')
	{
		getint(newobject) = longval;
		type = TYPE_INT;
	}
	else if (strchr(content, '"'))
	{
		char * newval = malloc(sizeof(content));

		memcpy(newval, content+1, strlen(content)-2);

		getstr(newobject) = newval;

		type = TYPE_STRING;
	}

	newobject.metadata = 
			makemetadata(
				newmem, 
				type,
				1,
				FLAG_SET);

	memory[newmem] = newobject;
}

/* Takes the name, pointed-to address, and type of a variable entry
	and returns either the address of the found entry or the newly-added one.
	Also adds the object in 'content' if the entry doesn't already exist.
*/
meta_t scanoradd(char * string,		// Var name
		   meta_t type,					// Type of variable (POINT)
		   char * content)				// Object content
{
	int bucket = lookup_bucket(string);

	meta_t entry = lookup_entry(string, bucket);

	if (entry == 0) // Not found...
	{
		// Allocate new stack for the new name
		scopestack_t *newentry = malloc(sizeof(scopestack_t));
		// Allocate new space for this particular instance of the name
		layer_t *newscope = malloc(sizeof(layer_t));

		newentry->name = string;

		newentry->head = newscope;
		newentry->head->metadata = 
			makemetadata(newmem, type, 1, FLAG_SET);
		newentry->head->next = SENTINEL;
		
		newentry->next = SENTINEL;

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

		addobject(content);

		newmem++; newbucket++;

		return newentry->head->metadata;
	}

	return entry;
}



int main(int argc, char **argv)
{
	memset(hashtable, 0, BUCKETS * sizeof(bucket_t));


	//char *varname	= malloc(50);
	//char *val	= malloc(50);
	
	unsigned int val_addr  = 0;

	meta_t hash_entry;

	object foundobj;

	hash_entry = scanoradd("a", TYPE_INT, "123");
	foundobj = memory[geta(hash_entry)];
	printf("(a -> %d) := ",  
		geta(hash_entry));
	printval(memory[geta(hash_entry)]);
	putchar('\n');

	hash_entry = scanoradd("b", TYPE_STRING, "\"123\"");
	foundobj = memory[geta(hash_entry)];
	printf("(b -> %d) := ",  
		geta(hash_entry));	
	printval(memory[geta(hash_entry)]);
	putchar('\n');

	/*printf("Enter a word to add it\n");
	while (1)
	{
		printf("%d>> ", newmem);
		scanf("%s %s", varname, val);
		putchar('\n');

		hash_entry = scanoradd(varname,
				  	 TYPE_POINT,
				  	 val);

		foundobj = memory[geta(hash_entry)];

		printf("(%s -> %d) := ", 
			varname, 
			geta(hash_entry));

		printval(foundobj);
		putchar('\n');
		putchar('\n');
	}*/

	return 0;
}
