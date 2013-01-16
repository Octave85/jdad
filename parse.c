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
	printf("Unexpected token %s, aborting\n",
		p->scan->str);

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
	// Get rid of quotes
	char *copy = new_copy(p->scan->str + 1, 
		strlen(p->scan->str) - 2);

	thing_t *newstr = new_scal(copy, String);
	sa(newstr, string) = copy;

	match(tString);

	//node_t *strnode = new_node(0, Scalar, newstr);

	return newstr;
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
	thing_t *aux;

	while (p->la != tRCurl)
	{
		aux = string(p);
		
		match(tColon);

		addkv(newobj, new_pair(aux->string, thing(p)));

		if (p->la == tComma)
			match(tComma);
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
	}

	match(tRBrace);

	return newarr;
}

thing_t * thing(parser_t *p)
{
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

		default:
			error(p);

	}

	return (thing_t *)NULL;
}


int main(int argc, char **argv)
{
	parser_t *p = malloc(sizeof(parser_t));

	p->scan = malloc(sizeof(scanner_t));
	p->scan->str = calloc(50, 1);
	p->scan->file = open_json("test.json");

	thing_t *t;

	unsigned int level = 0;

	p->la = scan_json(p->scan);

	while (p->la != tEnd)
	{
		t = thing(p);
		if (t) print_thing(t, &level);

		//del_scal(t);
		t = NULL;
		putchar('\n');
	}

	return 0; 
}