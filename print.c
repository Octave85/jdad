#include "print.h"

printer_t * new_printer(FILE *ostream, printmode_t mode)
{
	printer_t *newprinter = c_malloc(sizeof(printer_t));

	if (newprinter)
	{
		switch (mode)
		{
			case Compact:
				newprinter->donl = newprinter->doin = 0;
				break;

		 /* case Pretty: */
			default:
				newprinter->donl = newprinter->doin = 1;
				break;
		}
		
		newprinter->level = 0;
		newprinter->ostream = (ostream != NULL) ? ostream : stdout;
	}

	return newprinter;
}

#define princ(PR_NAME, c) fputc(c, pr->ostream)
#define print(...) fprintf(PR_NAME->ostream, __VA_ARGS__)
#define incin(PR_NAME) (PR_NAME->level++)
#define decin(PR_NAME) (PR_NAME->level--)

/* PRETTY_PRINT and COMPACT_PRINT override programmatic
** mode selection to speed things up. Allows one to compile
** "pretty" and "compact" JSON printers.
*/
#ifdef PRETTY_PRINT
#	define nl(PR_NAME) princ(PR_NAME, '\n')

	void in(printer_t *PR_NAME)
	{
		unsigned int lvl = pr->level;
		while (lvl--) print("  ");
	}

#else
#	ifdef COMPACT_PRINT
#		define nl(PR_NAME)
#		define in(PR_NAME)
#	else /* Define default functions that check the condition each time */
#		define nl(PR_NAME) if (PR_NAME->donl) princ(PR_NAME, '\n')
		void in(printer_t *PR_NAME)
		{
			if (PR_NAME->doin)
			{
				unsigned int lvl = pr->level;
				while (lvl--) print("  ");
			}
		}
#	endif
#endif

void print_scalar(printer_t *PR_NAME, thing_t *sc)
{
	switch (sa(sc, stype))
	{
		case Doble:
			print("%.3f", sa(sc, number).doble);
			if (sa(sc, number).exp > 0)
			{
				princ(PR_NAME, 'e');
				print("%d", sa(sc, number).exp);
			}
			break;
		case String:
			print("\"%s\"", sa(sc, string));
			break;
		case Truthval:
			switch (sa(sc, truthval))
			{
				case False:
					print("False");
					break;
				case True:
					print("True");
					break;
				case Null:
					print("Null");
					break;
			}
			break;
		default:
			print("Unrecognized scalar type to print\n");
	}
}

void print_obj(printer_t *PR_NAME, thing_t *obj)
{
	llm_t *key_list = oa(obj, key_first);
	pair_t *k;

	print("{");
	
	incin(PR_NAME);
	nl(PR_NAME);

#	define pair_print(pair) print("\"%s\": ", k->key); \
	print_thing(PR_NAME, k->val)
#	define pair_print_c(pair) pair_print(pair); princ(PR_NAME, ',')

	while (key_list && (key_list->next != NULL))
	{
		in(PR_NAME);
		k = (pair_t *)key_list->data;

		pair_print_c(k);

		key_list = key_list->next;
		nl(PR_NAME);
	}
	
	in(PR_NAME);
	
	k = (pair_t *)key_list->data;
	pair_print(k);
	
	nl(PR_NAME);

	decin(PR_NAME);
	in(PR_NAME);
	print("}");

#	undef pair_print_c
#	undef pair_print
}


void print_arr(printer_t *PR_NAME, thing_t *arr)
{
	print("[");
	incin(PR_NAME);
	nl(PR_NAME);

	llm_t *cur = aa(arr, c_first);

#	define el_print(lm) print_thing(PR_NAME, (thing_t *)lm->data)
#	define el_print_c(lm) el_print(lm); princ(PR_NAME, ',');

	if (cur)
	{
		while (cur && (cur->next != NULL))
		{
			in(PR_NAME);

			el_print_c(cur);

			cur = cur->next;

			nl(PR_NAME);
		}
		
		in(PR_NAME);
		
		el_print(cur);
		
		nl(PR_NAME);
	}
	
	decin(PR_NAME);
	in(PR_NAME);

#	undef el_print_c
#	undef el_print

	print("]");
}

void print_thing(printer_t *PR_NAME, thing_t *thing)
{
	switch (thing->type)
	{
		case Scalar:
			print_scalar(PR_NAME, thing);
			break;
		case Object:
			print_obj(PR_NAME, thing);
			break;
		case Array:
			print_arr(PR_NAME, thing);
			break;
		default:
			print("Unrecognized Object to print\n");
	}
}