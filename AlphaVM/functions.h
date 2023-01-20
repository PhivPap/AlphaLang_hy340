#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "vstack.h"

extern uint totalActuals;

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic


void execute_assign(instruction *instr);

void execute_arithmetic(instruction *instr);

void execute_jeq(instruction *instr);
void execute_jne(instruction *instr);
void execute_jle(instruction *instr);
void execute_jge(instruction *instr);
void execute_jlt(instruction *instr);
void execute_jgt(instruction *instr);
void execute_call(instruction *instr);
void execute_pusharg(instruction *instr);
void execute_funcenter(instruction *instr);
void execute_funcexit(instruction *instr);
void execute_newtable(instruction *instr);
void execute_tablegetelem(instruction *instr);
void execute_tablesetelem(instruction *instr);
void execute_jump(instruction *instr);
static inline void execute_nop(instruction *instr){
	perror("NO OP");
}




#endif