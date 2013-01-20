#include "types.h"

char bbuf[128] = "";
int bbuf_len = 0;

llm_t * new_llm(void *value, llm_t *next)
{
	llm_t *newllm = c_malloc(sizeof(llm_t));

	if (newllm)
	{
		newllm->data = value;
		newllm->next = next;
	}

	return newllm;
}

pair_t * new_pair(char *key, thing_t *val)
{
	pair_t *newpair = c_malloc(sizeof(pair_t));

	if (newpair)
	{
		newpair->key = key;
		newpair->val = val;
	}

	return newpair;
}

llm_t * addback(llm_t *last, void *add)
{
	llm_t *newllm = new_llm(add, NULL);
	
	if (newllm == NULL) return last;
		
	if (last == NULL) 
		last = newllm;
	else
		last->next = newllm;

	return newllm;
}

thing_t * new_scal(char *stringval, stype_t type)
{
	thing_t *newscal = (thing_t *)c_malloc(sizeof(thing_t));
	newscal->type = Scalar;
	sa(newscal, stype) = type;
	sa(newscal, stringval) = stringval;

	return newscal;
}

void del_scal(thing_t *scal)
{
	/* Problem: these may or may not be malloc'd(),
	** so we can get a segfault by freeing them.
	*/
	//if (sa(scal, stype) == String)
	//{
	//	c_free(sa(scal, string));
	//}

	//c_free(sa(scal, stringval));

	c_free(scal);
}

thing_t * new_arr(unsigned int maxlength)
{
	thing_t *newarr = c_malloc(sizeof(thing_t));
	
	aa(newarr, type)      = Array;
	aa(newarr, alen)      = 0;
	aa(newarr, maxlength) = maxlength;
	aa(newarr, c_first)	= NULL;
	aa(newarr, c_last) = NULL;

	return newarr;
}

int addelem(thing_t *arr, thing_t *value)
{
	aa(arr, c_last) = addback(oa(arr, c_last), value);

	if (aa(arr, c_first) == NULL)
		aa(arr, c_first) = aa(arr, c_last);

	aa(arr, alen)++;

	return 1;
}

thing_t * getarrval(thing_t *arr, unsigned int ind)
{
	llm_t *cur = oa(arr, c_first);

	while (cur != NULL && ind--)
		cur = cur->next;

	return (thing_t *)cur->data;
}

void del_arr(thing_t *arr)
{
	llm_t *lm = aa(arr, c_first), *nx;
	thing_t *da;

	while (lm != NULL)
	{
		da = (thing_t *)lm->data;
		del_thing(da);

		nx = lm->next;
		c_free(lm);
		lm = nx;
	}

	c_free(arr);
}


thing_t * new_obj(unsigned int length)
{
	thing_t *newobj = c_malloc(sizeof(thing_t));

	if (newobj)
	{
		oa(newobj, type) 	= Object;
		oa(newobj, olen)	= 0;	// Current number of k/v pairs
		oa(newobj, ht)   	= NULL;
		oa(newobj, useht)	= 0;
		oa(newobj, key_first) = NULL;
		oa(newobj, key_last) = NULL;
	} 

	return newobj;
}

pair_t * addkv(thing_t *object, pair_t *pair)
{
	if (oa(object, useht))
	{
		// Add key to object's key table
		nament_t *newk = addentry(oa(object, ht), pair->key);
		// Point key to value
		newk->first->val = pair->val;
	}

	oa(object, key_last) = addback(oa(object, key_last), pair);
	
	if (oa(object, key_first) == NULL)
	{
		oa(object, key_first) = oa(object, key_last);
	}

	oa(object, olen)++;

	return pair;
}

pair_t * getobjval(thing_t *obj, char *key)
{
	/*if (oa(obj, useht))
	{
		nament_t *entry = getentry(oa(obj, ht), key);

		if (entry)
		{
			return (thing_t *)entry->first->val;
		}

		return NULL;
	}*/

	llm_t *key_list = oa(obj, key_first);

	pair_t *k = (pair_t *)key_list->data;

	while (key_list != NULL)
	{
		if ( ! strncmp(key, k->key, strlen(key)))
			break;

		key_list = key_list->next;
		k = (pair_t *)key_list->data;
	}

	return k;
}

void del_obj(thing_t *obj)
{
	llm_t *lm = oa(obj, key_first), *nx;
	
	while (lm != NULL)
	{
		pair_t *pa = (pair_t *)lm->data;
		del_thing(pa->val);
		c_free(pa);

		nx = lm->next;
		c_free(lm);
		
		lm = nx;
	}

	c_free(obj);
}

void del_thing(thing_t *t)
{
	switch (t->type)
	{
		case Scalar:
			del_scal(t);
			break;

		case Array:
			del_arr(t);
			break;

		case Object:
			del_obj(t);
			break;

		default:
			printf("Illegal type for deletion\n");
	}
}


void TEST_obj(void)
{
	// Test: add some values to an object
	thing_t *test = new_obj(3);

	thing_t *value  = new_scal("1234", Doble);
	thing_t *value2 = new_scal("hey guise", String);

	// Populate the value's fields
	sa(value, number).doble = 1234.f;
	sa(value, number).exp = 0;

	pair_t *key1 = new_pair("key1", value);

	// Add the key and value
	addkv(test, key1);

	sa(value2, string) = "hey guise";

	pair_t *key2 = new_pair("key2", value2);

	addkv(test, key2);

	unsigned int level = 0;
	print_obj(test, &level);

}

void TEST_arr(void)
{
	thing_t *arr = new_arr(4);

	thing_t *val1 = new_scal("hehe", String);
	thing_t *val2 = new_scal("nonono", String);

	sa(val1, string) = "hehe";
	sa(val2, string) = "nonono";

	addelem(arr, val1);
	addelem(arr, val2);

	// Nested array

	thing_t *val3 = new_arr(1);
	thing_t *val4 = new_scal("40", Doble);

	sa(val4, number).doble = 40.f;
	sa(val4, number).exp = 0;

	addelem(val3, val4);
	addelem(arr, val3);

	unsigned int level = 0;
	print_thing(arr, &level);
}
/*
int main(int argc, char **argv)
{
	TEST_obj();
	TEST_arr();
	printf("\n%d", sizeof(thing_t));
	putchar('\n');

	printf("MEM STATS:\n"
		   "Malloc/Callocs: %d\n"
		   "Bytes allocated: %d\n", malloc_c, mem_c);

	return 0;
}*/