#include "functions.h"

static bool number_tobool(avm_memcell* m){ return m->data.numVal != 0;}
static bool string_tobool(avm_memcell* m){ return m->data.strVal[0] != 0;}
static bool bool_tobool(avm_memcell* m){ return m->data.boolVal;}
static bool table_tobool(avm_memcell* m){ return 1;}
static bool userfunc_tobool(avm_memcell* m){ return 1;}
static bool libfunc_tobool(avm_memcell* m){ return 1;}
static bool nil_tobool(avm_memcell* m){ return 0;}
static bool undef_tobool(avm_memcell* m){ assert(0); return 0;}



bool (*toboolFuncs[])(avm_memcell* m) = {
	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};

static bool number_cmp(avm_memcell* m1, avm_memcell* m2){
	assert(m1->type == number_m && m2->type == number_m);
	return m1->data.numVal == m2->data.numVal;
}

static bool string_cmp(avm_memcell* m1, avm_memcell* m2){
	assert(m1->type == string_m && m2->type == string_m);
	return (bool) !strcmp(m1->data.strVal, m2->data.strVal);
}

static bool bool_cmp(avm_memcell* m1, avm_memcell* m2){
	assert(0);
	return 0;
}

static bool table_cmp(avm_memcell* m1, avm_memcell* m2){ 
	assert(m1 && m2);
	return m1->data.tableVal == m2->data.tableVal;
}

static bool userfunc_cmp(avm_memcell* m1, avm_memcell* m2){
	assert(m1->type == userfunc_m && m2->type == userfunc_m);
	return (bool)(m1->data.funcVal == m2->data.funcVal);
}

static bool libfunc_cmp(avm_memcell* m1, avm_memcell* m2){
	assert(m1->type == libfunc_m && m2->type == libfunc_m);
	return (bool) !strcmp(m1->data.libfuncVal, m2->data.libfuncVal);
}

static bool nil_cmp(avm_memcell* m1, avm_memcell* m2){ 
	assert(0);
	return 0;
}
static bool undef_cmp(avm_memcell* m1, avm_memcell* m2){ 
	assert(0);
	return 0;
}

bool (*equalityCheck[])(avm_memcell* m1, avm_memcell* m2) = {
	number_cmp,
	string_cmp,
	bool_cmp,
	table_cmp,
	userfunc_cmp,
	libfunc_cmp,
	nil_cmp,
	undef_cmp
};

static bool avm_tobool(avm_memcell* m){
	assert(m->type >= 0 && m->type < undef_m);
	return toboolFuncs[m->type](m);
}

void execute_jeq(instruction *instr){
	assert(instr->res_type == label_a);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);

	bool result = 0;

	if(rv1->type == undef_m || rv2->type == undef_m){
		avm_error("undef in equality\n");
	}
	if(rv1->type == nil_m || rv2->type == nil_m)
		result = rv1->type == nil_m && rv2->type == nil_m;
	else if(rv1->type == bool_m || rv2->type == bool_m)
		result = (avm_tobool(rv1) == avm_tobool(rv2));
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal\n", memcell_typeToString[rv1->type], memcell_typeToString[rv2->type]);
	}
	else{
		result = equalityCheck[rv1->type](rv1, rv2);
	}
	if(result)
		PC = instr->res_val;
	else
		PC++;
}

void execute_jne(instruction *instr){
	assert(instr->res_type == label_a);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);

	bool result = 0;

	if(rv1->type == undef_m || rv2->type == undef_m){
		avm_error("undef in inequality\n");
	}
	if(rv1->type == nil_m || rv2->type == nil_m)
		result = rv1->type == nil_m && rv2->type == nil_m;
	else if(rv1->type == bool_m || rv2->type == bool_m)
		result = (avm_tobool(rv1) == avm_tobool(rv2));
	else if(rv1->type != rv2->type){
		avm_error("%s != %s is illegal\n", memcell_typeToString[rv1->type], memcell_typeToString[rv2->type]);
	}
	else{
		result = equalityCheck[rv1->type](rv1, rv2);
	}
	if(!result)
		PC = instr->res_val;
	else
		PC++;
}

void execute_jle(instruction *instr){
	assert(instr->res_type == label_a);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);

	if(rv1->type == undef_m || rv2->type == undef_m){
		avm_error("undef in less equal\n");
	}
	if(rv1->type != number_m || rv2->type != number_m ){
		avm_error("%s <= %s is illegal\n", memcell_typeToString[rv1->type], memcell_typeToString[rv2->type]);
	}

	if(rv1->data.numVal <= rv2->data.numVal)
		PC = instr->res_val;
	else
		PC++;
}

void execute_jge(instruction *instr){
	assert(instr->res_type == label_a);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);

	if(rv1->type == undef_m || rv2->type == undef_m){
		avm_error("undef in greater equal\n");
	}
	if(rv1->type != number_m || rv2->type != number_m ){
		avm_error("%s >= %s is illegal\n", memcell_typeToString[rv1->type], memcell_typeToString[rv2->type]);
	}

	if(rv1->data.numVal >= rv2->data.numVal)
		PC = instr->res_val;
	else
		PC++;
}

void execute_jlt(instruction *instr){
	assert(instr->res_type == label_a);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);

	if(rv1->type == undef_m || rv2->type == undef_m){
		avm_error("undef in less than\n");
	}
	if(rv1->type != number_m || rv2->type != number_m ){
		avm_error("%s < %s is illegal\n", memcell_typeToString[rv1->type], memcell_typeToString[rv2->type]);
	}

	if(rv1->data.numVal < rv2->data.numVal)
		PC = instr->res_val;
	else
		PC++;
}

void execute_jgt(instruction *instr){
	assert(instr->res_type == label_a);
	avm_memcell* rv1 = avm_translate_operand(instr->arg1_type, instr->arg1_val, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2_type, instr->arg2_val, &bx);

	if(rv1->type == undef_m || rv2->type == undef_m){
		avm_error("undef in greater than\n");
	}
	if(rv1->type != number_m || rv2->type != number_m ){
		avm_error("%s > %s is illegal\n", memcell_typeToString[rv1->type], memcell_typeToString[rv2->type]);
	}

	if(rv1->data.numVal > rv2->data.numVal)
		PC = instr->res_val;
	else
		PC++;
}

void execute_jump(instruction *instr){
	assert(instr->res_type == label_a);
	/* maybe instruction array overflow check w/e(assert) */
	PC = instr->res_val;
}