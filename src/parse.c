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
	printf("Unexpected token %s (%d), aborting\n",
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
	thing_t *newtr;
	switch (p->la)
	{
		case tTrue:
			match(tTrue);
			newtr = new_scal(_s("True"), Truthval);
			sa(newtr, truthval) = True;
			break;
		case tFalse:
			match(tFalse);
			newtr = new_scal(_s("False"), Truthval);
			sa(newtr, truthval) = False;
			break;
		case tNull:
			match(tNull);
			newtr = new_scal(_s("Null"), Truthval);
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

		thing_t *newstr = new_scal(copy, String);

		sa(newstr, string) = copy;
		sa(newstr, len) = p->scan->buflen - 2; // Account for quotes

		/*printf("\nHex contents(2): ");
		int i;
		for (i = 0; i < p->scan->buflen; i++)
			if (p->scan->str[i] == 0)
				printf("NUL at %d\n", i);
		*/
		

		match(tString);

		//node_t *strnode = new_node(0, Scalar, newstr);

		return newstr;
	}

	error(p);
}

thing_t * doble(parser_t *p)
{
	jchar *copy = new_copy(p->scan->str, 0);

	thing_t *newdob = new_scal(copy, Doble);
	sa(newdob, number).doble = jstrtod(copy, NULL);
	match(tDoble);

	if (p->la == tExp)
	{
		sa(newdob, number).exp = jstrtol(p->scan->str, NULL, 0);
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
	jchar *key;

	while (p->la != tRCurl)
	{		
		/* "Quick and dirty" match of string to avoid wasting
		** resources on making a thing_t * = string(p) on every
		** key. We just want the string.
		*/
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