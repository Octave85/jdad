#include "types.h"

int malloc_c = 0;
size_t mem_c = 0;

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


thing_t * new_scal(char *stringval, type_t type)
{
	thing_t *newscal = c_malloc(sizeof(thing_t));
	newscal->type = Scalar;
	newscal->scal.type = type;
	newscal->scal.stringval = stringval;

	return newscal;
}

thing_t * new_arr(unsigned int maxlength)
{
	thing_t *newarr = c_malloc(sizeof(thing_t));
	
	newarr->type      = Array;
	newarr->arr.length    = 0;
	newarr->arr.maxlength = maxlength;
	newarr->arr.c         = c_calloc(maxlength, sizeof(thing_t));

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
	thing_t *newobj = c_malloc(sizeof(thing_t));

	if (newobj)
	{
		newobj->type  		= Object;
		newobj->obj.length	= 0;	// Current number of k/v pairs
		newobj->obj.keys 	= c_calloc(length, sizeof(thing_t));
		newobj->obj.vals 	= c_calloc(length, sizeof(thing_t));
		newobj->obj.ht   	= new_hashtable(length);	// "full" number
	} 

	return newobj;
}

nament_t * addkv(thing_t *object, char *key, thing_t *value)
{
	// Add key to object's key table
	nament_t *newk = addentry(object->obj.ht, key);
	// Point key to value
	newk->first->val = value;
	// Append key to keyname list
	object->obj.keys[object->obj.length] = key;
	object->obj.vals[object->obj.length] = value;
	
	object->obj.length++;

	return newk;
}

thing_t * getobjval(thing_t *obj, char *key)
{
	nament_t *entry = getentry(obj->obj.ht, key);

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
	char *key;
	thing_t *value;

	printf("{ ");

	/* In progress: turn keys[] into an array instead of an LL
	** and add values to valuesp[] array. Change array values to 
	** **thing_t instead of llm_t */
	int i = 0;
	for (; i < obj->obj.length - 1; i++)
	{
		key = obj->obj.keys[i];
		value = obj->obj.vals[i];

		printf("\"%s\": ", key);
		print_thing(value);
		printf(", ");
	}

	printf("\"%s\": ", obj->obj.keys[i]);
	print_thing(obj->obj.vals[i]);

	printf(" }\n");
}

void print_arr(thing_t *arr)
{
	printf("[ ");
	int i;
	for (i = 0; i < arr->arr.length - 1; i++)
	{
		print_thing(arr->arr.c[i]);
		printf(", ");
	}

	print_thing(arr->arr.c[i]);

	printf(" ]");
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
	thing_t *value  = new_scal("1234", Doble);
	thing_t *value2 = new_scal("hey guise", String);

	// Populate the value's fields
	value->scal.number.doble = 1234.f;
	value->scal.number.exp = 0;

	// Add the key and value
	key = addkv(test, "key1", value);

	value2->scal.string = "hey guise";

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

	printf("MEM STATS:\n"
		   "Malloc/Callocs: %d\n"
		   "Bytes allocated: %d\n", malloc_c, mem_c);

	return 0;
}