#include "functions.h"

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

static double add_impl(double x, double y) {return x+y;}
static double sub_impl(double x, double y) {return x-y;}
static double mul_impl(double x, double y) {return x*y;}
static double div_impl(double x, double y) {return x/y;}
static double mod_impl(double x, double y) {return (uint)x % (uint)y;}

double (*arithmeticFuncs[])(double, double) = {
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl
};

void execute_assign(instruction *instr){
	PC++;
	avm_memcell* lv = avm_translate_operand(instr->res_type, instr->res_val , NULL);
	avm_memcell* rv = avm_translate_operand(instr->arg1_type, instr->arg1_val , &ax);

	assert(lv && ( (&stack[AVM_STACKSIZE - 1] >= lv && lv > &stack[top]) || lv == &retval));
	//assert(rv ) similar for rv... KAPPA

	avm_assign(lv,rv);
}


static inline char* concatenate_strings(char* str1, char* str2){
	char* str = malloc(sizeof(char) * (strlen(str1) + strlen(str2) + 1));
	strcpy(str, str1);
	strcat(str, str2);
	return str;
}

void execute_arithmetic(instruction *instr){
	PC++;
	avm_memcell* lv = avm_translate_operand(instr->res_type, instr->res_val, NULL);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);
	
	assert(lv && ( (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top]) || lv == &retval));
	assert(rv1 && rv2);
	
	if(rv1->type == string_m && rv2->type == string_m){
		if(instr->op != 1){
			avm_error("invalid operation on strings.\n");
		}
		char* tmp = concatenate_strings(rv1->data.strVal, rv2->data.strVal);
		avm_memcellclear(lv);
		lv->type = string_m;
		lv->data.strVal = tmp;
		return;
	}
	if(rv1->type != number_m || rv2->type != number_m)
		avm_error("not a number in arithmetic.\n");

	double (*op)(double, double) = arithmeticFuncs[instr->op - 1];
	avm_memcellclear(lv);
	lv->type = number_m;
	lv->data.numVal = op(rv1->data.numVal, rv2->data.numVal);
};


