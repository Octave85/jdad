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

static jchar * _extendbuf(jchar *buf, unsigned int buflen)
{
	jchar *tmp = (jchar *)realloc(buf, (buflen + SCANBUF_SIZE));

	if (tmp == NULL)
	{
		fprintf(stderr, "Error extending scanner buffer\n");
		exit(1);
	}


	return tmp;
}

static int _addc(scanner_t *sc, int c)
{
	jchar *buf = sc->str;
	unsigned int buflen = sc->buflen;


	if (buf != NULL)
	{
		if ((buflen + 1) % SCANBUF_SIZE == 0)
			sc->str = _extendbuf(buf, buflen);

		// Add character and new null byte
		buf[buflen] = c;

		sc->buflen = buflen + 1;
	}

	return c;
}

static int _error(int c, scanner_t *sc)
{
	printf("Unexpected %c in stream at line %d (in state %s) (Had partial token %s)\n", 
		c, lineno, state2str[sc->state], sc->str);
	sc->errors++;
	
	return 1;
}


/* These are functions that deal with the input. The scanner needs them to know how to deal with the input. */
static int _nextc_file(scanner_t *sc)
{
	int c;
	c = getc((FILE *)sc->in);
	lineno += (c == '\n');
	return c;
}

static int _nextc_string(scanner_t *sc)
{
	int c;
	c = *((jchar *)sc->in++);
	lineno += (c == '\n');
	return c;
}

static inline int _prevc_file(int c, scanner_t *sc)
{
	return ungetc(c, (FILE *)sc->in);
}

static inline int _prevc_string(int c, scanner_t *sc)
{

	sc->in--;
	return *((jchar *)sc->in);
}

static inline void _schars_file(int c, scanner_t *sc)
{
	// Skip spaces
	while (isspace(c))
	{
		c = _nextc_file(sc);
	}
	// Put back first nonspace
	_prevc_file(c, sc);
}

static inline void _schars_string(int c, scanner_t *sc)
{
	while (isspace(c))
	{
		c = _nextc_string(sc);
	}

	_prevc_string(c, sc);
}


/* This is the JSON DFA. It interacts directly with the input. */
static token_t __scan_json(scanner_t *sc, 
		int (*_nextc)(scanner_t *), 
		int (*_prevc)(int, scanner_t *), 
		void (*_schars)(int, scanner_t *))
{
	token_t tok;
	state_t *state = &(sc->state);
	int c = 0;

	*state = Start;

#define addc(c) _addc(sc, c)
#define nextc() _nextc(sc)
#define prevc(c) _prevc(c, sc)
#define schars() _schars(c, sc)
#define error() _error(c, sc)

#define unex() do { error(); *state = Error; tok = tErr; } while (0)
#define accept(tt) do { *state = Accept; tok = tt; } while (0)
#define accept_pb(tt) do { prevc(c); accept(tt); } while (0)

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
				*state = StartExp;
				break;
			case EOF:
			case '\0':
				accept_pb(tEnd);
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
			c = nextc();
			switch (c)
			{
			case '.':
				*state = InFrac;
				tok = tDoble;
				addc(c);
				break;
			default:
				if (isdigit(c) || c == '-')
				{
					*state = InNum;
					addc(c);
				}
				else
				{
					accept_pb(tInteger);
				}
			}
			break; // StartNum
		case InNum:
			c = nextc();
			switch (c)
			{
			case '.':
				*state = InFrac;
				addc(c);
				tok = tDoble;
				break;
			default:
				if (isdigit(c))
					addc(c);
				else
					accept_pb(tInteger);
			}
			break; // InNum
		case InFrac:
			c = nextc();
			switch (c)
			{
			case '.':
				unex();
				break;
			default:
				if (isdigit(c))
					addc(c);
				else
					accept_pb(tDoble);
			}
			break; //InFrac
		case StartExp:
			c = nextc();
			if (isdigit(c))
			{
				addc(c);
				*state = InExp;
			}
			else if (c == '-' || c == '+')
			{
				addc(c);
				// Check to see there is at least one digit
				c = nextc();
				if (isdigit(c))
				{
					addc(c);
					*state = InExp;
				}
				else
				{
					unex();
				}
			}			
			break; // StartExp
		case InExp:
			c = nextc();
			switch (c)
			{
			case_digit:	// Stay in this state
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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
				addc(c);
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

static inline token_t scan_json_file(scanner_t *sc)
{
	memset(sc->str, 0, SCANBUF_SIZE);
	sc->buflen = 0;

	return __scan_json(sc, _nextc_file, _prevc_file, _schars_file);
}

static inline token_t scan_json_string(scanner_t *sc)
{
	memset(sc->str, 0, SCANBUF_SIZE);
	sc->buflen = 0;

	return __scan_json(sc, _nextc_string, _prevc_string, _schars_string);
}

token_t scan_json(scanner_t *sc)
{
	if (sc->mode == mode_file)
		return scan_json_file(sc);
	else
		return scan_json_string(sc);
}
