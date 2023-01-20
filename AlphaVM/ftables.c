#include "functions.h"

/*	
	get/set:
	res  -> value
	arg1 -> table
    arg2 -> key
*/

void execute_newtable(instruction *instr){
	PC++;
	avm_memcell* t = avm_translate_operand(instr->res_type, instr->res_val, NULL);
	assert((t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[top]) || t == &retval);
	
	avm_memcellclear(t);
	
	t->type = table_m;
	t->data.tableVal = Atable_create();
	Atable_increase_ref(t->data.tableVal);
}

void execute_tablegetelem(instruction *instr){
	PC++;
	avm_memcell* v = avm_translate_operand(instr->res_type, instr->res_val, NULL);
	avm_memcell* t = avm_translate_operand(instr->arg1_type, instr->arg1_val, NULL);
	avm_memcell* k = avm_translate_operand(instr->arg2_type, instr->arg2_val, &ax);

	assert((v && &stack[AVM_STACKSIZE-1] >= v && v > &stack[top]) || v == &retval);
	assert(t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[top]);
	assert(k);

	avm_memcellclear(v);
	v->type = nil_m;
	if(t->type != table_m)
		avm_error("illegal use of type %s as table!", memcell_typeToString[t->type]);
	avm_memcell* value = Atable_get_elem(t->data.tableVal, k);
	if(value == NULL){
			avm_memcell nil;
			nil.type = nil_m;
			avm_assign(v, &nil);
			return;
	}
	avm_assign(v, value);
}

void execute_tablesetelem(instruction *instr){
	PC++;
	avm_memcell* t = avm_translate_operand(instr->arg1_type, instr->arg1_val, NULL);
	avm_memcell* k = avm_translate_operand(instr->arg2_type, instr->arg2_val, &ax);
	avm_memcell* v = avm_translate_operand(instr->res_type, instr->res_val, &bx);

	assert(t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[top]);
	assert(k && v);

	if(t->type != table_m)
		avm_error("illegal use of type %s as table!", memcell_typeToString[t->type]);
	
	Atable_insert(t->data.tableVal, k, v);
	
}
