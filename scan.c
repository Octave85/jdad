#include "scan.h"


#define case_digit case '0': \
case '1': \
case '2': \
case '3': \
case '4': \
case '5': \
case '6': \
case '7': \
case '8': \
case '9' 

#define nextc() _nextc(sc->file, sc->str)
#define prevc() _prevc(c, sc->file, sc->str)
#define sspaces() _sspaces(c, sc->file, sc->str)
#define error() _error(c, sc)

#define unex() error(); *state = Error; tok = tErr;
#define accept(tt) *state = Accept; tok = tt;
#define accept_pb(tt) prevc(); accept(tt);



FILE * open_json(char *filename)
{
	FILE *json = fopen(filename, "r");

	if (json)
	{
		return json;
	}
	else
	{
		printf("Error openixng file.\n");
		exit(1);
	}
}

int _nextc(FILE *file, char *str)
{
	int l, c;
	c = getc(file);

	if (str != NULL)
	{
		l = strlen(str);

		// Add character and new null byte
		str[l] = c;
		str[l+1] = '\0';
	}
	
	return c;
}

int _prevc(int c, FILE *file, char *str)
{
	if (str != NULL)
	{	
		int l = strlen(str);
		// Shorten by one char
		str[l-1] = '\0';
	}

	return ungetc(c, file);
}

int _error(int c, scanner_t *sc)
{
	printf("Unexpected %c in stream (in state %d)"
		" (Had partial token %s)\n", c, sc->state, sc->str);
	sc->errors++;
	
	return 1;
}

void _sspaces(int c, FILE *file, char *str)
{
	// Skip spaces
	while (isspace(c))
	{
		c = getc(file);
	}
	// Put back first nonspace
	_prevc(c, file, str);
}

token_t scan_json(scanner_t *sc)
{
	token_t tok;
	state_t *state = &(sc->state);
	*state = Start;

	int c, i = 0;
	memset(sc->str, 0, strlen(sc->str));

	for (;;i++)
	{
		
		switch (*state)
		{
			case Start:
				c = nextc();

				switch (c)
				{
					case '{':
						accept(tLCurl);
						break;

					case '}':
						accept(tRCurl);
						break;

					case ',':
						accept(tComma);
						break;

					case '[':
						accept(tLBrace);
						break;

					case ']':
						accept(tRBrace);
						break;

					case '"':
						tok = tString;
						*state = InStr;
						break;

					case ':':
						accept(tColon);
						break;
					
					case 'T':
					case 't':
						*state = InT1;
						tok = tTrue;
						break;

					case 'F':
					case 'f':
						*state = InF1;
						tok = tFalse;
						break;

					case 'N':
					case 'n':
						*state = InN1;
						tok = tNull;
						break;

					case '.':
					case '-':
					case_digit:
						*state = StartNum;
						prevc();
						break;

					case EOF:
						accept(tEnd);
						break;

					default:
						if (isspace(c))
						{
							sspaces();
						}
						else
						{
							unex();
						}
				}

				break;
			case Accept:
			case Error:
				return tok;

			case StartNum:
				c = nextc();

				switch (c)
				{
					case '-':
					case_digit:
						*state = InNum;
						tok = tDoble;
						break;

					case '.':
						*state = InFrac;
						tok = tDoble;
						break;

					default:
						unex();
				}
				break; // StartNum

			case InNum:
				c = nextc();

				switch (c)
				{
					case_digit:	// Stay in this state
						break;

					case '.':
						*state = InFrac;
						tok = tDoble;
						break;

					case 'e':
					case 'E':
						*state = InExp;
						tok = tDoble;
						break;

					default:
						accept_pb(tDoble);
				}
				break; // InNum

			case InFrac:
				c = nextc();
				switch (c)
				{
					case_digit:	// Stay in this state
						break;

					case 'e':
					case 'E':
						*state = InExp;
						break;

					case '.':
						unex();
						break;

					default:
						accept_pb(tDoble);
				}
				break; //InFrac

			case InExp:
				c = nextc();
				switch (c)
				{
					case '+':
					case '-':
					case_digit:	// Stay in this state
						break;

					case '.':
						unex();
						break;

					default:
						accept_pb(tDoble);
				}
				break; // InExp

			case InStr:
				c = nextc();
				switch (c)
				{
					case '\\':
						*state = InEscape;
						break;

					case '"':
						accept(tString);
						break;

					case EOF:
						unex();
				}
				break; // InStr

			case InEscape:
				c = nextc();
				switch (c)
				{
					case '"':
					case '\\':
					case '/':
					case 'b':
					case 'f':
					case 'n':
					case 'r':
					case 't':
					case 'u':
						*state = InStr;
						break;

					default:
						unex();
				}
				break; // InEscape

			case InT1:
				c = nextc();
				if (c == 'R' || c == 'r')
				{
					*state = InT2;
				}
				else
				{
					unex();
				}
				break; // InT1

			case InT2:
				c = nextc();
				if (c == 'U' || c == 'u')
				{
					*state = InT3;
				}
				else
				{
					unex();
				}
				break; // InT2

			case InT3:	
				c = nextc();
				if (c == 'E' || c == 'e')
				{
					accept(tTrue);
				}
				else
				{
					unex();
				}
				break; // InT3

			case InF1:
				c = nextc();
				if (c == 'A' || c == 'a')
				{
					*state = InF2;
				}
				else
				{
					unex();
				}
				break; // InF1

			case InF2:
				c = nextc();
				if (c == 'L' || c == 'l')
				{
					*state = InF3;
				}
				else
				{
					unex();
				}
				break; // InF2

			case InF3:	
				c = nextc();
				if (c == 'S' || c == 's')
				{
					*state = InF4;
				}
				else
				{
					unex();
				}
				break; // InF3

			case InF4:
				c = nextc();
				if (c == 'E' || c == 'e')
				{
					accept(tFalse);
				}
				else
				{
					unex();
				}
				break; // InF4

			case InN1:
				c = nextc();
				if (c == 'U' || c == 'u')
				{
					*state = InN2;
				}
				else
				{
					unex();
				}
				break; // InN1

			case InN2:
				c = nextc();
				if (c == 'L' || c == 'l')
				{
					*state = InN3;
				}
				else
				{
					unex();
				}
				break; // InN2

			case InN3:	
				c = nextc();
				if (c == 'L' || c == 'l')
				{
					accept(tNull);
				}
				else
				{
					unex();
				}
				break; // InN3;

		
		// End of state switch
		}
	
	// End of for loop
	}
}

int main(int argc, char **argv)
{
	scanner_t *json = malloc(sizeof(scanner_t));
	json->str = calloc(50, 1);
	json->file = open_json("test.json");

	token_t tok;

	while ((tok = scan_json(json)) != tEnd)
	{
		if (tok != tErr) printf("Toke: %s (%d)\n", json->str, tok);
	}
	putchar('\n');

	return 0;

}