#include <stdio.h>
#include <stdlib.h>
#include "src/jdad.h"


int main(int argc, char **argv) 
{
	if (argc < 2)
	{
		fprintf(stderr, "No filename supplied.\n");
		exit(-1);
	}

	parser_t *p = new_parser(argv[1]);
	printer_t *print = new_printer(NULL, Pretty);

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

	return 0; 
}