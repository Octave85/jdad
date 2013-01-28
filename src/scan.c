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

#define add_nextc() _addc(sc, _nextc(sc->file, sc))
#define nextc() _nextc(sc->file, sc)
#define addc(c) _addc(sc, c)
#define prevc() _prevc(c, sc->file, sc->str, sc->buflen)
#define schars() _schars(c, sc->file, sc->str, sc->buflen)
#define error() _error(c, sc)

#define unex() do { error(); *state = Error; tok = tErr; } while (0)
#define accept(tt) do { *state = Accept; tok = tt; } while (0)
#define accept_pb(tt) do { prevc(); accept(tt); } while (0)

static unsigned int lineno = 1;

static inline int char2hex(int c)
{
	int cu = toupper(c);
	if (cu >= 'A' && cu <= 'F')
		return cu - '7'; // same as - 'A' + 10
	else if (cu >= '0' && cu <= '9')
		return cu - '0';

	return -1;
}

FILE * open_json(jchar *filename)
{
	FILE *json = fopen(filename, "r");

	if (json)
	{
		return json;
	}
	else
	{
		fprintf(stderr, "Error opening file.\n");
		exit(1);
	}
}

jchar * _extendbuf(jchar *buf, unsigned int buflen)
{
	jchar *tmp = (jchar *)realloc(buf, (buflen + SCANBUF_SIZE));

	if (tmp == NULL)
	{
		fprintf(stderr, "Error extending scanner buffer\n");
		exit(1);
	}


	return tmp;
}

int _addc(scanner_t *sc, int c)
{
	jchar *buf = sc->str;
	unsigned int buflen = sc->buflen;


	if (buf != NULL)
	{
		if ((buflen + 1) % SCANBUF_SIZE == 0)
		{
			sc->str = _extendbuf(buf, buflen);
			
			//fprintf(stderr, "Extended at %d\n", buflen);
		}

		// Add character and new null byte
		buf[buflen] = c;
		
		/*if (buflen < 129 && buflen > 125)
		{	
			fprintf(stderr, "set buf[%d] to %d\n", buflen, c);
			fprintf(stderr, "buf[127] = %d\n", buf[127]);
		}*/
		
		buf[++buflen] = '\0';
		
		/*if (buflen < 130 && buflen > 125)
		{
			fprintf(stderr, "nulled buf[%d]\n", buflen);
			fprintf(stderr, "buf[127] = %d\n", buf[127]);
		}*/


		sc->buflen = buflen;
	}

	return c;
}

int _nextc(FILE *file, scanner_t *sc)
{
	unsigned int buflen = sc->buflen;
	jchar *buf = sc->str;

	int c = getc(file);

	lineno += (c == '\n');

	return c;
}

int _prevc(int c, FILE *file, jchar *str, unsigned int len)
{
	if (str != NULL)
	{	
		// Shorten by one char
		str[len-1] = '\0';
	}

	return ungetc(c, file);
}

int _error(int c, scanner_t *sc)
{
	printf("Unexpected %c in stream at line %d (in state %s) (Had partial token %s)\n", 
		c, lineno, state2str[sc->state], sc->str);
	sc->errors++;
	
	return 1;
}

void _schars(int c, FILE *file, jchar *str, unsigned int len)
{
	// Skip spaces
	while (isspace(c))
	{
		c = getc(file);
	}
	// Put back first nonspace
	_prevc(c, file, str, len);
}

token_t scan_json(scanner_t *sc)
{
	token_t tok;
	state_t *state = &(sc->state);
	*state = Start;

	int c;
	memset(sc->str, 0, SCANBUF_SIZE);
	sc->buflen = 0;

	for (;;)
	{
		
		switch (*state)
		{
		case Start:
			c = nextc();

			switch (c)
			{
				
			case '{':
				accept(tLCurl);
				addc(c);
				break;

			case '}':
				accept(tRCurl);
				addc(c);
				break;

			case ',':
				accept(tComma);
				addc(c);
				break;

			case '[':
				accept(tLBrace);
				addc(c);
				break;

			case ']':
				accept(tRBrace);
				addc(c);
				break;

			case '"':
				tok = tString;
				*state = InStr;
				break;

			case ':':
				accept(tColon);
				addc(c);
				break;
			
			case 'T':
			case 't':
				*state = InT1;
				tok = tTrue;
				addc(c);
				break;

			case 'F':
			case 'f':
				*state = InF1;
				tok = tFalse;
				addc(c);
				break;

			case 'N':
			case 'n':
				*state = InN1;
				tok = tNull;
				addc(c);
				break;

			case '.':
			case '-':
			// digit case handled after
				addc(c);
				*state = StartNum;
				break;

			case 'e':
			case 'E':
				*state = InExp;
				break;

			case EOF:
				accept(tEnd);
				break;

			default:
				if (isspace(c))
				{
					schars();
				}
				else if (isdigit(c))
				{
					addc(c);
					*state = StartNum;
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
			c = add_nextc();

			switch (c)
			{
			case '.':
				*state = InFrac;
				break;

			default:
				if (isdigit(c) || c == '-')
					*state = InNum;
				else
					unex();
			}
			tok = tDoble;
			break; // StartNum

		case InNum:
			c = add_nextc();

			switch (c)
			{
			case '.':
				*state = InFrac;
				tok = tDoble;
				break;

			default:
				if ( ! isdigit(c))
					accept_pb(tDoble);
			}
			break; // InNum

		case InFrac:
			c = add_nextc();

			switch (c)
			{
			case '.':
				unex();
				break;

			default:
				if ( ! isdigit(c))
					accept_pb(tDoble);
			}
			break; //InFrac

		case InExp:
			c = add_nextc();
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
				accept_pb(tExp);
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

			default:
				addc(c);
			}
			break; // InStr

		case InEscape:
			c = nextc();
			switch (c)
			{
			case '"':
				addc('"');
				goto tostr;
			case '\\':
				addc('\\');
				goto tostr;
			case '/':
				addc('/');
				goto tostr;
			case 'b':
				addc('\b');
				goto tostr;
			case 'f':
				addc('\f');
				goto tostr;
			case 'n':
				addc('\n');
				goto tostr;
			case 'r':
				addc('\r');
				goto tostr;
			case 't':
				addc('\t');
				goto tostr;
			case 'u':
				*state = InHex;
				break;
			default:
				unex();
			}
			break;
tostr:			
			*state = InStr;
			break; // InEscape

		case InHex:
			c = nextc();
			int hex;
			hex = 0;
			
			if (isxdigit(c))
				hex |= (char2hex(c) << 12);
			else
				unex();
			c = nextc();
			if (isxdigit(c))
				hex |= (char2hex(c) << 8);
			else
				unex();
			c = nextc();
			if (isxdigit(c))
				hex |= (char2hex(c) << 4);
			else
				unex();
			c = nextc();
			if (isxdigit(c))
				hex |= char2hex(c);
			else
				unex();

			addc(hex);
			
			*state = InStr;
			
			break; // InHex

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

		default:
			printf("Internal error: unexpected state %d\n", *state);
			return tErr;

	
		// End of state switch
		}
	
	// End of for loop
	}
}