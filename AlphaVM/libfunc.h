#ifndef _LIBFUNC_H_
#define _LIBFUNC_H_

#include "vstack.h"
#include "alphatables.h"
#define F_ARRAY_SIZE 13 // Change only after running program to find new array order.
#define F_HASH_MULTIPLIER 4017 // Change only after running program to find new array order.


extern void (*executeLibFuncs[])(char*);

void invalid_libfunc(char*);
void libfunc_print(char*);
void libfunc_input(char*);
void libfunc_objectmemberkeys(char*);
void libfunc_objecttotalmembers(char*);
void libfunc_objectcopy(char*);
void libfunc_totalarguments(char*);
void libfunc_argument(char*);
void libfunc_typeof(char*);
void libfunc_strtonum(char*);
void libfunc_sqrt(char*);
void libfunc_cos(char*);
void libfunc_sin(char*);

#endif