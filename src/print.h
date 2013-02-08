#ifndef PRINT_H
#define PRINT_H

#include "types.h"
#include "global.h"

#define PRINT_S pr->ostream
#define PR_NAME pr

#define BUFFER_START 1024

#define close_ostream(pr) fclose(pr->ostream)

typedef enum { Pretty, Compact } printmode_t;

struct printer_t_st {
	FILE *ostream;
	unsigned int level;
	unsigned int donl;
	unsigned int doin;
};
typedef struct printer_t_st printer_t;

#ifdef __cplusplus
extern "C" {
#endif // Prevent name-mangling

printer_t * JDAD_DLL new_printer(FILE *, printmode_t);
void JDAD_DLL del_printer(printer_t *);

void JDAD_DLL print_scalar(printer_t *, thing_t *); 
void JDAD_DLL print_arr(printer_t *, thing_t *); 
void JDAD_DLL print_obj(printer_t *, thing_t *); 
void JDAD_DLL print_thing(printer_t *, thing_t *); 

#ifdef __cplusplus
}
#endif

/* PRINT_H */
#endif