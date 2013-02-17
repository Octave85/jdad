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
	tInteger, // 9
	tExp,	  // 10
	tTrue,    // 11
	tFalse,   // 12
	tNull,    // 13
	tEscape,  // 14
	tEnd,     // 15
	tErr,	  // 16
	tBegin,	  // 17
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
	"Integer",
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

enum scan_mode {
	mode_file, mode_string, mode_undet,
};

typedef struct {
	jchar *str;
	unsigned int buflen;
	state_t state;
	unsigned int errors;
	void *in;
	enum scan_mode mode;
} scanner_t;


token_t scan_json(scanner_t *);
FILE *open_json(jchar *);

#endif
