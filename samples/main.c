#include <stdio.h>
#include <stdlib.h>
#include <jdad.h>


int main(int argc, char **argv) 
{
	if (argc < 2)
	{
		fprintf(stderr, "No filename supplied.\n");
		exit(-1);
	}

	FILE *in = fopen(argv[1], "r");

	if (in == NULL)
	{
		fprintf(stderr, "Error opening %s\n", argv[1]);
		exit(-1);
	}

	parser_t *p = new_json_parser();
	
	
	printer_t *print = new_printer(NULL, Pretty);

	unsigned int ctr = 0;

	register thing_t *t;

	while (! json_parser_eoi(p))
	{
		t = parse_json_file(p, in); 

		fprintf(stderr, "[%d]\n", ctr++);
		print_thing(print, t);
		del_thing(t);
	}

	del_printer(print);
	json_parser_quit(p);

	print_mem_use();

	return 0; 
}
