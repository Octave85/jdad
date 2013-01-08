#include "types.h"

struct llm_t_st {	// General linked list member
	void *data;
	struct llm_t_st *next;
};
typedef struct llm_t_st llm_t;

struct scalar_t_st {	// Scalar: string, double, or truthval
	union {
		char *string;
		double doble;
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
};
typedef struct object_t_st object_t;

struct array_t_st {		// Ordered, numerically-indexed array
	unsigned int length;
	unsigned int maxlength;
	llm_t *first;
	llm_t *last;
};
typedef struct array_t_st array_t;

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

array_t * new_arr(unsigned int maxlength)
{
	array_t *newarr = malloc(sizeof(array_t));
	newarr->length = 0;
	newarr->maxlength = maxlength;
	newarr->first = newarr->last = SENTINEL;

	return newarr;
}

int addelem(array_t *arr, int ind, void *value)
{
	// anymore than one past the length is a bounds error
	//if (ind > (arr->length + (1 && arr->length))) 
	//	return -1;

	if (ind < 0)
		ind = arr->length;

	if (ind == 0)
	{
		arr->first = new_llm(value, SENTINEL);
		arr->last  = arr->first;
	}

	llm_t *cur = arr->first;

	int i = 0;

	while (++i < ind)	// Stops one before ind to insert in its place
	{
		cur = cur->next;
	}

	cur->next = new_llm(value, cur->next);

	arr->length++;

	return ind;
}

void * getarrval(array_t *arr, unsigned int ind)
{
	llm_t *cur = arr->first;
	int i = 0;

	while(cur != SENTINEL && i++ < ind)	// Stops at ind to grab it
		cur = cur->next;


	return cur->data;
}


object_t * new_obj(unsigned int length)
{
	object_t *newobj = malloc(sizeof(object_t));

	if (newobj)
	{
		newobj->length = 0;	// Current number of k/v pairs
		newobj->first = newobj->last = SENTINEL; // First and last key names
		newobj->keys = new_hashtable(length);	// "full" number
	} 

	return newobj;
}

nament_t * addkv(object_t *object, char *key, void *value)
{
	// Add key to object's key table
	nament_t *newk = addentry(object->keys, key);
	// Point key to value
	newk->first->val = value;
	// Append key to keyname list
	llm_t *newname = new_llm(key, SENTINEL);
	
	if (object->last)
	{
		object->last->next = newname;
	}
	else
	{
		object->first = newname;
		object->last  = newname;
	}
	
	object->length++;

	return newk;
}

void * getobjval(object_t *obj, char *key)
{
	nament_t *entry = getentry(obj->keys, key);

	if (entry)
	{
		return entry->first->val;
	}

	return NULL;
}

void print_scalar(scalar_t *sc)
{
	switch (sc->type)
	{
		case Doble:
			printf("%.3f", sc->doble);
			break;
		case String:
			printf ("\"%s\"", sc->string);
			break;
		case Truthval:
			switch (sc->truthval)
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

void print_obj(object_t *object)
{
	int i;

	llm_t *key;
	scalar_t *value;

	printf("{\n");

	key = object->first;
	while (key != SENTINEL)
	{
		printf("%s: ", key->data);
		value = getobjval(object, key->data);
		print_scalar(value);
		putchar('\n');
		key = key->next;
	}

	printf("}\n");
}

void print_arr(array_t *array)
{
	llm_t *cur = array->first;

	printf("[\n");
	while (cur != SENTINEL)
	{
		print_scalar(cur->data);
		printf(", ");
	}
	printf("]\n");
}

void TEST_obj(void)
{
	// Test: add some values to an object
	object_t *test = new_obj(3);

	nament_t *key;
	scalar_t *value = malloc(sizeof(scalar_t));
	scalar_t *value2 = malloc(sizeof(scalar_t));

	// Populate the value's fields
	value->type = Doble;
	value->stringval = "1234";
	value->doble = 1234.f;

	// Add the key and value
	key = addkv(test, "key1", value);

	value2->type = String;
	value2->stringval = "hey guise";
	value2->string = "hey guise";

	key = addkv(test, "key2", value2);

	print_obj(test);

}

void TEST_arr(void)
{
	array_t *arr = new_arr(4);

	scalar_t *val1 = malloc(sizeof(scalar_t));
	scalar_t *val2 = malloc(sizeof(scalar_t));

	val1->type = String;
	val1->stringval = "hehe";
	val1->string = "hehe";

	val2->type = String;
	val2->stringval = "nonono";
	val2->string = "nonono";

	addelem(arr, -1, val1);
	addelem(arr, -1, val2);
	print_scalar(getarrval(arr, 0));
	print_scalar(getarrval(arr, 1));
	//print_scalar(getarrval(arr, addelem(arr, -1, val1)));
	//print_scalar(getarrval(arr, addelem(arr, -1, val2)));

	//print_arr(arr);
}

int main(int argc, char **argv)
{
	TEST_obj();
	TEST_arr();

	return 0;
}