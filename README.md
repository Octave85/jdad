# jdad

JSON done almost dangerously.

## Data Types
## type_t
Enum: Scalar, Object, Array. Thing type.
## stype_t
Enum: String, Doble (Double), Truthval. Scalar type.
## pair_t
Key-value pair for an object lookup. Fields: char *key, thing_t *val.
To construct: 

	pair_t *pa = new_pair((char *)key, (thing_t *_val);

## truthval_t
Enum: True, False, Null
## thing_t
An all-purpose type. 
Always has one field: type_t type. Either Scalar, Array, or Object.
Depending on type, its (used) fields should vary.
### del_thing
Recursively destroys a thing_t * instance.
	
	del_thing((thing_t *)t);

### Scalar
Access function: 

	sa(scal, field)

To construct: 

	thing_t *scal = new_scal((char *)stringval, (stype_t)stype);

Note that you must independently assign a value to one of the scalar's value fields, described below.
#### String
Field: 
	char *string.

#### Doble
Fields: 
	double doble, int exp.

#### Truthval
Fields: 

	truthval_t truthval.

### Array
Access function: 

	aa(arr, field)

To construct: 
	
	thing_t *arr = new_arr(maxlength); // maxlength means nothing right now

Arrays are variable length and grow to accomodate new entries.
Add elements to end:

	addelem(array, thing);

Index array:

	thing_t *val = getarrval(array, index);

### Object
Access function: oa(obj, field)
To construct: 

	thing_t *obj = new_obj(maxlength); // maxlength means nothing right now

Objects are also variable length and accomodate new key/value pairs.
Adding an entry: 
	
	addkv(object, pair);

Looking up an entry: 
	
	thing_t *found = getobjval(object, lookupstr);

## Printing
### printer_t
JSON printer.
To construct:
	
	printer_t *prin = new_printer((FILE *)outstream); 
	// Or NULL - defaults to stdout

To destroy:

	del_printer(prin);

Note that you have to close the outstream on your own with:
	
	close_ostream(prin);

### print_thing
General, simple print function.
	
	print_thing((printer_t *)p, (thing_t*)th);

## Parsing
### new_parser
	parser_t *p = new_parser((char *)filename_to_open);
### parse
Populates p->data with the next thing_t * constructed from the file.
	
	register thing_t *result;
	while (parse((parser_t *)p))
	{
		result = p->data;
		// Do something with result
	}

Note that you must del_thing() each result because they are not stored or automatically freed.
### parser_quit
Destroys all the resources of the parser.

	parser_quit((parser_t *)p);

## Example Usage
In parse.c, the main() function shows an example usage of the parser. 
It simply reads in the contents of the file indicated by its first command line argument, creates data structures for it,
and prints them. By itself it could be used to prettify JSON. It also prints out memory statistics.
Its code:

	int main(int argc, char **argv)
	{
		if (argc < 2)
		{
			fprintf(stderr, "no filename supplied.\n");
			exit(-1);
		}
	
		parser_t *p = new_parser(argv[1]);
		printer_t *print = new_printer(NULL);
	
		unsigned int level = 0, ctr = 0;
	
		register thing_t *t;
	
		while (parse(p))
		{
			t = p->data;
	
			fprintf(stderr, "[%d]\n", ctr++);
			print_thing(print, t);
			del_thing(t);
		}
	
		del_printer(print);
		parser_quit(p);
	
		fprintf(stderr, "MEM STATS:\n");
		fprintf(stderr, "Allocs: %d\nBytes: %d\n", malloc_c, mem_c);
		fprintf(stderr, "Frees: %d\n", free_c);
	
		return 0; 
	}
