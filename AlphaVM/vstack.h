#ifndef _V_STACK_H_
#define _V_STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "alphatables.h"

#ifdef __DEBUG
	#include <assert.h>
#else
	#define assert(...) 
#endif

#define AVM_STACKSIZE 8192
#define AVM_MAX_INSTRUCTIONS 27
#define AVM_NUMACTUALS_OFFSET 4
#define AVM_SAVEDPC_OFFSET 3
#define AVM_SAVEDTOP_OFFSET 2
#define AVM_SAVEDTOPSP_OFFSET 1

typedef unsigned int uint;
typedef unsigned char bool;
typedef struct userfunc userfunc;
typedef struct instruction instruction;
typedef enum avm_memcell_t avm_memcell_t;
typedef struct avm_memcell avm_memcell;
typedef enum vmarg_t vmarg_t;


enum avm_memcell_t {
	number_m, string_m, bool_m, table_m, userfunc_m, libfunc_m, nil_m, undef_m
};

enum vmarg_t {
	label_a, global_a, formal_a, local_a, number_a, string_a, bool_a, nil_a, \
	userfunc_a, libfunc_a, retval_a, no_vmarg
};

struct avm_memcell {
	avm_memcell_t type;
	union {
		double numVal;
		char* strVal;
		bool boolVal;
		Atable* tableVal;
		uint funcVal;
		char* libfuncVal;
	} data;
};

struct userfunc {
	uint address;
	uint localSize;
	char* id;
};


struct instruction {
	unsigned char op;
	unsigned char res_type;
	unsigned char arg1_type;
	unsigned char arg2_type;
	uint res_val;
	uint arg1_val;
	uint arg2_val;
	uint srcLine;
};

extern avm_memcell stack[];
extern avm_memcell ax,bx,cx;
extern avm_memcell retval;

extern uint currLine;
extern const char* memcell_typeToString[];

extern uint PC;
extern uint top,topsp;
extern uint glob_mem;

extern uint totalActuals;

extern char** string_array;
extern double* number_array;
extern userfunc* userfunc_array;
extern char** libfunc_array;
extern uint userfuncs_a;

extern instruction* instructions; //tmp

void avm_initstack(uint globals_a);
void avm_memcellclear(avm_memcell* memcell);
avm_memcell* avm_translate_operand(unsigned char type, uint val, avm_memcell* reg);
void avm_dec_top();
void avm_callsaveenvironment();
uint avm_get_envvalue(uint i);
uint avm_totalactuals();
avm_memcell* avm_getactual(uint i);
char* avm_tostring(avm_memcell* m);

void avm_error(char* fmt, ...);
void avm_assign(avm_memcell* lv, avm_memcell* rv);

#endif