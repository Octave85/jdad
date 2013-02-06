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
	printf("Unexpected %s %s, aborting\n",
		tok2str[p->la], p->scan->str);

	exit(-1);
}

token_t  _match(token_t t, parser_t *p)
{
	if (p->la == t)
		return scan_json(p->scan);
	else
		error(p);
}

jchar *new_copy(jchar *str, int len)
{
	len = ( ! len) ? jstrlen(_s(str)) : len;

	jchar *newcopy = (jchar *)c_malloc(sizeof(jchar) * (len + 1));
	newcopy = (jchar *)memcpy(newcopy, str, sizeof(jchar) * len);
	newcopy[len] = 0;

	return newcopy;
}

thing_t * truthval(parser_t *p)
{
	thing_t *newtr = NULL;
	switch (p->la)
	{
		case tTrue:
			match(tTrue);
			newtr = new_scal(Truthval);
			sa(newtr, truthval) = True;
			break;
		case tFalse:
			match(tFalse);
			newtr = new_scal(Truthval);
			sa(newtr, truthval) = False;
			break;
		case tNull:
			match(tNull);
			newtr = new_scal(Truthval);
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
		jchar *copy = copy_sansquotes(p->scan->str, p->scan->buflen);

		thing_t *newstr = new_scal(String);

		sa(newstr, string) = copy;
		sa(newstr, len) = p->scan->buflen;

		/*printf("\nHex contents(2): ");
		int i;
		for (i = 0; i < p->scan->buflen; i++)
			if (p->scan->str[i] == 0)
				printf("NUL at %d\n", i);
		*/
		

		match(tString);

		return newstr;
	}

	error(p);
}

int get_exponent(parser_t *p)
{
	int exponent;
	if (p->la == tExp)
	{
		exponent = jstrtol(p->scan->str, NULL, 0);
		match(tExp);
		return exponent;
	}
	else
	{
		return 0;
	}
}

thing_t * integer(parser_t *p)
{
	thing_t *newint = new_scal(Integer);
	jchar *copy = new_copy(p->scan->str, 0);
	sa(newint, stringval) = copy;

	errno = 0;
	sa(newint, number).integer = jstrtol(copy, NULL, 0);

	if (errno == ERANGE)
		sa(newint, stype) = BigInt;
	
	match(tInteger);
	sa(newint, number).exponent = get_exponent(p);

	return newint;
}

thing_t * doble(parser_t *p)
{
	thing_t *newdob = new_scal(Doble);
	jchar *copy = new_copy(p->scan->str, 0);
	sa(newdob, stringval) = copy;

	errno = 0;
	sa(newdob, number).doble = jstrtod(copy, NULL);

	if (errno == ERANGE)
		sa(newdob, stype) = BigDob;

	match(tDoble);
	sa(newdob, number).exponent = get_exponent(p);
	return newdob;
}

thing_t * object(parser_t *p)
{
	thing_t *newobj = new_obj(0);
	jchar *key;

	while (p->la != tRCurl)
	{		
		if (p->la == tString)
		{
			key = copy_sansquotes(p->scan->str, p->scan->buflen);
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
		addelem(newarr, thing(p));

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

	case tInteger:
		return integer(p);
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
		fprintf(stderr, "Got End from beggining parse\n");
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

parser_t *new_parser(jchar *filename)
{
	parser_t *newp = (parser_t *)c_malloc(sizeof(parser_t));

	if (newp)
	{
		newp->scan = (scanner_t *)c_malloc(sizeof(scanner_t));
		if (newp->scan)
			newp->scan->str = 
			(jchar *)c_calloc(SCANBUF_SIZE, sizeof(jchar));

		newp->scan->file = open_json(filename);

		newp->la = tBegin;
	}

	return newp;
}

inline int parse_eof(parser_t *p)
{
	return feof(p->scan->file);
}

parser_t *parser_reopen(parser_t *p, jchar *filename)
{
	p->scan->file = open_json(filename);

	return p;
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