#include "print.h"

printer_t * new_printer(FILE *ostream, printmode_t mode)
{
	printer_t *newprinter = (printer_t *)c_malloc(sizeof(printer_t));

	if (newprinter)
	{
		newprinter->donl = newprinter->doin = (mode == Pretty);
		
		newprinter->level = 0;
		newprinter->ostream = (ostream != NULL) ? ostream : stdout;
	}

	return newprinter;
}

void del_printer(printer_t *pr) { c_free(pr); }


#define princ(PR_NAME, c) fputc(c, PR_NAME->ostream)
#define print(...) fprintf(PR_NAME->ostream, __VA_ARGS__)
#define inc_in(PR_NAME) (PR_NAME->level++)
#define dec_in(PR_NAME) (PR_NAME->level--)

/* PRETTY_PRINT and COMPACT_PRINT override programmatic
** mode selection to speed things up. Allows one to compile
** "pretty" and "compact" JSON printers.
*/
#if defined(PRETTY_PRINT)

#define nl(PR_NAME) princ(PR_NAME, '\n')

void in(printer_t *pr)
{
	unsigned int lvl = pr->level;
	while (lvl--) print("  ");
}

#define SPACE " "

#elif defined(COMPACT_PRINT)

#define nl(PR_NAME)
#define in(PR_NAME)

#define SPACE ""

#else /* Define default functions that check the condition each time */

#define nl(PR_NAME) if (PR_NAME->donl) princ(PR_NAME, '\n')
#define SPACE ((PR_NAME->doin) ? " " : "")

void in(printer_t *pr)
{
	if (PR_NAME->doin)
	{
		unsigned int lvl = pr->level;
		while (lvl--) print("  ");
	}
}
#endif



void print_scalar(printer_t *pr, thing_t *sc)
{
	switch (sa(sc, stype))
	{
	case Integer:
		print("%d", sa(sc, number).integer);
		goto print_exp;
		break;

	case Doble:
	case BigInt:
	case BigDob:
		print("%s", sa(sc, stringval));
		goto print_exp;
		break;

	case String:
		princ(pr, '"');
		fwrite(sa(sc, string), 
			   sa(sc, len) * sizeof(jchar), 
			   1, 
			   pr->ostream);
		princ(pr, '"');
		break;

	case Truthval:
		print(tv2str[sa(sc, truthval)]);
		break;

	default:
		print("Unrecognized scalar type to print\n");
	}

	return;

print_exp:
	if (sa(sc, number).exponent > 0)
	{
		princ(pr, 'e');
		print("%d", sa(sc, number).exponent);
	}
}

void print_obj(printer_t *pr, thing_t *obj)
{
#define pair_print(pair) do { print("\"%s\":%s", k->key, SPACE); \
	print_thing(pr, k->val); } while (0)
#define pair_print_c(pair) do { pair_print(pair); princ(pr, ','); } while (0)

	llm_t *key_list = oa(obj, key_first);
	pair_t *k;

	print("{");
	
	inc_in(pr);
	nl(pr);

	while (key_list && (key_list->next != NULL))
	{
		k = (pair_t *)key_list->data;

		/* Print every element with comma */
		in(pr);
		pair_print_c(k);
		nl(pr);

		key_list = key_list->next;
	}
	
	k = (pair_t *)key_list->data;

	/* Print the last element, sans comma. */
	in(pr);
	pair_print(k);
	nl(pr);
	dec_in(pr);
	in(pr);

	print("}");

#undef pair_print_c
#undef pair_print
}


void print_arr(printer_t *pr, thing_t *arr)
{
#define el_print(lm) do { print_thing(pr, (thing_t *)lm->data); } while(0)
#define el_print_c(lm) do { el_print(lm); princ(pr, ','); } while(0)	

	print("[");
	inc_in(pr);
	nl(pr);

	llm_t *cur = aa(arr, c_first);


	if (cur)
	{
		while (cur && (cur->next != NULL))
		{
			/* Print each element with comma */
			in(pr);
			el_print_c(cur);
			nl(pr);
			cur = cur->next;
		}
		/* Print last element, sans comma. */
		in(pr);
		el_print(cur);
		nl(pr);
	}
	
	dec_in(pr);
	in(pr);

	print("]");

#undef el_print_c
#undef el_print
}


void print_thing(printer_t *pr, thing_t *thing)
{
	switch (thing->type)
	{
	case Scalar:
		print_scalar(pr, thing);
		break;
	case Object:
		print_obj(pr, thing);
		break;
	case Array:
		print_arr(pr, thing);
		break;
	default:
		print("Unrecognized Object to print\n");
	}
}