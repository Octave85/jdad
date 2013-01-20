/* document -> thing document | thing | empty
** thing -> object | array | string | number
** object -> '{' object-contents '}'
** object-contents -> object-contents string ':' thing | empty
** array -> '[' array-contents ']'
** array-contents -> thing ',' array-contents | thing | empty
*/

#include "parse.h"

#define match(t) p->la = _match(t, p)

void error(parser_t *p)
{
	printf("Unexpected token %s(%d), aborting\n",
		p->scan->str, p->la);

	exit(-1);
}

token_t  _match(token_t t, parser_t *p)
{
	if (p->la == t)
		return scan_json(p->scan);
	else
		error(p);
}

char *new_copy(char *str, int len)
{
	len = ( ! len) ? strlen(str) : len;

	char *newcopy = malloc(len + 1);
	newcopy = strncpy(newcopy, str, len);
	newcopy[len] = '\0';

	return newcopy;
}

thing_t * truthval(parser_t *p)
{
	thing_t *newtr;
	switch (p->la)
	{
		case tTrue:
			match(tTrue);
			newtr = new_scal("True", Truthval);
			sa(newtr, truthval) = True;
			break;
		case tFalse:
			match(tFalse);
			newtr = new_scal("False", Truthval);
			sa(newtr, truthval) = False;
			break;
		case tNull:
			match(tNull);
			newtr = new_scal("Null", Truthval);
			sa(newtr, truthval) = Null;
			break;
		default:
			printf("errrrr\n");
			exit(-1);
			break;
	}

	return newtr;
}

thing_t * string(parser_t *p)
{
	if (p->la == tString)
	{
		// Get rid of quotes
		char *copy = copy_sansquotes(p->scan->str);

		thing_t *newstr = new_scal(copy, String);
		sa(newstr, string) = copy;

		match(tString);

		//node_t *strnode = new_node(0, Scalar, newstr);

		return newstr;
	}

	error(p);
}

thing_t * doble(parser_t *p)
{
	char *copy = new_copy(p->scan->str, 0);

	thing_t *newdob = new_scal(copy, Doble);
	sa(newdob, number).doble = strtod(copy, NULL);
	match(tDoble);

	if (p->la == tExp)
	{
		sa(newdob, number).exp = strtol(p->scan->str, NULL, 0);
		match(tExp);
	}
	else
	{
		sa(newdob, number).exp = 0;
	}

	//node_t *dobnode = new_node(0, Scalar, newdob);

	return newdob;
}

thing_t * object(parser_t *p)
{
	thing_t *newobj = new_obj(0);
	char *key;

	while (p->la != tRCurl)
	{		
		/* "Quick and dirty" match of string to avoid wasting
		** resources on making a thing_t * = string(p) on every
		** key. We just want the string.
		*/
		if (p->la == tString)
		{
			key = copy_sansquotes(p->scan->str);
			match(tString);
		}
		else
		{
			error(p);
		}
		
		match(tColon);

		addkv(newobj, new_pair(key, thing(p)));

		if (p->la == tComma)
			match(tComma);
		else if (p->la == tRCurl)
			break;
		else
			error(p);
	}

	match(tRCurl);


	return newobj;
}

thing_t * array(parser_t *p)
{
	thing_t *newarr = new_arr(0);

	while (p->la != tRBrace)
	{
		addelem(newarr, ARR_A, thing(p));

		if (p->la == tComma)
			match(tComma);
		else if (p->la == tRBrace)
			break;
		else
			error(p);
	}

	match(tRBrace);

	return newarr;
}

thing_t * thing(parser_t *p)
{
	/* Can't just make this a case with fall-through... the p->la
	** considered by the switch doesn't change to the newly-matched token.
	** pre-stored or put in register, maybe?
	*/
	if (p->la == tBegin)
		match(tBegin);

	switch (p->la)
	{
		case tLCurl:
			match(tLCurl);
			return object(p);
			break;

		case tLBrace:
			match(tLBrace);
			return array(p);
			break;

		case tDoble:	// All primitives resolve to themselves
			return doble(p);
			break;

		case tString:
			return string(p);
			break;

		case tNull:
		case tTrue:
		case tFalse:
			return truthval(p);

		case tErr:
			error(p);

		case tEnd:
			return NULL;

		default:
			error(p);

	}

	return (thing_t *)NULL;
}

int parse(parser_t *p)
{
	p->data = thing(p);

	return (p->data != NULL);
}

parser_t *new_parser(char *filename)
{
	parser_t *newp = c_malloc(sizeof(parser_t));

	if (newp)
	{
		newp->scan = c_malloc(sizeof(scanner_t));
		if (newp->scan)
			newp->scan->str = c_calloc(SCANBUF_SIZE, sizeof(char));
		newp->scan->file = open_json(filename);

		newp->la = tBegin;
	}

	return newp;
}

void parser_quit(parser_t *p)
{
	if (p->data)
		del_thing(p->data);

	c_free(p->scan->str);

	fclose(p->scan->file);

	c_free(p->scan);

	c_free(p);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "no filename supplied.\n");
		exit(-1);
	}

	parser_t *p = new_parser(argv[1]);

	unsigned int level = 0, ctr = 0;

	register thing_t *t;

	while (parse(p))
	{
		t = p->data;

		fprintf(stderr, "[%d]\n", ctr++);
		print_thing(t, &level);
		del_thing(t);
	}

	parser_quit(p);

	fprintf(stderr, "MEM STATS:\n");
	fprintf(stderr, "Allocs: %d\nBytes: %d\n", malloc_c, mem_c);
	fprintf(stderr, "Frees: %d\n", free_c);

	return 0; 
}