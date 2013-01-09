#include "types.h"
#include "symtab.h"

/*** Stuff for Paul Hsieh's SuperFastHash function
**** This is licensed under LGPL 2.1.
***/
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


bucket_t hashtable[BUCKETS];

hashtable_t * new_hashtable(unsigned int buckets)
{
	hashtable_t *newtable = (hashtable_t *)malloc(sizeof(hashtable_t));
	newtable->length = buckets;
	newtable->b = (bucket_t *)calloc(buckets, sizeof(bucket_t));

	return newtable;
}

unsigned int lookup_bucket(hashtable_t *ht, char *name)
{
	return SuperFastHash(name, strlen(name)) % ht->length;
}

nament_t * lookup_entry(hashtable_t *ht,
	char *name, 
	unsigned int bucket)
{
	if (ht->b[bucket].first == SENTINEL)
	{
		return SENTINEL;
	}

	nament_t * cur = ht->b[bucket].first;


	while (strcmp(name, cur->name))
	{
		cur = cur->next;
		if (cur == SENTINEL)
			break;		
	}

	return cur;
}

truthval_t push_layer(nament_t * entry, meta_t data)
{
	layer_t *newhead = malloc(sizeof(layer_t));

	newhead->metadata = data;

	newhead->next = entry->first;

	entry->first = newhead;

	return True;
}

meta_t pop_layer(nament_t *entry)
{
	layer_t *oldhead = entry->first;

	meta_t data = oldhead->metadata;

	entry->first = oldhead->next;

	free(oldhead);

	return data;
}

void append_chain(hashtable_t *ht,
	nament_t * newentry, 
	int bucket)
{
	// Check if this bucket has anything in it
	if (ht->b[bucket].first == SENTINEL)
	{
		ht->b[bucket].first = newentry;
	}
	else
	{
		(ht->b[bucket].last)->next = newentry;
	}

	ht->b[bucket].last = newentry;
}

nament_t * addentry(hashtable_t *ht,
	char * string)
{
	int bucket = lookup_bucket(ht, string);
	nament_t * entry = lookup_entry(ht, string, bucket);

	if (entry == SENTINEL) // Not found...
	{
		// Allocate new stack for the new name
		nament_t *newentry = malloc(sizeof(nament_t));
		// Allocate new space for this particular instance of the name
		layer_t *newlayer = malloc(sizeof(layer_t));

		newlayer->metadata =
			makemetadata(0, 0, 0, FLAG_SET);
		newlayer->next = SENTINEL;
		newentry->first = newlayer;

		newentry->name = string;
		
		newentry->next = SENTINEL;

		append_chain(ht, newentry, bucket);

		return newentry;
	}

	return NULL;
}

truthval_t delnament(nament_t *inq)
{
	if (inq)
	{
		// Free every layer of this name
		layer_t *on = inq->first, *del;

		while (on != SENTINEL)
		{
			del = on;
			free(del);
			on = on->next;
		}

		free(inq);

		return True;
	}
	return False;
}

truthval_t delentry(hashtable_t *ht, char *name)
{
	nament_t *inq = getentry(ht, name);
	return delnament(inq);
}

findwherefun(gett(cur->metadata) == TYPE_STRING, findstr);

/*int main(int argc, char **argv)
{
	memset(hashtable, 0, BUCKETS * sizeof(bucket_t));
	
	meta_t md;
	nament_t *found_entry;
	
	found_entry = addentry(hashtable, "a", TYPE_INT);
	md = found_entry->first->metadata;
	printf("Entering 'a' gives %d with type %d.\n" 
		   "Looking up 'a' gives %s with type %d.\n",
		md, TYPE_INT,
		found_entry->name, gett(md));
	

	layer_t *found_layer = findstr(found_entry);

	if (found_layer != SENTINEL)
	{
		printf("Found %d, type %d\n", 
			found_layer->metadata,
			gett(found_layer->metadata));
	}
	else
		printf("Not found.\n");


	found_entry = addentry(hashtable, "b", TYPE_STRING);
	md = found_entry->first->metadata;
	printf("Entering 'b' gives %d with type %d.\n" 
		   "Looking up 'b' gives %s with type %d.\n",
		md, TYPE_STRING,
		found_entry->name, gett(md));

	printf("%d\n", sizeof(bucket_t));

	return 0;
}
*/