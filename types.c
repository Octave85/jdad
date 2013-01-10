#include "types.h"
/*
struct scalar_t_st {	// Scalar: string, double, or truthval
	union {
		char *string;
		struct {
			double 	doble;
			int		exp;
		} number;
		truthval_t truthval;
	};
	char *stringval;
	type_t type;
};
typedef struct scalar_t_st scalar_t;

struct object_t_st { 	// Key-value dictionary
	unsigned int length;
	llm_t *first;
	llm_t *last;
	hashtable_t *keys;
	type_t type;
};
typedef struct object_t_st object_t;

struct array_t_st {		// Ordered, numerically-indexed array
	unsigned int length;	// Current length
	unsigned int maxlength; // Max possible length
	llm_t **c;			// Contents: dynamic array of pointers to llm_t's
	type_t type;
};
typedef struct array_t_st array_t;
*/

llm_t * new_llm(void *value, llm_t *next)
{
	llm_t *newllm = malloc(sizeof(llm_t));

	if (newllm)
	{
		newllm->data = value;
		newllm->next = next;
	}

	return newllm;
}

/*thing_t * new_thing(void *value, type_t type)
{
	thing_t *newthing = malloc(sizeof(thing_t));

	switch (type)
	{
		case Scalar:
			newthing->scal = value;
			break;
		case Object:
			newthing->obj  = value;
			break;
		case Array:
			newthing->arr  = value;
			break;
	}

	newthing->type = type;

	return newthing;
}*/

thing_t * new_scal(char *stringval, type_t type)
{
	thing_t *newscal = malloc(sizeof(thing_t));
	newscal->type = Scalar;
	newscal->scal.type = type;
	newscal->scal.stringval = stringval;

	return newscal;
}

thing_t * new_arr(unsigned int maxlength)
{
	thing_t *newarr = malloc(sizeof(thing_t));
	
	newarr->type      = Array;
	newarr->arr.length    = 0;
	newarr->arr.maxlength = maxlength;
	newarr->arr.c         = calloc(maxlength, sizeof(thing_t));

	return newarr;
}

int addelem(thing_t *arr, int ind, thing_t *value)
{
	if (ind < 0)
		ind = arr->arr.length;

	if (++(arr->arr.length) > arr->arr.maxlength)
	{
		(arr->arr.length)--;
		return -1;
	}

	arr->arr.c[ind] = value;


	return ind;
}

thing_t * getarrval(thing_t *arr, unsigned int ind)
{
	if (ind < arr->arr.length)
		return arr->arr.c[ind];

	return NULL;
}


thing_t * new_obj(unsigned int length)
{
	thing_t *newobj = malloc(sizeof(thing_t));

	if (newobj)
	{
		newobj->type   = Object;
		newobj->obj.length = 0;	// Current number of k/v pairs
		newobj->obj.first  = newobj->obj.last = SENTINEL; // First and last key names
		newobj->obj.keys   = new_hashtable(length);	// "full" number
	} 

	return newobj;
}

nament_t * addkv(thing_t *object, char *key, void *value)
{
	// Add key to object's key table
	nament_t *newk = addentry(object->obj.keys, key);
	// Point key to value
	newk->first->val = value;
	// Append key to keyname list
	llm_t *newname = new_llm(key, SENTINEL);
	
	if (object->obj.last)
	{
		object->obj.last->next = newname;
	}
	else
	{
		object->obj.first = newname;
		object->obj.last  = newname;
	}
	
	object->obj.length++;

	return newk;
}

thing_t * getobjval(thing_t *obj, char *key)
{
	nament_t *entry = getentry(obj->obj.keys, key);

	if (entry)
	{
		return (thing_t *)entry->first->val;
	}

	return NULL;
}

void print_scalar(thing_t *sc)
{
	switch (sc->scal.type)
	{
		case Doble:
			printf("%.3f", sc->scal.number.doble);
			if (sc->scal.number.exp > 0)
			{
				putchar('e');
				printf("%d", sc->scal.number.exp);
			}
			break;
		case String:
			printf ("\"%s\"", sc->scal.string);
			break;
		case Truthval:
			switch (sc->scal.truthval)
			{
				case False:
					printf("False");
					break;
				case True:
					printf("True");
					break;
				case Null:
					printf("Null");
					break;
			}
			break;
	}
}

void print_obj(thing_t *obj)
{
	int i;

	llm_t *key;
	thing_t *value;

	printf("{\n");

	key = obj->obj.first;
	while (key != SENTINEL)
	{
		printf("%s: ", key->data);
		value = getobjval(obj, key->data);
		print_thing(value);
		putchar('\n');
		key = key->next;
	}

	printf("}\n");
}

void print_arr(thing_t *arr)
{
	printf("[");
	int i;
	for (i = 0; i < arr->arr.length - 1; i++)
	{
		print_thing(arr->arr.c[i]);
		printf(", ");
	}

	print_thing(arr->arr.c[i]);

	printf("]");
}

void print_thing(thing_t *thing)
{
	switch (thing->type)
	{
		case Scalar:
			print_scalar(thing);
			break;
		case Object:
			print_obj(thing);
			break;
		case Array:
			print_arr(thing);
			break;
	}
}

void TEST_obj(void)
{
	// Test: add some values to an object
	thing_t *test = new_obj(3);

	nament_t *key;
	thing_t *value = malloc(sizeof(thing_t));
	thing_t *value2 = malloc(sizeof(thing_t));

	// Populate the value's fields
	value->type = Scalar;
	value->scal.stringval = "1234";
	value->scal.number.doble = 1234.f;
	value->scal.number.exp = 0;
	value->scal.type = Doble;

	// Add the key and value
	key = addkv(test, "key1", value);

	value2->type = Scalar;
	value2->scal.stringval = "hey guise";
	value2->scal.string = "hey guise";
	value2->scal.type = String;

	key = addkv(test, "key2", value2);

	print_obj(test);

}

void TEST_arr(void)
{
	thing_t *arr = new_arr(4);

	thing_t *val1 = new_scal("hehe", String);
	thing_t *val2 = new_scal("nonono", String);

	val1->scal.string = "hehe";
	val2->scal.string = "nonono";

	addelem(arr, ARR_A, val1);
	addelem(arr, ARR_A, val2);
	print_thing(getarrval(arr, 0));
	print_thing(getarrval(arr, 1));

	// Nested array

	thing_t *val3 = new_arr(1);
	thing_t *val4 = new_scal("40", Doble);

	val4->scal.number.doble = 40.f;
	val4->scal.number.exp = 0;

	addelem(val3, ARR_A, val4);
	addelem(arr, ARR_A, val3);

	print_thing(arr);
}

int main(int argc, char **argv)
{
	TEST_obj();
	TEST_arr();
	printf("\n%d", sizeof(thing_t));
	putchar('\n');

	return 0;
}