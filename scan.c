#include "global.h"
#include "types.h"
#include "symtab.h"
#include <ctype.h>
#include <stdlib.h>

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

#define unex() error(c, state, str, file); state = Error; tok = tEnd;
#define accept(tt) prevc(c, file, str); state = Accept; tok = tt;

typedef enum {
	tLCurl,   // 0
	tRCurl,   // 1
	tColon,   // 2
	tComma,   // 3
	tLBrace,  // 4
	tRBrace,  // 5
	tDQuote,  // 6
	tString,  // 7
	tDoble,   // 8
	tSpecial, // 9
	tEscape,  // 10
	tEnd,     // 11
} token_t;

typedef enum {
	Start, Accept, Error,
	InObjK, InObjV,
	InArr, 
	StartNum, InNum, InFrac, InExp, InStr,
	InEscape, InHex, InSpecial,
} state_t;

typedef struct {
	token_t tok;
	char *stringval;
} scan_t;

FILE * open_json(char *filename)
{
	FILE *json = fopen(filename, "r");

	if (json)
	{
		return json;
	}
	else
	{
		printf("Error opening file.\n");
		exit(1);
	}
}

int nextc(FILE *file, char *str)
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

int prevc(int c, FILE *file, char *str)
{
	if (str != NULL)
	{	
		int l = strlen(str);
		// Shorten by one char
		str[l-1] = '\0';
	}

	return ungetc(c, file);
}

int error(int c, state_t st, char *str, FILE *file)
{
	printf("Unexpected %c in stream (in state %d)"
		" (Had partial token %s)\n", c, st, str);
	return 1;
}

void sspaces(int c, FILE *file, char *str)
{
	// Skip spaces
	while (isspace(c))
	{
		c = nextc(file, NULL);
	}
	// Put back first nonspace
	prevc(c, file, str);
}

token_t scan_json(FILE *file, char *str)
{
	token_t tok;
	state_t state = Start;
	int c, i = 0;
	memset(str, 0, strlen(str));

	for (;;i++)
	{
		
		switch (state)
		{
			case Start:
				c = nextc(file, str);

				switch (c)
				{
					case '{':
						state = Accept;
						tok = tLCurl;
						break;

					case '}':
						state = Accept;
						tok = tRCurl;
						break;

					case ',':
						state = Accept;
						tok = tComma;
						break;

					case '[':
						state = Accept;
						tok = tLBrace;
						break;

					case ']':
						state = Accept;
						tok = tRBrace;
						break;

					case '"':
						state = InStr;
						tok = tString;
						break;

					case ':':
						state = Accept;
						tok = tColon;
						break;
					
					case '.':
					case '-':
					case_digit:
						state = StartNum;
						prevc(c, file, str);
						break;

					case EOF:
						state = Accept;
						tok = tEnd;

					default:
						if (isalpha(c))
						{
							state = InSpecial;
						}
						else if (isspace(c))
						{
							sspaces(c, file, str);
						}
						else if (c == EOF)
						{
							state = Accept;
							tok = tEnd;
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
				c = nextc(file, str);

				switch (c)
				{
					case '-':
					case_digit:
						state = InNum;
						tok = tDoble;
						break;

					case '.':
						state = InFrac;
						tok = tDoble;
						break;

					default:
						unex();
				}
				break; // StartNum

			case InNum:
				c = nextc(file, str);

				switch (c)
				{
					case_digit:	// Stay in this state
						break;

					case '.':
						state = InFrac;
						tok = tDoble;
						break;

					case 'e':
					case 'E':
						state = InExp;
						tok = tDoble;
						break;

					default:
						accept(tDoble);
				}
				break; // InNum

			case InFrac:
				c = nextc(file, str);
				switch (c)
				{
					case_digit:	// Stay in this state
						break;

					case 'e':
					case 'E':
						state = InExp;
						break;

					case '.':
						unex();
						break;

					default:
						accept(tDoble);
				}
				break; //InFrac

			case InExp:
				c = nextc(file, str);
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
						accept(tDoble);
				}
				break; // InExp

			case InStr:
				c = nextc(file, str);
				switch (c)
				{
					case '\\':
						state = InEscape;
						break;

					case '"':
						state = Accept;
						tok = tString;
						break;

					case EOF:
						unex();
				}
				break; // InStr

			case InEscape:
				c = nextc(file, str);
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
						state = InStr;
						break;

					default:
						unex();
				}
				break; // InEscape

			case InSpecial:
				c = nextc(file, str);
				if ( ! isalpha(c))
				{
					accept(tSpecial);
				}
				break; // InSpecial


		
		// End of state switch
		}
	
	// End of for loop
	}
}

int main(int argc, char **argv)
{
	FILE *json = open_json("test.json");

	token_t tok;
	char *stringval = malloc(50);
	memset(stringval, 0, 50);

	while ((tok = scan_json(json, stringval)) != tEnd)
	{
		printf("Toke: %s (%d)\n", stringval, tok);
	}
	putchar('\n');

	return 0;

}