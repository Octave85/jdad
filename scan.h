#ifndef SCAN_H
#define SCAN_H

#include "types.h"
#include <ctype.h>
#include <stdlib.h>

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
	tExp,	  // 9
	tTrue,    // 10
	tFalse,   // 11
	tNull,    // 12
	tEscape,  // 13
	tEnd,     // 14
	tErr,	  // 15
} token_t;

typedef enum {
	Start, Accept, Error,
	InObjK, InObjV,
	InArr, 
	StartNum, InNum, InFrac, EndNum, InExp, InStr,
	InEscape, InHex,
	InT1, InT2, InT3,
	InF1, InF2, InF3, InF4,
	InN1, InN2, InN3, 
} state_t;

typedef struct {
	char *str;
	state_t state;
	unsigned int errors;
	FILE *file;
} scanner_t;

token_t scan_json(scanner_t *);
FILE *open_json(char *);

#endif