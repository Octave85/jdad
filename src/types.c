#include "types.h"

jchar bbuf[128] = "";
int bbuf_len = 0;

llm_t * new_llm(void *value, llm_t *next)
{
	llm_t *newllm = 
		(llm_t *)c_malloc(sizeof(llm_t));

	if (newllm)
	{
		newllm->data = value;
		newllm->next = next;
	}

	return newllm;
}

pair_t * new_pair(jchar *key, thing_t *val)
{
	pair_t *newpair = 
		(pair_t *)c_malloc(sizeof(pair_t));

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

thing_t * new_scal(jchar *stringval, stype_t type)
{
	thing_t *newscal = (thing_t *)c_malloc(sizeof(thing_t));
	newscal->type = Scalar;
	sa(newscal, stype) = type;
	sa(newscal, stringval) = stringval;

	return newscal;
}

thing_t * new_string(jchar *stringval)
{
	thing_t *newstr = new_scal(stringval, String);
	sa(newstr, string) = stringval;

	return newstr;
}

thing_t * new_doble(double doble, int exponent)
{
	jchar *stringval = (jchar *)c_malloc(64);
	thing_t *newdob = NULL;

	if (stringval != NULL)
	{
		snprintf(stringval, 64, "%.3fe%d", doble, exponent);
		newdob = new_scal(stringval, Doble);
		sa(newdob, number.doble) = doble;
		sa(newdob, number.exp  ) = exponent;
	}

	return newdob;
}

thing_t * new_truthval(truthval_t tv)
{
	jchar *stringval = (jchar *)c_malloc(6);
	thing_t *newtv = NULL;

	if (stringval != NULL)
	{
		snprintf(stringval, 6, "%s", tv2str[tv]);
		newtv = new_scal(stringval, Truthval);
		sa(newtv, truthval) = tv;
	}

	return newtv;
}

inline void del_scal(thing_t *scal)
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
	thing_t *newarr = 
		(thing_t *)c_malloc(sizeof(thing_t));
	
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
	thing_t *newobj = 
		(thing_t *)c_malloc(sizeof(thing_t));

	if (newobj)
	{
		oa(newobj, type) 	= Object;
		oa(newobj, olen)	= 0;	// Current number of k/v pairs
		oa(newobj, key_first) = NULL;
		oa(newobj, key_last) = NULL;
	} 

	return newobj;
}

pair_t * addkv(thing_t *object, pair_t *pair)
{
	oa(object, key_last) = addback(oa(object, key_last), pair);
	
	if (oa(object, key_first) == NULL)
	{
		oa(object, key_first) = oa(object, key_last);
	}

	oa(object, olen)++;

	return pair;
}

pair_t * getobjval(thing_t *obj, jchar *key)
{
	llm_t *key_list = oa(obj, key_first);

	pair_t *k = (pair_t *)key_list->data;

	while (key_list != NULL)
	{
		if ( ! strncmp(key, k->key, jstrlen(key)))
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