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
	thing_t *newscal = (thing_t *)c_malloc(sizeof(thing_t));
	newscal->type = Scalar;
	sa(newscal, stype) = type;
	sa(newscal, stringval) = stringval;

	return newscal;
}

void del_scal(thing_t *scal)
{
	if (scal->stype == String)
	{
		free(scal->string);
	}

	free(scal->stringval);

	free(scal);
}

thing_t * new_arr(unsigned int maxlength)
{
	thing_t *newarr = c_malloc(sizeof(thing_t));
	
	newarr->type      = Array;
	aa(newarr, alen)    = 0;
	aa(newarr, maxlength) = maxlength;
	aa(newarr, c)         = 
		(thing_t **)c_calloc(maxlength, sizeof(thing_t));

	return newarr;
}

int addelem(thing_t *arr, int ind, thing_t *value)
{
	if (ind < 0)
		ind = aa(arr, alen);

	if (++(aa(arr, alen)) > aa(arr, maxlength))
	{
		(aa(arr, alen))--;
		return -1;
	}

	aa(arr, c)[ind] = value;


	return ind;
}

thing_t * getarrval(thing_t *arr, unsigned int ind)
{
	if (ind < aa(arr, alen))
		return aa(arr, c)[ind];

	return NULL;
}

void del_arr(thing_t *arr)
{
	int i;

	for (i = 0; i < aa(arr, alen); i++)
	{
		//del_thing(aa(arr, c)[i]);
	}

	free(arr);
}


thing_t * new_obj(unsigned int length)
{
	thing_t *newobj = c_malloc(sizeof(thing_t));

	if (newobj)
	{
		newobj->type 		= Object;
		oa(newobj, olen)	= 0;	// Current number of k/v pairs
		oa(newobj, keys) 	= 
			(thing_t **)c_calloc(length, sizeof(thing_t));
		oa(newobj, vals) 	= 
			(thing_t **)c_calloc(length, sizeof(thing_t));
		oa(newobj, ht)   	= new_hashtable(length);	// "full" number
	} 

	return newobj;
}

nament_t * addkv(thing_t *object, char *key, thing_t *value)
{
	// Add key to object's key table
	nament_t *newk = addentry(oa(object, ht), key);
	// Point key to value
	newk->first->val = value;
	// Append key to keyname list
	oa(object, keys)[oa(object, olen)] = key;
	oa(object, vals)[oa(object, olen)] = value;
	
	oa(object, olen)++;

	return newk;
}

thing_t * getobjval(thing_t *obj, char *key)
{
	nament_t *entry = getentry(oa(obj, ht), key);

	if (entry)
	{
		return (thing_t *)entry->first->val;
	}

	return NULL;
}

void del_obj(thing_t *obj)
{
	int i;
	for (i = 0; i < oa(obj, olen); i++)
	{
		//del_thing(oa(obj, vals)[i]);
	}
}

void print_scalar(thing_t *sc)
{
	switch (sa(sc, stype))
	{
		case Doble:
			printf("%.3f", sa(sc, number).doble);
			if (sa(sc, number).exp > 0)
			{
				putchar('e');
				printf("%d", sa(sc, number).exp);
			}
			break;
		case String:
			printf ("\"%s\"", sa(sc, string));
			break;
		case Truthval:
			switch (sa(sc, truthval))
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
	for (; i < oa(obj, olen) - 1; i++)
	{
		key   = oa(obj, keys)[i];
		value = oa(obj, vals)[i];

		printf("\"%s\": ", key);
		print_thing(value);
		printf(", ");
	}

	printf("\"%s\": ", oa(obj, keys)[i]);
	print_thing(oa(obj, vals)[i]);

	printf(" }\n");
}

void print_arr(thing_t *arr)
{
	printf("[ ");
	int i;
	for (i = 0; i < aa(arr, olen) - 1; i++)
	{
		print_thing(aa(arr, c)[i]);
		printf(", ");
	}

	print_thing(aa(arr, c)[i]);

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
	sa(value, number).doble = 1234.f;
	sa(value, number).exp = 0;

	// Add the key and value
	key = addkv(test, "key1", value);

	sa(value2, string) = "hey guise";

	key = addkv(test, "key2", value2);

	print_obj(test);

}

void TEST_arr(void)
{
	thing_t *arr = new_arr(4);

	thing_t *val1 = new_scal("hehe", String);
	thing_t *val2 = new_scal("nonono", String);

	sa(val1, string) = "hehe";
	sa(val2, string) = "nonono";

	addelem(arr, ARR_A, val1);
	addelem(arr, ARR_A, val2);

	// Nested array

	thing_t *val3 = new_arr(1);
	thing_t *val4 = new_scal("40", Doble);

	sa(val4, number).doble = 40.f;
	sa(val4, number).exp = 0;

	addelem(val3, ARR_A, val4);
	addelem(arr, ARR_A, val3);

	print_thing(arr);
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