/* document -> thing document | thing | empty
** thing -> object | array | string | number
** object -> '{' object-contents '}'
** object-contents -> object-contents string ':' thing | empty
** array -> '[' array-contents ']'
** array-contents -> thing ',' array-contents | thing | empty
*/

#include "parse.h"

#define match(t) p->la = _match(t, p)

token_t  _match(token_t t, parser_t *p)
{
	if (p->la == t)
		return scan_json(p->scan);
	else
		printf("errrr\n");
}

char *new_copy(char *str, int len)
{
	len = ( ! len) ? strlen(str) : len;

	char *newcopy = malloc(len + 1);
	newcopy = strncpy(newcopy, str, len);
	newcopy[len] = '\0';

	return newcopy;
}

node_t * new_node(unsigned int len, type_t type, thing_t *thing)
{
	node_t *newnode = (node_t *)malloc(sizeof(node_t));

	newnode->type = type;
	newnode->thing = thing;
	newnode->length = len;
	newnode->next = NULL;

	return newnode;
}

node_t * string(parser_t *p)
{
	// Get rid of quotes
	char *copy = new_copy(p->scan->str + 1, 
		strlen(p->scan->str) - 2);

	thing_t *newstr = new_scal(copy, String);
	sa(newstr, string) = copy;

	match(tString);

	node_t *strnode = new_node(0, Scalar, newstr);

	return strnode;
}

node_t * doble(parser_t *p)
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

	node_t *dobnode = new_node(0, Scalar, newdob);

	return dobnode;
}

node_t * object(parser_t *p)
{
	unsigned int len = 0;
	node_t *key; 
	node_t *value;

	node_t *newobj = new_node(0, Object, NULL);

	while (p->la != tRCurl)
	{
		key = string(p);
		len++;
		match(tColon);
		value = thing(p);
		len++;
		
		key->next = newobj->next;
		newobj->next = key;
		value->next = newobj->next;
		newobj->next = value;
	}

	match(tRCurl);

	return newobj;
}

node_t * thing(parser_t *p)
{
	switch (p->la)
	{
		case tLCurl:
			match(tLCurl);
			return object(p);
			break;

		//case tLBrace:
		//	match(tLBrace);
		//	array(p);
		//	break;

		case tDoble:	// All primitives resolve to themselves
			return doble(p);
			break;

		case tString:
			return string(p);
			break;

		case tNull:
			match(tNull);
		case tTrue:
			match(tTrue);
		case tFalse:
			match(tFalse);
			printf("%s\n", p->scan->str);

		default:
			printf("%s\n", p->scan->str);

	}

	return (node_t *)NULL;
}


int main(int argc, char **argv)
{
	parser_t *p = malloc(sizeof(parser_t));

	p->scan = malloc(sizeof(scanner_t));
	p->scan->str = calloc(50, 1);
	p->scan->file = open_json("test.json");

	node_t *t;

	p->la = scan_json(p->scan);

	while (p->la != tEnd)
	{
		t = thing(p);
		//if (t) print_thing(t->thing);

		//del_scal(t);
		t = NULL;
		putchar('\n');
	}

	return 0; 
}