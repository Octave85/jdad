/* document -> thing document | thing | empty
** thing -> object | array | string | number
** object -> '{' object-contents '}'
** object-contents -> object-contents string ':' thing | empty
** array -> '[' array-contents ']'
** array-contents -> thing ',' array-contents | thing | empty
*/

#include "parse.h"

#define match(t) p->la = _match(t, p)

static void error(parser_t *p)
{
	printf("Unexpected %s %s, aborting\n",
		tok2str[p->la], p->scan->str);

	exit(-1);
}

static token_t  _match(token_t t, parser_t *p)
{
	if (p->la == t)
		return scan_json(p->scan);
	else
		error(p);
}

static jchar *new_copy(jchar *str, int len)
{
	len = ( ! len) ? jstrlen(_s(str)) : len;

	jchar *newcopy = (jchar *)c_malloc(sizeof(jchar) * (len + 1));
	newcopy = (jchar *)memcpy(newcopy, str, sizeof(jchar) * len);
	newcopy[len] = 0;

	return newcopy;
}

static thing_t * truthval(parser_t *p)
{
	thing_t *newtr = NULL;
	switch (p->la)
	{
		case tTrue:
			match(tTrue);
			newtr = new_json_scal(Truthval);
			sa(newtr, truthval) = True;
			break;
		case tFalse:
			match(tFalse);
			newtr = new_json_scal(Truthval);
			sa(newtr, truthval) = False;
			break;
		case tNull:
			match(tNull);
			newtr = new_json_scal(Truthval);
			sa(newtr, truthval) = Null;
			break;
		default:
			printf("errrrr\n");
			exit(-1);
			break;
	}

	return newtr;
}

static thing_t * string(parser_t *p)
{
	if (p->la == tString)
	{
		// Get rid of quotes
		jchar *copy = copy_sansquotes(p->scan->str, p->scan->buflen);

		thing_t *newstr = new_json_scal(String);

		sa(newstr, string) = copy;
		sa(newstr, len) = p->scan->buflen;

		match(tString);

		return newstr;
	}

	error(p);
}

static int get_exponent(parser_t *p)
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

static thing_t * integer(parser_t *p)
{
	thing_t *newint = new_json_scal(Integer);
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

static thing_t * doble(parser_t *p)
{
	thing_t *newdob = new_json_scal(Doble);
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

static thing_t * object(parser_t *p)
{
	thing_t *newobj = new_json_obj(0);
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
		json_obj_add_pair(newobj, new_json_obj_pair(key, thing(p)));
		
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

static thing_t * array(parser_t *p)
{
	thing_t *newarr = new_json_arr(0);

	while (p->la != tRBrace)
	{
		json_arr_add_elem(newarr, thing(p));

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

static thing_t * thing(parser_t *p)
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
		return NULL;

	default:
		error(p);

	}

	return (thing_t *)NULL;
}


/* These are the main interface parsing functions */

thing_t * parse_json_file(parser_t *p, FILE *fh)
{
	p->scan->mode = mode_file;
	p->scan->in = fh;
	p->data = thing(p);

	return p->data;
}

thing_t * parse_json_string(parser_t *p, jchar *str)
{
	p->scan->mode = mode_string;
	p->scan->in = str;
	p->data = thing(p);
	
	return p->data;
}

parser_t *new_json_parser(void)
{
	parser_t *newp = (parser_t *)c_malloc(sizeof(parser_t));

	if (newp)
	{
		newp->scan = (scanner_t *)c_malloc(sizeof(scanner_t));
		if (newp->scan)
			newp->scan->str = 
			(jchar *)c_calloc(SCANBUF_SIZE, sizeof(jchar));
		newp->scan->mode = mode_undet;

		newp->data = NULL;
		newp->la = tBegin;
	}

	return newp;
}

inline int json_parser_eoi(parser_t *p)
{
	switch (p->scan->mode) {
	case mode_file:
		return feof((FILE *)p->scan->in);
	case mode_string:
		return (*((jchar *)p->scan->in) == '\0');
	case mode_undet:
		return 0;
	default:
		return 1;
	}
}

parser_t *parser_reopen(parser_t *p, jchar *filename)
{
	p->scan->in = (FILE *)open_json(filename);

	return p;
}

void json_parser_quit(parser_t *p)
{
	c_free(p->scan->str);

	c_free(p->scan);

	c_free(p);
}
