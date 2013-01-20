#ifndef PRINT_H
#define PRINT_H

#include "types.h"
#include "global.h"

#define PRINT_S pr->ostream
#define PR_NAME pr

#define del_printer(pr) c_free(pr)
#define close_ostream(pr) fclose(pr->ostream)

typedef enum { Pretty, Compact } printmode_t;

struct printer_t_st {
	FILE *ostream;
	unsigned int level;
	unsigned int donl;
	unsigned int doin;
};
typedef struct printer_t_st printer_t;

printer_t *new_printer(FILE *, printmode_t);

void print_scalar(printer_t *, thing_t *); 
void print_arr(printer_t *, thing_t *); 
void print_obj(printer_t *, thing_t *); 
void print_thing(printer_t *, thing_t *); 

/* PRINT_H */
#endif