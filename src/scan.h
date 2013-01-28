#ifndef SCAN_H
#define SCAN_H

#include "types.h"
#include <ctype.h>
#include <stdlib.h>

#define SCANBUF_SIZE 128



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
	tBegin,	  // 16
} token_t;

static const char tok2str[][8] = {
	"LCurl",
	"RCurl",
	"Colon",
	"Comma",
	"LBrace",
	"RBrace",
	"DQuote",
	"String",
	"Doble",
	"Exp",
	"True",
	"False",
	"Null",
	"Escape",
	"End",
	"Err",
	"Begin",
};

typedef enum {
	Start, Accept, Error,
	InObjK, InObjV,
	InArr, 
	StartNum, InNum, InFrac, EndNum, StartExp, InExp, 
	InStr, InEscape, InHex,
	InT1, InT2, InT3,
	InF1, InF2, InF3, InF4,
	InN1, InN2, InN3, 
} state_t;

static const char state2str[][9] = {
	"Start", "Accept", "Error",
	"InObjK", "InObjV",
	"InArr",
	"StartNum", "InNum", "InFrac", "EndNum", "StartExp", "InExp", 
	"InStr", "InEscape", "InHex",
	"InT1", "InT2", "InT3",
	"InF1", "InF2", "InF3", "InF4",
	"InN1", "InN2", "InN3",
};

typedef struct {
	jchar *str;
	unsigned int buflen;
	state_t state;
	unsigned int errors;
	FILE *file;
} scanner_t;


token_t scan_json(scanner_t *);
FILE *open_json(jchar *);

#endif